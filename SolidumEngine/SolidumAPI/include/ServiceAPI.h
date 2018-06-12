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

struct SolServiceResponse {

	bool		   isValid;

	ISolFunction*									  call;
	std::function<void(ISolFunction*, ISolFunction*)> boundReturnCallback;
};

class ISolServiceProxy {
private:

	virtual void			   submitRequestAsync(bool isBuilderCall, std::string requestName, std::function<void(SolServiceResponse&)> callback) = 0;
	virtual SolServiceResponse submitRequest(bool isBuilderCall, std::string requestName) = 0;


public:
	virtual ClientID           ID() = 0;

	template<typename T>
	ISolServiceProxy* clientBuilder(std::string attrib, T value) {

		SolServiceResponse response = submitRequest(true, attrib);

		if (response.isValid) {

			response.call->bindNext(SolAnyImpl<T>(value));

			response.call->invokeBound();
		}

		return this;

	}

	virtual void clientBuilderFinalize() = 0;

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


struct ISolInterface {

	virtual std::list<std::pair<std::string, ISolServiceProxy**>>& getRequestedServices() = 0;

};

template<typename T_SERVICE_ENUM>
struct SolInterface : ISolInterface {

	std::array<ISolServiceProxy*, T_SERVICE_ENUM::Count>	registeredServices;
	std::list<std::pair<std::string, ISolServiceProxy**>>	requestedServices;
	IEngine*												m_engine;

	SolInterface(IEngine* engine) {

		m_engine = engine;

	}

	~SolInterface() {
	}

	std::list<std::pair<std::string, ISolServiceProxy**>>& getRequestedServices() {
		return requestedServices;
	}

	template<T_SERVICE_ENUM ServiceID>
	SolInterface<T_SERVICE_ENUM>* requestService(std::string name) {

		requestedServices.push_back(std::make_pair(
			name,
			&registeredServices[static_cast<int>(ServiceID)]
		));

		return this;
	}

	ISolServiceProxy* service(T_SERVICE_ENUM ServiceID) {

		ISolServiceProxy* test = registeredServices[static_cast<int>(ServiceID)];

		int test2 = static_cast<int>(ServiceID);

		return registeredServices[static_cast<int>(ServiceID)];
	}

	void finalizeInit() {
		m_engine->registerClient(this);
	}
};