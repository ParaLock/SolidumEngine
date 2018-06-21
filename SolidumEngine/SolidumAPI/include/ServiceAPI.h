#pragma once

#include <vector>
#include <functional>
#include <list>
#include <vector>
#include <map>

#include "../include/ResourceAPI.h"

#include "EngineAPI.h"

class IEngine;

typedef unsigned int ClientID;
typedef unsigned int GenericHandle;

struct ISolContract;


struct SolServiceResponse {

	bool		   				  isValid;

	ISolFunction*	   				  call;
	std::function<void(ISolFunction*, ISolFunction*)> boundReturnCallback;
};

class ISolServiceProxy {
private:

	virtual void			   submitRequestAsync(bool isBuilderCall, std::string requestName, std::function<void(SolServiceResponse&)> callback) = 0;
	virtual SolServiceResponse submitRequest(bool isBuilderCall, std::string requestName) = 0;


public:
	virtual ClientID ID() = 0;

	template<typename T>
	void setClientProperty(std::string name, T val) {

		SolServiceResponse response = submitRequest(true, name);

		if (response.isValid) {

			response.call->bindNext(SolAnyImpl<T>(val));

			response.call->invokeBound();
		}
	}

	template<typename... T>
	void builder(T... args) {
		
		using List = int[];
		(void)List {
			0, ((void)(setClientProperty<decltype(args.second)>(args.first, args.second)), 0) ...
		};

		endBuilder();
	}

	virtual bool acceptAndVerifyRuntimeContract(ISolContract*) = 0;

	virtual void endBuilder() = 0;

	template<typename T, typename... T_ARGS>
	void callServiceAsync(std::string name, ISolFunction* callback, T_ARGS... args) {

		SolServiceResponse response = submitRequestAsync(false, name, callback);
	}

	template<typename T_RET, typename... T_ARGS>
	T_RET callService(std::string name, T_ARGS... args) {

		SolServiceResponse response = submitRequest(false, name);

		using List = int[];
		(void)List {
			0, ((void)(response.call->bindNext(SolAnyImpl<T_ARGS>(args))), 0) ...
		};
			
		if constexpr(std::is_void<T_RET>::value) {
			response.call->invokeBound();
		}
		else {

			SolAnyImpl<T_RET> result;

			response.call->bindRet(result);
			response.call->invokeBound();

			return result.data();
		}
	}
};


template<typename T>
struct ContractSlot {
	typedef T INNER;

	T				m_val;
	std::string		m_name;

	ContractSlot(T val, std::string name) {
		m_val = val;
		m_name = name;
	}
};


struct ISolContract {
	virtual std::map<std::string, ISolFunction*>& getFunctions() = 0;
	virtual std::map<std::string, SolAny*>&		 getValues() = 0;
};


template<typename T_PARENT, typename... T_ELEMENT>
struct SolContract : ISolContract {

	template<typename T>
	struct ElementWrapper {

		T							 element;
		std::string					 name;
	};

	std::tuple<ElementWrapper<T_ELEMENT>...>			 elements;

	std::map<std::string, ISolFunction*>	 functions;
	std::map<std::string, SolAny*>			 values;

	template<bool IS_STATIC, unsigned INDEX, typename T_PARENT, typename T>
	void processElement(T_PARENT* parent, T slot) {

		auto& wrapper = std::get<INDEX>(elements);
		wrapper.name = slot.m_name;

		if constexpr(std::is_member_function_pointer<typename T::INNER>::value) {
			if constexpr (IS_STATIC) {
				wrapper.element.set(slot.m_val);
			}
			else {
				wrapper.element.set(objectBind(slot.m_val, parent));
			}
	
			functions.insert({ wrapper.name, &wrapper.element });
		}
		else {

			wrapper.element = SolAnyImpl<typename T::INNER>(slot.m_val);

			values.insert({ wrapper.name, &wrapper.element });
		}


	}

	//--------------------RECURSION MADNESS BEGIN-----------------//

	template<unsigned INDEX, typename T_PARENT, typename T>
	constexpr void processElements(T_PARENT* parent, T first)
	{
		processElement<false, INDEX, T_PARENT, T>(parent, first);
	}

	template<unsigned INDEX, typename T_PARENT, typename T, typename... Rest>
	constexpr void processElements(T_PARENT* parent, T first, Rest... rest)
	{

		processElement<false, INDEX, T_PARENT, T>(parent, first);

		processElements<INDEX + 1, T_PARENT, Rest...>(parent, rest...);
	}

	//Starting case
	template<typename T_PARENT, typename... T>
	void setAll(T_PARENT* parent, T... args) {
		processElements<0, T_PARENT, T...>(parent, args...);
	}

	//-------------------------------------------------------------------//

	template<unsigned INDEX, typename T>
	constexpr void processElements(T first)
	{
		processElement<true, INDEX, T>(nullptr, first);
	}

	template<unsigned INDEX, typename T, typename... Rest>
	constexpr void processElements(T first, Rest... rest)
	{

		processElement<true, INDEX, T>(first);

		processElements<INDEX + 1, Rest...>(rest...);
	}

	//Starting case
	template<typename... T>
	void setAll(T... args) {
		processElements<0, T...>(args...);
	}

	std::map<std::string, ISolFunction*>& getFunctions() {
		return functions;
	}

	std::map<std::string, SolAny*>& getValues() {
		return values;
	}
};



struct ISolInterface {

	virtual std::list<std::pair<std::string, ISolServiceProxy**>>& getRequestedServices() = 0;
	virtual ISolContract*										   getStaticContract() = 0;
};

template<typename T_SERVICE_ENUM, typename T_STATIC_CONTRACT, typename T_DYNAMIC_CONTRACT>
struct SolInterface : ISolInterface {

	static T_STATIC_CONTRACT                                STATIC_CONTRACT;
	T_DYNAMIC_CONTRACT										DYNAMIC_CONTRACT;

	std::array<ISolServiceProxy*, T_SERVICE_ENUM::Count>	registeredServices;
	std::list<std::pair<std::string, ISolServiceProxy**>>	requestedServices;
	
	IEngine*												m_engine;

	SolInterface(IEngine* engine) {

		m_engine = engine;

	}

	~SolInterface() {

		m_engine->unregisterClient(this);

	}

	static T_STATIC_CONTRACT& getStaticContractConcrete() {
		return STATIC_CONTRACT;
	}

	ISolContract* getStaticContract() {
		return &STATIC_CONTRACT;
	}

	T_DYNAMIC_CONTRACT& getDynamicContract() {
		return DYNAMIC_CONTRACT;
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
};

template<typename T_SERVICE_ENUM, typename T_STATIC_CONTRACT, typename T_DYNAMIC_CONTRACT> 
T_STATIC_CONTRACT SolInterface<T_SERVICE_ENUM, T_STATIC_CONTRACT, T_DYNAMIC_CONTRACT>::STATIC_CONTRACT;
