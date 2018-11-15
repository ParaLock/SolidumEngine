#pragma once
#include <mutex>
#include <list>

enum class QUEUE_STATE {
	EMPTY,
	ACTIVE,
	CLOSED,
	SATURATED,

	OP_SUCCESSFUL
};

template<typename T>
class SolQueue {
private:
	std::mutex   m_mutex;
	std::list<T> m_storage;

	QUEUE_STATE  m_state;

	unsigned int m_maxDepth;
public:

	SolQueue() : 
		m_maxDepth(255),
		m_state(QUEUE_STATE::EMPTY)
	{

	}

	SolQueue<T>& operator=(const SolQueue<T>& other) {

		m_storage = other.m_storage;
		m_state = other.m_state;
		m_maxDepth = other.m_maxDepth;

		return *this;

	}

	void depth(unsigned int depth) {

		{
			std::unique_lock<std::mutex> lock(m_mutex);

			m_maxDepth = depth;
		}

	}

	QUEUE_STATE state() {

		{
			std::unique_lock<std::mutex> lock(m_mutex);

			return m_state;
		}

	}

	QUEUE_STATE push(T& item) {

		{
			std::unique_lock<std::mutex> lock(m_mutex);

			if (m_storage.size() == m_maxDepth) {

				m_state = QUEUE_STATE::SATURATED;

				return m_state;
			}

			m_storage.push_back(item);

			m_state = QUEUE_STATE::ACTIVE;

			return QUEUE_STATE::OP_SUCCESSFUL;
		}
	}

	QUEUE_STATE pop(T& out) {

		{
			std::unique_lock<std::mutex> lock(m_mutex);

			if (m_storage.size() == 0) {

				m_state = QUEUE_STATE::EMPTY;

				return m_state;
			}

			if (m_storage.size() == 1) {

				m_state = QUEUE_STATE::EMPTY;
			}

			out = m_storage.front();
			m_storage.pop_front();

			return QUEUE_STATE::OP_SUCCESSFUL;
		}
	}
};