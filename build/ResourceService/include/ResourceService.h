#pragma once

#include "../../../SolidumAPI/include/ResourceAPI.h"

#include "../../../SolidumAPI/include/EngineAPI.h"

#include "../../EngineCore/include/IBus.h"
#include "../../EngineCore/include/SolService.h"

namespace ResourceFramework {

	using namespace SolService;
	using namespace ObjectPool;

	struct ResourceTypeStore {

		struct Type {

			Contract	staticContract;
			std::string typeName;
		};

		std::map<std::string, Type> types;
	};

	struct ClientInfo {
		
		ClientInfo() {
			
			m_resID.instanceID = -1;
			m_resID.groupID    = -1;
		}


		ClientInfo* m_parentCreatorInfo;

		std::string   m_typeName;

		ObjectID	  clientID;
		ObjectID      m_resID;

		void*         m_mem;

		bool		  m_isPooled;
	};

	class ResourceService {
	public:

		ResourceService(IBus& bus);

		void reloadClient(ClientInfo* info, Contract* clientContract);

		void registerType(ClientInfo* res, std::string name, void (*reg)(ContractBuilder& builder));

		void unloadResource(ClientInfo* callerInfo, ObjectID resHandle);
		bool getResource(ClientInfo* callerInfo, void** resPtr, ObjectID resID);

		void testABC(ClientInfo* callerInfo, std::string str);
		
		ObjectID createResourceInstance(ClientInfo* parentCreatorInfo, std::string typeName, std::vector<SolAny*>* args);


		SolService::Service<ResourceTypeStore, ClientInfo> SOL_SERVICE;

	private:
		//Contains pool of resource instances.. Makes use of allocator 

		Pool<ClientInfo*, std::string> m_resPool;

	};


}
