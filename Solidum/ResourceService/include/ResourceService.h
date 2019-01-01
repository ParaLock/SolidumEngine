#pragma once

#include "ResourceAPI.h"

#include "EngineAPI.h"

#include "IBus.h"
#include "SolService.h"

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

		ObjectID	  m_clientID;
		ObjectID      m_resID;

		void*         m_mem;

		bool		  m_isPooled;
	};

	class ResourceService {
	public:

		ResourceService(IBus& bus) 
			: SOL_SERVICE(bus)
		{
			
			ContractBuilder& builder = SOL_SERVICE.getContractBuilder();
			
			builder.functions<ResourceService>(
					{
						"register_type", 
						"get_resource", 
						"unload_resource", 
						"create_resource_instance",
						"client_reload",
						"testABC"
					}, 
					this,
					&ResourceService::registerType,
					&ResourceService::getResource,
					&ResourceService::unloadResource,
					&ResourceService::createResourceInstance,
					&ResourceService::reloadClient,
					&ResourceService::testABC
				);

			SOL_SERVICE.setName("ResourceService");
		}

		void reloadClient(ClientInfo* info, Contract* clientContract) {
			
			bool isPooled = clientContract->getMember<bool>("isPooled");
			info->m_isPooled = isPooled;

		}

		void registerType(ClientInfo* res, std::string name, void (*reg)(ContractBuilder& builder)) {

			BusRepo<ResourceTypeStore>& store = SOL_SERVICE.getRepo();

			ResourceTypeStore& info = store.read();
			
			if(info.types.find(name) == info.types.end()) {

				info.types.insert({ name,{ Contract(), name } });

				ResourceTypeStore::Type& type = info.types.at(name);

				reg(type.staticContract.getBuilder());

				
			}
		}

		void unloadResource(ClientInfo* callerInfo, ObjectID resHandle) {
			
			PooledWrapper<ClientInfo*>& resourceWrapper = m_resPool.getObject(resHandle);

			ClientInfo* resource						  = resourceWrapper.getVal();

			if (resource->m_isPooled) {

				m_resPool.free(resourceWrapper);

			} else {

				SOL_SERVICE.disconnectClient(&SOL_SERVICE.getClientState(resource->m_clientID).getVal().proxy);

			}
		}

		bool getResource(ClientInfo* callerInfo, void** resPtr, ObjectID resID) {

			auto& infoWrapper = m_resPool.getObject(resID);
			ClientInfo* info = infoWrapper.getVal();

			SolAnyImpl<void**> arg_res_ptr(resPtr);

			SOL_SERVICE.callClient(info->m_clientID, "get_resource_ptr")->invoke({ &arg_res_ptr });

			return true;

		}

		void testABC(ClientInfo* callerInfo, std::string str) {

			std::cout << "FROM RESOURCE SERVICE: " << str << std::endl;
		}

		ObjectID createResourceInstance(ClientInfo* parentCreatorInfo, std::string typeName, std::vector<SolAny*>* args) {

			if (m_resPool.hasFree(typeName)) {

				PooledWrapper<ClientInfo*>& instanceTrackPtr = m_resPool.getFree(typeName);
				ObjectID	resID							   = instanceTrackPtr.getVal()->m_resID;
				ObjectID    clientID						   = instanceTrackPtr.getVal()->m_clientID;


				ClientInfo& info = SOL_SERVICE.getClientState(clientID).getVal().clientInfo;

				info.m_parentCreatorInfo = parentCreatorInfo;

				return resID;
			}
			else {

				PooledWrapper<ClientInfo*>& instanceTrackPtr = m_resPool.getFree(typeName);

				ResourceTypeStore& typeInfo		= SOL_SERVICE.getRepo().read();
				Contract&          typeContract = typeInfo.types.at(typeName).staticContract;

				SolAnyImpl<size_t> typeSize;
				typeContract.invokeCall("get_size", {}, &typeSize);

				void* mem = SOL_SERVICE.getServiceBus().getAllocator().getMemory(typeSize.data());

				ArgPack<void*, IEngine*> create_args(mem, &SOL_SERVICE.getServiceBus().getEngine());

				SolAnyImpl<ObjectID> clientID;
				typeContract.invokeCall("create_resource", create_args.getArgs(), &clientID);

				ClientInfo& info = SOL_SERVICE.getClientState(clientID.data()).getVal().clientInfo;

				instanceTrackPtr.getVal() = &info;

				info.m_parentCreatorInfo = parentCreatorInfo;
				info.m_clientID			 = clientID.data();
				info.m_resID			 = instanceTrackPtr.ID;
				info.m_mem				 = mem;

				return info.m_resID;
			}

		}

		SolService::Service<ResourceTypeStore, ClientInfo> SOL_SERVICE;

	private:
		//Contains pool of resource instances.. Makes use of allocator 

		Pool<ClientInfo*, std::string> m_resPool;

	};


}
