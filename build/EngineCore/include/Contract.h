#pragma once

#include "../../Containers/include/ObjectPool.h"
#include "../../../SolidumAPI/include/ServiceAPI.h"

#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>

using namespace ObjectPool;

struct CallBuffer {

	CallBuffer() {
		isInitialized = false;
	}

	~CallBuffer() {}

	bool                 isInitialized;
	SolAny*              retBuffer;
	std::vector<SolAny*> bufferPtrs;

};

struct Callable {

	Pool<CallBuffer, std::string>	  callBuffers;
	ISolFunction*					  function;
};

struct Contract {

	ContractBuilder m_builder;

	std::map<std::string, unsigned int> elementIDByName;

	struct Functions {

		std::vector<std::string>   names;
		std::vector<Callable>	   functionList;

	} m_functions;

	struct MemberData {
		
		std::vector<std::string>   names;
		std::vector<SolAny*>       attribList;

	} m_attribs;

	
	Contract() {

	}


	~Contract() {

	}

	ContractBuilder& getBuilder();
	std::vector<std::string> getContractKeys();

	ISolFunction* getFunction(unsigned int id);
	ISolFunction* getFunction(std::string name);
	SolAny* getMember(unsigned int id);

	SolAny* getMember(std::string name);

	template<typename T>
	T getMember(std::string name) {
	
		SolAnyImpl<T> itemStore;

		unsigned int id   = elementIDByName.at(name);
		SolAny*      item = m_attribs.attribList[id];

		item->copyTo(&itemStore);

		return itemStore.data();

	}

	unsigned int getCallID(std::string callName);
	void addFunctions(std::vector<FuncElement> funcs);
	void addAttribs(std::vector<std::pair<std::string, SolAny*>> attribs);
	void appendFunction(std::string name, ISolFunction* func);
	void invokeCachedCall(SolAny* ret, std::string callName, ObjectID buffID);
	void invokeCachedCall(SolAny* out, unsigned int callID, ObjectID buffID);
	void invokeCall(std::string callName, std::vector<SolAny*> args, SolAny* ret = nullptr);
	void invokeCall(unsigned int callID, std::vector<SolAny*> args, SolAny* ret = nullptr);

	ObjectID cacheArgs(std::string callName, std::vector<SolAny*> args,std::vector<unsigned int> order);

	ObjectID cacheMoreArgs(std::string callName, std::vector<SolAny*> args, std::vector<unsigned int> order, ObjectID callBuffID);
	ObjectID cacheArgs(unsigned int callID, std::vector<SolAny*> args, std::vector<unsigned int> order, ObjectID callBuffID);
};

