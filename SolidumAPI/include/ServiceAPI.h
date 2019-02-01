#pragma once

#include <vector>
#include <functional>
#include <list>
#include <vector>
#include <map>

#include <tuple>
#include <utility> 
#include <type_traits>
#include <stack>
#include <numeric>

#include "./ResourceAPI.h"
#include "./EngineAPI.h"

template <typename ... Types> struct type_list {};

struct SolServiceResponse {

	bool		   				  isValid;

};

struct FuncElement {

	FuncElement(ISolFunction* f, std::string n) {
		func = f;
		name = n;
	}

	ISolFunction* func;
	std::string name;
};

struct AttribElement {

	SolAny* any;
	std::string name;

};

struct ContractBuilder {


	typedef std::function<void(std::vector<FuncElement>&)>	RegFuncCallback;
	typedef std::function<void(std::vector<std::pair<std::string, SolAny*>>)>		RegAttribCallback;
	typedef std::function<void*(size_t)>											MemCallback;
	
	MemCallback       memCallback;
	RegFuncCallback   regFuncsCallback;
	RegAttribCallback regAttribsCallback;



	ContractBuilder() {

	}

	ContractBuilder(MemCallback mem, RegFuncCallback registerFuncs, RegAttribCallback registerAttribs) {
		memCallback = mem;
		regFuncsCallback = registerFuncs;
		regAttribsCallback = registerAttribs;
	}


	template<typename... T>
	size_t parameterPackSize() {

		size_t size = 0;

		using List = int[];
		(void)List {
			0, ((void)(size += sizeof(T)), 0) ...
		};

		return size;
	}

	template<typename T_THIS, typename T>
	bool initFunction(std::string name, unsigned int& nameIndex, T_THIS* pParent, std::vector<FuncElement>& functions, T funcPtr) {
		//@TODO: Use engine allocator.
		SolFunction<T>* pFunc = new SolFunction<T>;
		
		pFunc->set(objectBind(funcPtr, pParent));

		functions.emplace_back((ISolFunction*)pFunc, name);

		nameIndex++;

		return true;
	}

	template<typename T>
	bool initStaticFunction(std::string name, unsigned int& nameIndex, std::vector<FuncElement>& functions, T funcPtr) {
		//@TODO: Use engine allocator.
		SolFunction<T>* pFunc = new SolFunction<T>;
		
		pFunc->setStatic(funcPtr);

		functions.emplace_back((ISolFunction*)pFunc, name);

		nameIndex++;

		return true;
	}

	template<typename T>
	constexpr bool initAttrib(std::string name, SolAnyImpl<T> any, std::vector<std::pair<std::string, SolAny*>>& members) {
		
		 SolAnyImpl<T>* attrib = new SolAnyImpl<T>;
		 *attrib = any;

		members.push_back(std::make_pair(name, attrib));

		return true;
	}

	template<typename... T>
	void attribs(T... args) {

		std::vector<std::pair<std::string, SolAny*>> members;

		bool status[] = {initAttrib<decltype(args.second)>(args.first, args.second, members)...};

		regAttribsCallback(members);

	}

	template<typename T_THIS, typename... T_FUNCS>
	void functions(std::vector<std::string> names, T_THIS* pThis, T_FUNCS... funcs) {

		size_t offset				= 0;
		unsigned int index			= 0;

		//std::vector<std::pair<std::string, ISolFunction*>> functions;
		std::vector<FuncElement> functions;

		bool status[] = {initFunction<T_THIS>(names[index], index, pThis, functions, funcs)...};
		
		regFuncsCallback(functions);

	}

	template<typename T_THIS, typename... T_FUNCS>
	void staticFunctions(std::vector<std::string> names, T_FUNCS... funcs) {

		size_t offset				= 0;
		unsigned int index			= 0;

		std::vector<FuncElement> functions;

		bool status[] = {initStaticFunction<T_FUNCS>(names[index], index, functions, funcs)...};
		
		regFuncsCallback(functions);

	}

};

class ISolServiceProxy {
private:

	virtual ObjectID           preCache(std::string requestName, std::vector<SolAny*> args, std::vector<unsigned int> order) = 0;

	virtual void			   submitRequestAsync(std::string requestName, ObjectID argBufferID, std::function<void(SolServiceResponse&)> callback) = 0;
	virtual SolServiceResponse submitRequest(std::string requestName, ObjectID argBufferID, SolAny* ret) = 0;
	virtual SolServiceResponse submitRequest(std::string requestName, ObjectID argBufferID) = 0;

public:
	virtual ObjectID ID() = 0;

	virtual ContractBuilder& getContractBuilder() = 0;

	virtual void finalize() = 0;

	template<typename T, typename... T_ARGS>
	void callServiceAsync(std::string name, ISolFunction* callback, T_ARGS... args) {

		//SolServiceResponse response = submitRequestAsync(false, name, { &SolAnyImpl<T>(args)... }, callback);
	}

	template<typename T_RET, typename... T_ARGS>
	T_RET callService(std::string name, T_ARGS... args) {

		SolServiceResponse response;

		ArgPack<T_ARGS...> argPack(args...);

		std::vector<unsigned int> order(sizeof...(T_ARGS));
		std::iota(order.begin(), order.end(), 1);


		if constexpr(!std::is_void<T_RET>::value) {

			SolAnyImpl<T_RET> retStore;

			response = submitRequest(name, preCache(name, argPack.getArgs(), order), &retStore);

			return retStore.data();
		}
		else {

			response = submitRequest(name, preCache(name, argPack.getArgs(), order));
		}


	}
};


struct ISolInterface {

	virtual std::list<std::pair<std::string, ISolServiceProxy**>>& getRequestedServices() = 0;
};

template<typename T_SERVICE_ENUM>
struct SolInterface : ISolInterface {

	std::array<ISolServiceProxy*, T_SERVICE_ENUM::Count>	registeredServices;
	std::list<std::pair<std::string, ISolServiceProxy**>>	requestedServices;

	IEngine*												m_engine;

	SolInterface() {
		
	}

	SolInterface(IEngine* engine) {

		m_engine = engine;

	}

	~SolInterface() {

		m_engine->unregisterClient(this);

	}

	std::list<std::pair<std::string, ISolServiceProxy**>>& getRequestedServices() {
		return requestedServices;
	}

	void addServiceMapping(std::string name, T_SERVICE_ENUM enumVal) {
		requestedServices.push_back(std::make_pair(
			name,
			&registeredServices[static_cast<int>(enumVal)]
		));
	}

	template<typename... T>
	void mapServices(T... args) {

		using List = int[];
		(void)List {
			0, ((void)(addServiceMapping(args.first, args.second)), 0) ...
		};

		finalizeServiceMapping();
	}

	ISolServiceProxy* service(T_SERVICE_ENUM ServiceID) {

		return registeredServices[static_cast<int>(ServiceID)];
	}

	void finalizeServiceMapping() {
		m_engine->registerClient(this);
	}

	IEngine* getEngine() {
		return m_engine;
	}
};
