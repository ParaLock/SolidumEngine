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

	template<typename T>
	class Pool {
	private:
		std::list<unsigned int> m_freeIDs;
		std::deque<PooledWrapper<T>> m_pool;
	public:

		PooledWrapper<T>& getObject(ObjectID& id) {
			return m_pool.at(id);
		}

		PooledWrapper<T>& getFree() {

			if (m_freeIDs.size() == 0) {

				PooledWrapper<T> obj;
			
				obj.ID = m_pool.size();
				m_pool.emplace_back(obj);

				return m_pool.back();
			}
			else {

				PooledWrapper<T>& obj = m_pool.at(m_freeIDs.back());

				//if constexpr(!std::is_pointer<T>::value)
				//	obj.val = T();

				m_freeIDs.pop_back();

				return obj;
			}
		}

		bool hasFree() {
			return m_freeIDs.size() != 0;
		}

		void free(PooledWrapper<T>& obj) {

			m_freeIDs.push_back(obj.ID);
		}

		void free(ObjectID id) {

			m_freeIDs.push_back(id);

		}
	};

}
