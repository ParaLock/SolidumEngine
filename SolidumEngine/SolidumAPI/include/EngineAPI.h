#pragma once
#include <map>
#include <unordered_map>

struct ISolInterface;
struct MetaType;

class IEngine {
public:
	
	virtual void registerClient(ISolInterface* clientInterface) = 0;
	virtual void unregisterClient(ISolInterface* clientInterface) = 0;


};

