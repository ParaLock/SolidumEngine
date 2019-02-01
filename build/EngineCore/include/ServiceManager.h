#pragma once

#include "../../../SolidumAPI/include/EngineAPI.h"
#include "../../../SolidumAPI/include/ServiceAPI.h"
#include "../../../SolidumAPI/include/ResourceAPI.h"
#include "IService.h"

#include <map>
#include <string>

class ServiceManager {
private:
	std::map<std::string, SolService::ISolService*> m_services;
public:

	//@NOTE: Due to linker issues, functions are defined in header. WILL FIX!
	void addService(SolService::ISolService * service);

	SolService::ISolService * getService(std::string name);

	void registerServiceClient(ISolInterface * clientInterface);
	void unregisterServiceClient(ISolInterface * clientInterface);

};