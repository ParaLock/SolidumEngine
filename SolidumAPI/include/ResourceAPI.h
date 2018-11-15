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

struct ObjectID {

	bool isActive = false;
	int			 groupID;
	unsigned int instanceID;

	ObjectID() {
		groupID = -1;
	}
};

class SolAny {
public:

	template<typename T>
	T data() {
		
		T* ptr = (T*)pData();

		return *ptr;
	}

	virtual void copyTo(SolAny* other) = 0;
	virtual void* pData() = 0;
	virtual void clone(void* mem, SolAny** any) = 0;
	virtual std::string type() = 0;


	virtual size_t size() = 0;

	virtual std::function<void(void*, void*)>& getSetter() = 0;
};

template<typename T>
class SolAnyImpl : public SolAny {
private:
	T						   m_data;
	std::string                m_type;
	std::function<void(void*, void*)> m_setter;

	void init() {
		m_type = typeid(T).name();

		m_setter = [&](void* src, void* dest) {

			new(dest) T();

			*(T*)dest = *(T*)src;

		};
	}

public:

	SolAnyImpl(T data) {
		
		m_data = data;
		init();

	}

	SolAnyImpl() {
		init();
	}

	std::string type() {

		return m_type;

	}

	void copyTo(SolAny* other) {

		T* src = (T*)pData();
		T* dest = (T*)other->pData();

		*dest = *src;
	}

	void clone(void* mem, SolAny** out) {
		*out = new(mem) SolAnyImpl<T>(m_data);
	}

	size_t size() {
		return sizeof(decltype(this));
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



template<typename... T_ARGS>
struct ArgPack {
	
	std::tuple<SolAnyImpl<T_ARGS>...> tempStorage;
	std::vector<SolAny*>              pArgs;

	ArgPack(SolAnyImpl<T_ARGS>... args) : tempStorage(args...) {

		packArgs<0, T_ARGS...>();

	}

	template<unsigned Index>
	void packArg() {

		pArgs.push_back(&std::get<Index>(tempStorage));
	}

	template<unsigned Index, typename First>
	void packArgs() {
		packArg<Index>();
	}


	template<unsigned Index, typename First, typename Second, typename... Rest>
	void packArgs() {

		packArg<Index>();
		packArg<Index + 1>();
	}

	std::vector<SolAny*>& getArgs() {

		return pArgs;
	}
};

class ISolFunction {
private:
public:

	virtual std::string   getSignature()			  = 0;
	virtual size_t        getArgsSize() = 0;

	virtual void createCallBuffer(std::vector<SolAny*>& args, SolAny** ret = nullptr) = 0;
	virtual size_t size() = 0;
	virtual void invoke(SolAny* ret, std::vector<SolAny*> args) = 0;
	virtual void invoke(std::vector<SolAny*> args) = 0;
	virtual bool hasRet() = 0;

};

template<typename... Args> struct count;

template<>
struct count<> {
    static const int value = 0;
};

template<typename T, typename... Args>
struct count<T, Args...> {
    static const int value = 1 + count<Args...>::value;
};

template <typename> class SolFunction;

template <typename T_RET, typename... T_ARGS>
class SolFunction<T_RET(*)(T_ARGS...)> : public ISolFunction
{
private: 

	size_t					     argBufferSize;
	std::string				 	 m_signature;

	template<typename T>
	constexpr void getOffset(std::vector<SolAny*>& args) {
		//@TODO: Use engine allocator
		args.push_back(new SolAnyImpl<T>());
	}

	//Yuck! fix this allocation at some point!!
	void createCallBuffer(std::vector<SolAny*>& args, SolAny** ret = nullptr) {
		
		size_t offset = 0;

		if constexpr(!std::is_void<T_RET>::value) {
			*ret = new SolAnyImpl<T_RET>();
		}

		using List = int[];
		(void)List {
			0, ((void)(getOffset<T_ARGS>( args)), 0) ...
		};
	}

	template<typename T>
	constexpr auto getArg(int& index, std::vector<SolAny*>& args) {

		SolAnyImpl<T>* data = (SolAnyImpl<T>*)args.at(index);
		index--;

		return data->data();
	}

	size_t size() {
		return sizeof(decltype(this));
	}


	bool hasRet() {
		if constexpr(std::is_void<T_RET>::value) {
			return false;
		}
		else {
			return true;
		}
	}

	std::function<T_RET(T_ARGS...)>			m_funcPtr;

public:
	typedef								    T_RET T_RETURN;
	typedef std::function<T_RET(T_ARGS...)> T_FUNC;

	void init() {

		m_signature = "";//generateSignature();

		using List = int[];
		(void)List {
			0, ((void)(argBufferSize += sizeof(T_ARGS)), 0) ...
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
	
	size_t getArgsSize() {
		return argBufferSize;
	}

	void setStatic(T_RET(*staticFunc)(T_ARGS...)) {
		m_funcPtr = staticFunc;
	}

	void set(std::function<T_RET(T_ARGS...)> func) {
		m_funcPtr = func;
	}

	std::string getSignature() {
		return m_signature;
	}

	void invoke(SolAny* ret, std::vector<SolAny*> args) {
		
		int index = 0;
			
		if constexpr(count<T_ARGS...>::value > 0) {
			index = count<T_ARGS...>::value - 1;	
		} else {
			index = count<T_ARGS...>::value;
		}

		//Darn it c++ :(
		if constexpr(!std::is_void<T_RET>::value) {

			T_RET* dest = nullptr;

			T_RET result = m_funcPtr((getArg<T_ARGS>(index, args))...);

			dest = (T_RET*)ret->pData();

			*dest = result;
		

		}
		else {
			m_funcPtr((getArg<T_ARGS>(index, args))...);
		}		 
	}

	void invoke(std::vector<SolAny*> args) {

		int index = 0;
			
		if constexpr(count<T_ARGS...>::value > 0) {
			index = count<T_ARGS...>::value - 1;	
		} else {
			index = count<T_ARGS...>::value;
		}

		m_funcPtr((getArg<T_ARGS>(index, args))...);

	}

};
template <typename T_RET, typename Object, typename... T_ARGS>
class SolFunction<T_RET(Object::*)(T_ARGS...)> : public ISolFunction
{
private: 

	size_t					     argBufferSize;
	std::string				 	 m_signature;

	template<typename T>
	constexpr void getOffset(std::vector<SolAny*>& args) {
		//@TODO: Use engine allocator
		args.push_back(new SolAnyImpl<T>());
	}

	//Yuck! fix this allocation at some point!!
	void createCallBuffer(std::vector<SolAny*>& args, SolAny** ret = nullptr) {
		
		size_t offset = 0;

		if constexpr(!std::is_void<T_RET>::value) {
			*ret = new SolAnyImpl<T_RET>();
		}

		using List = int[];
		(void)List {
			0, ((void)(getOffset<T_ARGS>( args)), 0) ...
		};
	}

	template<typename T>
	constexpr auto getArg(int& index, std::vector<SolAny*>& args) {

		SolAnyImpl<T>* data = (SolAnyImpl<T>*)args.at(index);
		index--;

		return data->data();
	}

	size_t size() {
		return sizeof(decltype(this));
	}


	bool hasRet() {
		if constexpr(std::is_void<T_RET>::value) {
			return false;
		}
		else {
			return true;
		}
	}

	std::function<T_RET(T_ARGS...)>			m_funcPtr;

	bool                                    m_isStatic;

	template<typename... Args>
	struct pack { };
public:
	//T_RET(Object::*staticFunc)(T_ARGS...);
	typedef								    T_RET T_RETURN;
	typedef std::function<T_RET(T_ARGS...)> T_FUNC;
	typedef Object ParentType;

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

		m_signature = "";//generateSignature();

		using List = int[];
		(void)List {
			0, ((void)(argBufferSize += sizeof(T_ARGS)), 0) ...
		};
	}

	SolFunction() {
		m_isStatic = true;
		init();
	}

	SolFunction(std::function<T_RET(T_ARGS...)> func) {
		m_funcPtr = func;
		m_isStatic = true;
		init();
	}

	~SolFunction() {
	}
	
	size_t getArgsSize() {
		return argBufferSize;
	}

	void setStatic(T_RET(Object::*staticFunc)(T_ARGS...)) {
		m_funcPtr = staticFunc;
		m_isStatic = true;
	}

	void set(std::function<T_RET(T_ARGS...)> func) {
		m_funcPtr = func;
		m_isStatic = false;
	}

	std::string getSignature() {
		return m_signature;
	}

	void invoke(SolAny* ret, std::vector<SolAny*> args) {
		
		int index = 0;
			
		if constexpr(count<T_ARGS...>::value > 0) {
			index = count<T_ARGS...>::value - 1;	
		} else {
			index = count<T_ARGS...>::value;
		}

		//Darn it c++ :(
		if constexpr(!std::is_void<T_RET>::value) {

			T_RET* dest = nullptr;

			if (m_isStatic) {

				T_RET result = m_funcPtr((getArg<T_ARGS>(index, args))...);

				dest = (T_RET*)ret->pData();

				*dest = result;
			}
			else {
				T_RET result = m_funcPtr((getArg<T_ARGS>(index, args))...);

				dest = (T_RET*)ret->pData();

				*dest = result;
			}
		}
		else {

			if (m_isStatic) {

				m_funcPtr((getArg<T_ARGS>(index, args))...);
			}
			else {
				m_funcPtr((getArg<T_ARGS>(index, args))...);
			}
		}		 
	}

	void invoke(std::vector<SolAny*> args) {

		int index = 0;
			
		if constexpr(count<T_ARGS...>::value > 0) {
			index = count<T_ARGS...>::value - 1;	
		} else {
			index = count<T_ARGS...>::value;
		}

		m_funcPtr((getArg<T_ARGS>(index, args))...);

	}

};

// template<typename ...Args>
// void print_1(Args&&... args) {
//     (std::cout << ... << args) << '\n';
// }
template<typename R, typename C, typename... Args>
std::function<R(Args...)> objectBind(R(C::* func)(Args...), C* instance) {
	return [=](Args... args) { 
		
		return (instance->*func)(args...); 
		
		
	};
}
template<typename R, typename C, typename... Args>
std::function<R(Args...)> objectBind(R(C::* func)(Args...) const, C const& instance) {
	return [=](Args... args) { return (instance.*func)(args...); };
}
