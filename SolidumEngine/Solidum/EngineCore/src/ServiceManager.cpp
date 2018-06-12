#include "../include/ServiceManager.h"

void ServiceManager::addService(SolService::ISolService * service)
{
	m_services.insert({ service->getName(), service });
}

SolService::ISolService * ServiceManager::getService(std::string name)
{
	return m_services.at(name);
}
