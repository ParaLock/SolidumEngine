#pragma once

#include "../../../SolidumAPI/include/EngineAPI.h"
#include "../../../SolidumAPI/include/ResourceAPI.h"
#include "../../../SolidumAPI/include/ServiceAPI.h"

#include "Bus.h"

#include "ServiceManager.h"
#include "SolService.h"
#include "IScheduler.h"

#include "SolAllocator.h"

using namespace SolService;

class EngineInstance : public IEngine
{
private:
	ServiceManager  m_serviceManager;
	IScheduler&     m_scheduler;
	SolAllocator    m_allocator;
	Bus             m_serviceBus;
public:

	EngineInstance(IScheduler& scheduler) 
		:  m_scheduler(scheduler), 
		   m_serviceBus(scheduler, m_allocator, *this, m_serviceManager)
	{
	}


	~EngineInstance() {
	}

	//@NOTE: Due to linker issues, functions are defined in header. WILL FIX!
	void registerClient(ISolInterface* clientInterface);

	IBus& getServiceBus();

	ServiceManager& getServiceManager();

	void unregisterClient(ISolInterface * clientInterface);

	void start();

	IAllocator* getAllocator() { return &m_allocator; };
};
