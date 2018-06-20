#pragma once

#include "../../../SolidumAPI/include/ResourceAPI.h"

#include "../../EngineCore/include/SolService.h"

#include "ResourcePool.h"

namespace ResourceFramework {

	class ResourceInfo {
		ResourceInfo* m_creatorInfo;

		ResourceID	  m_resID;
	};

	class ResourceBuilder {
	private:
		bool		m_isHotSwapable;
		bool		m_isDiskLoadable;
		bool        m_isPooled;
		std::string m_typeName;
		size_t      m_typeSize;


		FunctionTable
		<
			SolFunction<void, std::string>,
			SolFunction<void, size_t>,
			SolFunction<void, bool>,
			SolFunction<void, bool>,
			SolFunction<void, bool>
		> 
			FUNCTIONS;
	public:

		ResourceBuilder() {

			FUNCTIONS.setAll(
				TableSlot(objectBind(&ResourceBuilder::typeName, this), "typeName"),
				TableSlot(objectBind(&ResourceBuilder::typeSize, this), "typeSize"),
				TableSlot(objectBind(&ResourceBuilder::isDiskLoadable, this), "isDiskLoadable"),
				TableSlot(objectBind(&ResourceBuilder::isHotSwapable, this), "isHotSwapable"),
				TableSlot(objectBind(&ResourceBuilder::isPooled, this), "isPooled")
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
		ResourcePool                                       m_resPool;

		FunctionTable
			<
				SolFunction<void, ResourceInfo*, GenericHandle>
			>
			FUNCTIONS;

		SolService::Service<ResourceInfo, ResourceBuilder> SOL_SERVICE;
	public:

		void unloadResource(ResourceInfo* info, GenericHandle resHandle) {
			
			

		}

		GenericHandle createResourceInstance(ResourceInfo* parentCreatorInfo, std::string typeName, std::vector<SolAny*>& args) {
			
			
		}

		ResourceService(IEngine* engine) {
			
		}
	};
}
