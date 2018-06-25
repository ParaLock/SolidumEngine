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
	m_serviceManager.registerServiceClient(clientInterface);
}

void EngineInstance::unregisterClient(ISolInterface * clientInterface)
{
	m_serviceManager.unregisterServiceClient(clientInterface);
}

void EngineInstance::registerTypeStaticContract(ISolContract * contract, std::string name)
{
	m_typeCollection.addTypeContract(name, contract);
}

ISolContract * EngineInstance::getTypeStaticContract(std::string name)
{
	return m_typeCollection.getTypeContract(name);
}
