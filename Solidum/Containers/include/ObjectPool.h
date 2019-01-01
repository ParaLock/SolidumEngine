#pragma once

#include <deque>
#include <vector>
#include <functional>
#include <map>

class ObjectID;

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
			std::function<T()>          creator;
		};

		std::map<T_GROUP_KEY, unsigned int>	m_groupIDMap;
		std::vector<PoolStorage>			m_pools;


		PooledWrapper<T>& getFreeInner(unsigned int groupID) {

			auto& pool = m_pools.at(groupID);

			if (pool.idList.size() == 0) {

				PooledWrapper<T> obj;

				if (pool.creator) {
					obj.val = pool.creator();
				}

				ObjectID id;

				id.instanceID = pool.storage.size();
				id.groupID = groupID;
				id.isActive = true;

				obj.ID = id;

				pool.storage.push_back(obj);

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
			addDefaultGroup();
		}


		void addGroup(T_GROUP_KEY key, std::function<T()> creator) {

			PoolStorage store;
			store.creator = creator;

			m_pools.push_back(store);
			m_groupIDMap.insert({ key, m_pools.size() - 1 });

		}

		void addDefaultGroup() {

			PoolStorage store;
			store.creator = []() { return T(); };

			m_pools.push_back(store);
		}

		ObjectID getGroupID(std::string key) {

			ObjectID id;
			id.groupID = m_groupIDMap.at(key);

			return id;
		}

		PooledWrapper<T>& getObjectIfValid(ObjectID& id) {
		
			if(id.isActive) {
				return m_pools.at(id.groupID).storage.at(id.instanceID);
			} else {
				return getFreeInner(0);
			}
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

		bool hasGroup(T_GROUP_KEY key) {

			return (m_groupIDMap.find(key) != m_groupIDMap.end());

		}

		bool hasFree(T_GROUP_KEY key) {

			if(m_groupIDMap.find(key) == m_groupIDMap.end()) {
				addGroup(key, []() {return T();});
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
