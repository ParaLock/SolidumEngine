#pragma once

#include "EngineAPI.h"
#include "ResourceAPI.h"
#include "ServiceAPI.h"

#include "ServiceManager.h"
#include "SolService.h"

#include "SolAllocator.h"

class EngineInstance : public IEngine
{
private:
	ServiceManager& m_serviceManager;
	SolAllocator    m_allocator;
public:

	EngineInstance(ServiceManager& serviceManager) 
		: m_serviceManager(serviceManager)
	{
	}


	~EngineInstance() {
	}

	//@NOTE: Due to linker issues, functions are defined in header. WILL FIX!
	void registerClient(ISolInterface* clientInterface) {
		m_serviceManager.registerServiceClient(clientInterface);
	}

	void unregisterClient(ISolInterface * clientInterface) {
		m_serviceManager.unregisterServiceClient(clientInterface);
	}


	ISolAllocator* getAllocator() { return &m_allocator; };
};
