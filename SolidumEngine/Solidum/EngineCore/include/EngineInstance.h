#pragma once

#include "../../../SolidumAPI/include/ResourceAPI.h"
#include "../../../SolidumAPI/include/ServiceAPI.h"

#include "TypeCollection.h"
#include "ServiceManager.h"
#include "SolService.h"

#include "SolAllocator.h"

class EngineInstance : public IEngine
{
private:
	ServiceManager& m_serviceManager;
	TypeCollection  m_typeCollection;
	SolAllocator    m_allocator;
public:
	EngineInstance(ServiceManager& serviceManager);
	~EngineInstance();


	void registerClient(ISolInterface* clientInterface);
	void unregisterClient(ISolInterface* clientInterface);

	void registerTypeStaticContract(ISolContract* contract, std::string name);

	ISolContract* getTypeStaticContract(std::string name);

	ISolAllocator* getAllocator() { return &m_allocator; };
};

