#include "../include/ServiceManager.h"


void ServiceManager::registerServiceClient(ISolInterface * clientInterface) {
    
    auto& requestedServices = clientInterface->getRequestedServices();

    for (auto itr = requestedServices.begin(); itr != requestedServices.end(); itr++) {

        std::string			serviceName = itr->first;
        ISolServiceProxy**	proxyPtr = itr->second;

        SolService::ISolService* service = getService(serviceName);

        *proxyPtr = service->connectClient(clientInterface);
    }
}

void ServiceManager::addService(SolService::ISolService * service) {
    m_services.insert({ service->getName(), service });
}

SolService::ISolService * ServiceManager::getService(std::string name) {
    return m_services.at(name);
}

void ServiceManager::unregisterServiceClient(ISolInterface * clientInterface) {

    auto& requestedServices = clientInterface->getRequestedServices();

    for (auto itr = requestedServices.begin(); itr != requestedServices.end(); itr++) {

        SolService::ISolService* service = getService(itr->first);

        service->disconnectClient(*itr->second);

    }
}