#pragma once
#include <map>
#include <unordered_map>

class ISolInterface;

class IAllocator {
private:
public:
	virtual void* getMemory(size_t size) 					   = 0;
	virtual void  freeMemory(size_t size, void* mem)           = 0;
};

class IEngine {
public:
	
	virtual void registerClient(ISolInterface* clientInterface) = 0;
	virtual void unregisterClient(ISolInterface* clientInterface) = 0;

	virtual IAllocator* getAllocator() = 0;
};

