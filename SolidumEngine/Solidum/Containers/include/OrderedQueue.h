#pragma once
#include <condition_variable>
#include <mutex>
#include <list>
#include <map>

template<typename T>
class OrderedQueue {
private:
	std::condition_variable m_dependencyWait;
	std::mutex              m_dependencyWaitLock;
	unsigned int            m_dependentPriority;
	bool                    m_waitingOnDependency;

	unsigned int            m_maxQueueDepth;
	unsigned int            m_mostRecentPriority;

	std::map<unsigned int, std::list<T>> m_buffer;
public:

	void push(unsigned int priority, T item) {

		if (m_buffer.find(priority) == m_buffer.end()) {

			m_buffer.insert({ priority, std::list<T>() });

		}

		if (m_waitingOnDependency && priority == m_dependentPriority) {

			auto& list = m_buffer.at(priority);

			list.push_back(item);

			m_dependencyWait.notify_all();
		}
		else {

			auto& list = m_buffer.at(priority);

			list.push_back(item);
		}
	}

	T getNext() {

		T item;

		auto& itr = m_buffer.find(m_mostRecentPriority);

		if (itr != m_buffer.end()) {

			auto& list = m_buffer.at(m_mostRecentPriority);

			if (list.empty()) {

				std::unique_lock<std::mutex> lk(m_dependencyWaitLock);
				m_dependencyWait.wait(lk);

				lk.unlock();
			}

		}

		auto& list = m_buffer.at(m_mostRecentPriority);

		item = list.front();
		list.pop_front();

		m_mostRecentPriority++;

		return item;
	}
};
