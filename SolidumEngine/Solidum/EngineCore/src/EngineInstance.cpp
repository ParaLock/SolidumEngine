#include "../include/EngineInstance.h"


EngineInstance::EngineInstance(ServiceManager& serviceManager) :
	m_serviceManager(serviceManager)
{
}


EngineInstance::~EngineInstance()
{
}


void EngineInstance::registerClient(ISolInterface* clientInterface)
{
	auto& requestedServices = clientInterface->getRequestedServices();

	for (auto itr = requestedServices.begin(); itr != requestedServices.end(); itr++) {
		
		std::string			serviceName = itr->first;
		ISolServiceProxy**	proxyPtr	= itr->second;

		SolService::ISolService* service = m_serviceManager.getService(serviceName);

		*proxyPtr = service->connectClient();
	}
}

void EngineInstance::unregisterClient(ISolInterface * clientInterface)
{
	auto& requestedServices = clientInterface->getRequestedServices();

	for (auto itr = requestedServices.begin(); itr != requestedServices.end(); itr++) {

		SolService::ISolService* service = m_serviceManager.getService(itr->first);

		service->disconnectClient(*itr->second);

	}
}
