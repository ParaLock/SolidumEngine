#pragma once

#include "EngineAPI.h"
#include "ResourceAPI.h"
#include "ServiceAPI.h"

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
	void registerClient(ISolInterface* clientInterface) {
		m_serviceManager.registerServiceClient(clientInterface);
	}

	IBus& getServiceBus() {

		return m_serviceBus;
	}

	ServiceManager& getServiceManager() {
		return m_serviceManager;
	}

	void unregisterClient(ISolInterface * clientInterface) {
		m_serviceManager.unregisterServiceClient(clientInterface);
	}


	IAllocator* getAllocator() { return &m_allocator; };
};
