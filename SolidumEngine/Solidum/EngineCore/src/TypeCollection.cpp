#include "../include/TypeCollection.h"

void TypeCollection::addTypeContract(std::string name, ISolContract * contract)
{
	SolType type;
	type.staticContract = contract;

	m_typeInfoStorage.insert({ name, type });
}

ISolContract * TypeCollection::getTypeContract(std::string name)
{
	auto& typeInfo = m_typeInfoStorage.at(name);

	return typeInfo.staticContract;
}
