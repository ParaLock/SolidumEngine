#pragma once

#include "../../../SolidumAPI/include/ResourceAPI.h"

#include "../../EngineCore/include/SolService.h"

namespace ResourceFramework {

	using namespace ObjectPool;

	struct ResourceInfo {
		ResourceInfo* m_parentCreatorInfo;
		ObjectID	  m_resClientID;

		bool		  m_isPooled;
	};

	struct ResourceBuilder {

		SolContract
		<
			ResourceBuilder,
			SolFunction<void, std::string>,
			SolFunction<void, size_t>,
			SolFunction<void, bool>,
			SolFunction<void, bool>,
			SolFunction<void, bool>
		> 
			DynamicContract;

		bool		m_isHotSwapable;
		bool		m_isDiskLoadable;
		bool        m_isPooled;
		std::string m_typeName;
		size_t      m_typeSize;

		ResourceBuilder() {


		}

		void init() {

			DynamicContract.setAll(
				this,
				ContractSlot(&ResourceBuilder::typeName, "typeName"),
				ContractSlot(&ResourceBuilder::typeSize, "typeSize"),
				ContractSlot(&ResourceBuilder::isDiskLoadable, "isDiskLoadable"),
				ContractSlot(&ResourceBuilder::isHotSwapable, "isHotSwapable"),
				ContractSlot(&ResourceBuilder::isPooled, "isPooled")
			);
		}

		void isPooled(bool val) {
			m_isPooled = val;
		}

		void isDiskLoadable(bool val) {
			m_isDiskLoadable = val;
		}

		void isHotSwapable(bool val) {
			m_isHotSwapable = val;
		}

		void typeName(std::string val) {
			m_typeName = val;
		}

		void typeSize(size_t val) {
			m_typeSize = val;
		}
	};

	class ResourceService {
	private:
		//Contains pool of resource instances.. Makes use of allocator 
		Pool<ResourceInfo*>        m_resPool;

		typedef SolContract
			<
				ResourceService,
				SolFunction<bool, ResourceInfo*, void**, ObjectID>,
				SolFunction<void, ResourceInfo*, ResourceBuilder*>,
				SolFunction<ObjectID, ResourceInfo*, std::string, std::vector<SolAny*>*>
			>
			DynamicContract;


	public:
		SolService::Service<ResourceInfo, ResourceBuilder, DynamicContract> SOL_SERVICE;

		ResourceService(IEngine* engine) 
			: SOL_SERVICE(engine)
		{
			SOL_SERVICE.getDynamicContract().setAll
			(
				this,
				ContractSlot(&ResourceService::getResource, "get_resource"),
				ContractSlot(&ResourceService::resourceBuilderFinalize, "BUILDER_FINALIZE"),
				ContractSlot(&ResourceService::createResourceInstance, "create_resource_instance")
			);
			
			SOL_SERVICE.setName("ResourceService");
		}

		void resourceBuilderFinalize(ResourceInfo* res, ResourceBuilder* builderInfo) {

			res->m_isPooled = builderInfo->m_isPooled;

		}

		void unloadResource(ResourceInfo* callerInfo, ObjectID resHandle) {
			
			PooledWrapper<ResourceInfo*>& resourceWrapper = m_resPool.getObject(resHandle);

			ResourceInfo* resource = resourceWrapper.getVal();

			if (resource->m_isPooled) {

				m_resPool.free(resourceWrapper);

			} else {

				SOL_SERVICE.disconnectClient(&SOL_SERVICE.getClientState(resourceWrapper.ID).getVal().proxy);

			}
		}

		bool getResource(ResourceInfo* callerInfo, void** resPtr, ObjectID resHandle) {

			SOL_SERVICE.callClient(resHandle, "get_resource_ptr")->invoke({ *resPtr });

			return true;

		}

		ObjectID createResourceInstance(ResourceInfo* parentCreatorInfo, std::string typeName, std::vector<SolAny*>* args) {

			ISolContract* staticContract = SOL_SERVICE.getEngine()->getTypeStaticContract(typeName);
			size_t		  resSize		 = staticContract->getValues().at("size")->data<size_t>();

			void* mem = SOL_SERVICE.getEngine()->getAllocator()->getMemory(resSize);

			std::vector<void*> create_args;

			create_args.push_back(mem);
			create_args.push_back(SOL_SERVICE.getEngine());

			ObjectID id = 0;
			staticContract->getFunctions().at("create_resource")->invoke(&id, create_args);

			ResourceInfo& clientInstanceInfo = SOL_SERVICE.getClientState(id).getVal().clientInfo;

			clientInstanceInfo.m_parentCreatorInfo = parentCreatorInfo;
			clientInstanceInfo.m_resClientID = id;

			return id;
		}

	};
}
