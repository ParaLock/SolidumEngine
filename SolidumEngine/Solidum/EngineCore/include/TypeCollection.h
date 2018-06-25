#pragma once

#include "../../../SolidumAPI/include/ServiceAPI.h"

struct ISolContract;

struct SolType {
	std::function<void(SolAny*, SolAny*)> bufferCopy;

	ISolContract* staticContract;
};

class TypeCollection {
private:
	std::map<std::string, SolType> m_typeInfoStorage;
public:

	void		  addTypeContract(std::string name, ISolContract* contract);
	ISolContract* getTypeContract(std::string name);

};