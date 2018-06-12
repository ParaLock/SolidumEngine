#pragma once

#include "SolService.h"

#include <map>
#include <string>

class ServiceManager {
private:
	std::map<std::string, SolService::ISolService*> m_services;
public:

	void addService(SolService::ISolService* service);
	SolService::ISolService* getService(std::string name);

};