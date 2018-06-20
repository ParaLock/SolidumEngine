#pragma once

#include <map>
#include <list>
#include <functional>
#include <iostream>
#include <type_traits>
#include <vector>
#include <typeindex>
#include <string>
#include <atomic>
#include <memory>
#include <sstream>
#include <array>

class ISolFunction;

class SolAny {
public:
	virtual void* pData() = 0;
};

template<typename T>
class SolAnyImpl : public SolAny {
private:
	T m_data;
public:

	SolAnyImpl(T data) {
		m_data = data;
	}

	SolAnyImpl() {

	}

	SolAnyImpl<T> operator=(SolAnyImpl<T>& other) {

		m_data = other.m_data;

		return *this;

	}

	void* pData() {
		return &m_data;
	}

	T data() {
		return m_data;
	}

};

class ISolFunction {
private:
public:

	virtual std::string   getSignature()			  = 0;

	template<typename T_RET, typename... T_ARGS>
	T_RET invoke(T_ARGS... args) {
		return dynamic_cast<SolFunction<T_RET, T_ARGS...>*>(this)->m_funcPtr(args...);
	}

	virtual void invokeBound() = 0;

	virtual void bindNext(SolAny& src) = 0;
	virtual void bindRet(SolAny& ret) = 0;
};

template<typename T_RET, typename... T_ARGS>
class SolFunction : public ISolFunction {
private:

	std::string				 	 m_signature;
	T_RET*                       m_retBuff;
	int                          m_nextUnboundArg;

	std::array<std::function<void(void*, SolFunction<T_RET, T_ARGS...>*)>, sizeof...(T_ARGS)>	m_argSetters;
	std::tuple<T_ARGS...>																		m_args;

	template<unsigned index, typename T>
	void generateSetter() {

		m_argSetters[index] = [](void* src, SolFunction<T_RET, T_ARGS...>* func) {

			auto& arg = std::get<index>(func->m_args);

			arg = *(T*)src;
		};
	}

	template<unsigned numArgs>
	void generateArgHelpers() {}

	template<unsigned numArgs, typename FirstArg>
	void generateArgHelpers()
	{
		generateSetter<numArgs, FirstArg>();
	}

	template<unsigned numArgs, typename FirstArg, typename SecondArg, typename... RestOfArgs>
	void generateArgHelpers()
	{
		generateSetter<numArgs, FirstArg>();

		generateArgHelpers<numArgs + 1, SecondArg, RestOfArgs...>();
	}

public:

	typedef								    T_RET T_RETURN;
	typedef std::function<T_RET(T_ARGS...)> T_FUNC;

	template<typename T_RET, typename... T_ARGS>
	static std::string generateSignature() {

		std::ostringstream stream;

		stream << typeid(T_RET).name();

		using List = int[];
		(void)List {
			0, ((void)(stream << typeid(T_ARGS).name()), 0) ...
		};


		return stream.str();

	}

	template<typename Function, typename Tuple, size_t ... I>
	auto call(Function f, Tuple t, std::index_sequence<I ...>)
	{
		return f(std::get<I>(t) ...);
	}

	template<typename Function, typename Tuple>
	auto call(Function f, Tuple t)
	{
		static constexpr auto size = std::tuple_size<Tuple>::value;
		return call(f, t, std::make_index_sequence<size>{});
	}

	void init() {

		m_retBuff = nullptr;
		m_signature = generateSignature<T_RET, T_ARGS...>();
		m_nextUnboundArg = 0;

		generateArgHelpers<0, T_ARGS...>();
	}

	SolFunction() {
		init();
	}

	SolFunction(std::function<T_RET(T_ARGS...)> func) {
		m_funcPtr = func;
		init();
	}

	~SolFunction() {
	}
	
	void set(std::function<T_RET(T_ARGS...)> func) {
		m_funcPtr = func;
	}
	std::string getSignature() {
		return m_signature;
	}

	void bindNext(SolAny& src) {

		m_argSetters[m_nextUnboundArg++](src.pData(), this);

	}

	void bindRet(SolAny& ret) {
		
		m_retBuff = (T_RET*)ret.pData();

	}

	void invokeBound() {

		if constexpr(std::is_void<T_RET>::value) {
			call(m_funcPtr, m_args);
		}
		else {
			*m_retBuff = call(m_funcPtr, m_args);
		}

		bindFinish();

	}

	void bindFinish() {
		m_nextUnboundArg = 0;
		m_retBuff = nullptr;
	}

	std::function<T_RET(T_ARGS...)>			m_funcPtr;

};

template<typename R, typename C, typename... Args>
std::function<R(Args...)> objectBind(R(C::* func)(Args...), C* instance) {
	return [=](Args... args) { return (instance->*func)(args...); };
}
template<typename R, typename C, typename... Args>
std::function<R(Args...)> objectBind(R(C::* func)(Args...) const, C const& instance) {
	return [=](Args... args) { return (instance.*func)(args...); };
}
