#pragma once
#include <deque>

#include "../../../SolidumAPI/include/ResourceAPI.h"

namespace ObjectPool {

	template<typename T>
	struct PooledWrapper {
		ObjectID ID;

		T            val;

		T& getVal() {
			return val;
		}

	};

	template<typename T, typename T_GROUP_KEY>
	class Pool {
	private:

		struct PoolStorage {
			std::list<unsigned int>		 idList;
			std::deque<PooledWrapper<T>> storage;
		};

		std::map<T_GROUP_KEY, unsigned int>	m_groupIDMap;
		std::vector<PoolStorage>			m_pools;

		void addGroup(T_GROUP_KEY key) {

			m_pools.push_back(PoolStorage());
			m_groupIDMap.insert({key, m_pools.size() - 1});

		}

		PooledWrapper<T>& getFreeInner(unsigned int groupID) {

			auto& pool = m_pools.at(groupID);

			if (pool.idList.size() == 0) {

				PooledWrapper<T> obj;

				ObjectID id;

				id.instanceID = pool.storage.size();
				id.groupID = groupID;

				obj.ID = id;

				pool.storage.emplace_back(obj);

				return pool.storage.back();
			}
			else {

				PooledWrapper<T>& obj = pool.storage.at(pool.idList.back());

				pool.idList.pop_back();

				return obj;
			}
		}



	public:

		Pool() {
			//Create index zero type pool for systems that only pool a single type.
			m_pools.push_back(PoolStorage());
		}

		PooledWrapper<T>& getObject(ObjectID& id) {
			
			auto& pool = m_pools.at(id.groupID);
			return pool.storage.at(id.instanceID);
		}

		PooledWrapper<T>& getFree(T_GROUP_KEY groupID) {

			unsigned int id = m_groupIDMap.at(groupID);

			return getFreeInner(id);
		}

		PooledWrapper<T>& getFree(unsigned int groupID) {

			return getFreeInner(groupID);
		}

		bool hasFree(unsigned int groupID) {

			return m_pools.at(groupID).idList.size() != 0;
		}

		bool hasFree(T_GROUP_KEY key) {

			if (m_groupIDMap.find(key) == m_groupIDMap.end()) {
				addGroup(key);
			}

			return hasFree(m_groupIDMap.at(key));
		}

		void free(PooledWrapper<T>& obj) {

			auto& pool = m_pools.at(obj.ID.groupID);

			pool.idList.push_back(obj.ID.instanceID);
		}

		void free(ObjectID id) {

			auto& pool = m_pools.at(id.groupID);

			pool.idList.push_back(id.instanceID);

		}
	};

}
