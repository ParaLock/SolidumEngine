#pragma once

#include <string>
#include <vector>

struct Contract;
struct SolServiceResponse;
struct ISolServiceProxy;

namespace SolService {

    class Request;

	class ISolService {
	public:
		virtual ObjectID             preCache(ObjectID clientID, std::string requestName, std::vector<SolAny*> args, std::vector<unsigned int> order) = 0;

		virtual SolServiceResponse   submitRequest(Request& request) = 0;
		virtual SolServiceResponse   submitRequestAsync(Request& request) = 0;

        virtual Contract&            getServiceContract() = 0;

		virtual std::string          getName() = 0;
		virtual void				 setName(std::string name) = 0;

		virtual void                 reloadClient(ObjectID clientID) = 0;

		virtual ISolServiceProxy*    connectClient(ISolInterface* solInterface) = 0;
		virtual void				 disconnectClient(ISolServiceProxy*) = 0;
	};

}
