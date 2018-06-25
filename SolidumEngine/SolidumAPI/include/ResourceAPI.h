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

typedef unsigned int ObjectID;

class SolAny {
public:

	template<typename T>
	T data() {
		return *(T*)pData();
	}
	
	virtual void* pData() = 0;
	virtual std::function<void(void*, void*)>& getSetter() = 0;
};

template<typename T>
class SolAnyImpl : public SolAny {
private:
	T						   m_data;
	std::function<void(void*, void*)> m_setter;
public:

	SolAnyImpl(T data) {
		
		m_data = data;

		m_setter = [&](void* src, void* dest) {

			new(dest) T();

			*(T*)dest = *(T*)src;

		}
	}

	SolAnyImpl() {

	}

	std::function<void(void*, void*)>& getSetter() {
		return m_setter;
	}

	SolAnyImpl<T> operator=(SolAnyImpl<T>& other) {

		m_data = other.m_data;
		m_setter = other.m_setter;
		return *this;

	}

	void* pData() {
		return &m_data;
	}

	T data() {
		return m_data;
	}

	void data(T data) {
		m_data = data;
	}

};

class ISolFunction {
private:
public:

	virtual std::string   getSignature()			  = 0;
	virtual size_t        getArgsSize() = 0;

	virtual void invoke(void* ret, std::vector<void*> args) = 0;
	virtual void invoke(std::vector<void*> args) = 0;
};

template<typename T_RET, typename... T_ARGS>
class SolFunction : public ISolFunction {
private:

	template<typename T>
	constexpr T getArg(unsigned int& index, std::vector<void*>& args) {

		if constexpr(std::is_pointer<T>::value) {
			return (T)args.at(index--);
		}
		else {
			return *(T*)args.at(index--);
		}
	}

	std::string				 	 m_signature;
	size_t                       m_callBufferSize;


	size_t getArgsSize() {
		return m_callBufferSize;
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

	void init() {

		m_signature = generateSignature<T_RET, T_ARGS...>();
		m_callBufferSize = 0;

		using List = int[];
		(void)List {
			0, ((void)(m_callBufferSize += sizeof(T_ARGS)), 0) ...
		};

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

	void invoke(void* ret, std::vector<void*> args) {

		unsigned int count = sizeof...(T_ARGS) - 1;

		//Darn it c++ :(
		if constexpr(!std::is_void<T_RET>::value) {

			*(T_RET*)ret = m_funcPtr((getArg<T_ARGS>(count, args))...);

		}
		else {

			m_funcPtr((getArg<T_ARGS>(count, args))...);
		}		 
	}

	void invoke(std::vector<void*> args) {

		unsigned int count = sizeof...(T_ARGS) - 1;

		m_funcPtr((getArg<T_ARGS>(count, args))...);

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
