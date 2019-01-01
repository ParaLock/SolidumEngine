#pragma once

#include <map>

#include "ServiceManager.h"

#include "IBus.h"
#include "IScheduler.h"
#include "EngineAPI.h"

namespace SolService {

	class IBusRepo {
	public:
		virtual void* placeholder() = 0;
	};

	template<typename T>
	class BusRepo : public IBusRepo {
	private:
		T m_data;
	public:
		T& read() {
			return m_data;
		}

		void write(const T& data) {
			m_data = data;
		}

		void* placeholder() {

			return nullptr;
		}
	};



	class Bus : public IBus {
	private:
		std::map<std::string, IBusRepo*> m_repos;
		IScheduler&                      m_scheduler;
		IAllocator&                      m_allocator;
		IEngine&                         m_engine;
		ServiceManager&                  m_manager;  
	public:

		Bus(IScheduler& scheduler, IAllocator& allocator, IEngine& engine, ServiceManager& manager) : 
					m_scheduler(scheduler), 
					m_allocator(allocator),
					m_engine(engine),
					m_manager(manager)
		{

		}

		void attachService(ISolService* service) {
			
			m_manager.addService(service);
		}

		IAllocator& getAllocator() {
			return m_allocator;
		}

		IScheduler& getServiceScheduler() {
			return m_scheduler;
		}

		IEngine& getEngine() {
			return m_engine;
		}
	};
}

