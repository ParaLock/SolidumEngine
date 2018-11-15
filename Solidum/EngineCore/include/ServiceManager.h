#pragma once

#include "EngineAPI.h"
#include "ResourceAPI.h"
#include "SolService.h"

#include <map>
#include <string>

class ServiceManager {
private:
	std::map<std::string, SolService::ISolService*> m_services;
public:

	//@NOTE: Due to linker issues, functions are defined in header. WILL FIX!
	void addService(SolService::ISolService * service) {
		m_services.insert({ service->getName(), service });
	}

	SolService::ISolService * getService(std::string name) {
		return m_services.at(name);
	}

	void registerServiceClient(ISolInterface * clientInterface) {
		
		auto& requestedServices = clientInterface->getRequestedServices();

		for (auto itr = requestedServices.begin(); itr != requestedServices.end(); itr++) {

			std::string			serviceName = itr->first;
			ISolServiceProxy**	proxyPtr = itr->second;

			SolService::ISolService* service = getService(serviceName);

			*proxyPtr = service->connectClient(clientInterface);
		}
	}

	void unregisterServiceClient(ISolInterface * clientInterface) {

		auto& requestedServices = clientInterface->getRequestedServices();

		for (auto itr = requestedServices.begin(); itr != requestedServices.end(); itr++) {

			SolService::ISolService* service = getService(itr->first);

			service->disconnectClient(*itr->second);

		}
	}


};