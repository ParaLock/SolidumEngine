#pragma once
#include <map>
#include <unordered_map>

class ISolService;
struct ISolInterface;

class IEngine {
public:
	
	virtual void registerClient(ISolInterface* clientInterface) = 0;
	virtual void unregisterClient(ISolInterface* clientInterface) = 0;

};

