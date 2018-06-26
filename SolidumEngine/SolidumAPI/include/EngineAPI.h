#pragma once
#include <map>
#include <unordered_map>

struct ISolInterface;
struct ISolContract;

typedef unsigned int TypeID;

class ISolAllocator {
private:
public:
	virtual void* getMemory(size_t size) = 0;
	virtual void  freeMemory(size_t size, void*)           = 0;
};

class IEngine {
public:
	
	virtual void registerTypeStaticContract(ISolContract* contract, std::string name) = 0;

	virtual void registerClient(ISolInterface* clientInterface) = 0;
	virtual void unregisterClient(ISolInterface* clientInterface) = 0;

	template<typename T>
	void registerType(std::string name) {

		T::initStaticContract();

		T::SOL_INTERFACE::STATIC_CONTRACT.setEngine(this);

		registerTypeStaticContract(&T::SOL_INTERFACE::getStaticContractConcrete(), name);
	}

	virtual ISolAllocator* getAllocator() = 0;
	virtual ISolContract* getTypeStaticContract(std::string name) = 0;
};

