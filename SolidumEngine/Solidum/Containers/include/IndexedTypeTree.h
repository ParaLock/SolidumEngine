#pragma once

#include <map>
#include <list>

template<typename T>
class IndexedTypeTree
{
private:

	std::map<unsigned int, std::list<T>> m_rootList;

public:
	IndexedTypeTree() {};
	~IndexedTypeTree() {};

	void push(unsigned int typeIndex, T item) {

		auto& itr = m_rootList.find(typeIndex);

		if (itr != m_rootList.end()) {
			m_rootList.insert({typeIndex, std::list<T>()});
		}

		auto& list = m_rootList.at(typeIndex);

		list.push_back(item);
	}

	void getAll(unsigned int typeIndex, std::list<T>& in) {

		in = m_rootList.at(typeIndex);

	}
};

