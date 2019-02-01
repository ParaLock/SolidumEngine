#include "../include/EngineInstance.h"

void EngineInstance::registerClient(ISolInterface* clientInterface) {

    m_serviceManager.registerServiceClient(clientInterface);
}

IBus& EngineInstance::getServiceBus() {

    return m_serviceBus;
}

ServiceManager& EngineInstance::getServiceManager() {

    return m_serviceManager;
}

void EngineInstance::unregisterClient(ISolInterface * clientInterface) {

    m_serviceManager.unregisterServiceClient(clientInterface);
}

void EngineInstance::start() {
    m_scheduler.start();
}