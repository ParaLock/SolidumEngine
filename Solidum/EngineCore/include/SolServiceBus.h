#pragma once

#include <map>

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

	class Bus {
	private:
		std::map<std::string, IBusRepo*> m_repos;
	public:

	};
}

