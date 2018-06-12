#pragma once

#include "../../../SolidumAPI/include/ResourceAPI.h"
#include "../../../SolidumAPI/include/ServiceAPI.h"

#include "ServiceManager.h"
#include "SolService.h"

class EngineInstance : public IEngine
{
private:
	ServiceManager& m_serviceManager;
public:
	EngineInstance(ServiceManager& serviceManager);
	~EngineInstance();


	void registerClient(ISolInterface* clientInterface);
	void unregisterClient(ISolInterface* clientInterface);
};

