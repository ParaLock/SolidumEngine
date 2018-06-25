// SolidumEngine.cpp : Defines the entry point for the application.
//

#include "windows.h"

#include <iostream>
#include <atomic>
#include <memory>
#include <utility>

#include "SolidumAPI\include\ResourceAPI.h"
#include "SolidumAPI\include\ServiceAPI.h"

#include "Solidum\EngineCore\include\ServiceManager.h"
#include "Solidum\EngineCore\include\EngineInstance.h"
#include "Solidum\EngineCore\include\SolService.h"


#include "Solidum/ResourceService/include/ResourceService.h"

//Don't worry, global is just for testing purposes.
static unsigned int TEST_COUNTER = 0;

//---------------------------------------ENGINE--------------------------------------------//

//Example service frontend.
namespace ServiceExample {

	////Each client of this service will have its own clientInfo instance.
	//struct ClientInfo {
	//	std::string favoriteColor;
	//	std::string name;
	//};

	//struct ClientBuilder {

	//	std::string m_color;
	//	std::string m_name;

	//	SolContract
	//		<
	//			ClientBuilder,

	//			SolFunction<void, std::string>,
	//			SolFunction<void, std::string>
	//		> DynamicContract;

	//	//@TODO: Get rid of the need for this init function, perform contract init directly in constructor. Can't now due to lack of proper copy/move handlers.
	//	void init() {
	//		DynamicContract.setAll(
	//			this,
	//			ContractSlot(&ClientBuilder::setFavoriteColor, "myFavoriteColorIs"),
	//			ContractSlot(&ClientBuilder::setName, "myNameIs")
	//		);
	//	}

	//	ClientBuilder() {

	//	}

	//	~ClientBuilder() {
	//	}

	//	void setFavoriteColor(std::string color) {

	//		m_color = color;
	//	}

	//	void setName(std::string name) {
	//		m_name = name;
	//	}

	//};
/*

	class ServiceExample 
	{
	public:

		SolService::Service<ClientInfo, ClientBuilder> SOL_SERVICE;

		SolContract
		<
			ServiceExample,

			SolFunction<void, ClientInfo*, ClientBuilder*>,
			SolFunction<std::string, ClientInfo*, int, std::string>,
			SolFunction<void, ClientInfo*>

		> DynamicContract;
	public:


		ServiceExample() {

			DynamicContract.setAll(
				this,
				ContractSlot(&ServiceExample::taskBuilderFinalize, "BUILDER_FINALIZE"),
				ContractSlot(&ServiceExample::doSomething, "doSomething"),
				ContractSlot(&ServiceExample::printSomething, "printSomething")
			);

			SOL_SERVICE.registerContract(&DynamicContract);

			SOL_SERVICE.setName("ExampleService");

		}

		void printSomething(ClientInfo* client) {

			std::cout << "Hello from Service client: " << client->name << std::endl;
		}

		void taskBuilderFinalize(ClientInfo* client, ClientBuilder* builderInfo) {

			client->favoriteColor = builderInfo->m_color;
			client->name		  = builderInfo->m_name;

		}

		std::string doSomething(ClientInfo* client, int a, std::string b) {

			return "TEST123: " + client->favoriteColor;

		}
	};
	*/
}

//------------------------------------------------------------------------------------------//

//----------------------------------------USER----------------------------------------------//
//Example engine service client.. @TODO: Optimize service call selection with enum id mapping in SolInterface.
//
class ResourceExample {
private:

	float myVal = 7.3f;
	
	enum Services {
		ResourceService,
		Count
	};

	typedef SolContract
	<
		ResourceExample,
		SolFunction<void>,
		SolFunction<void, void**>

	> DynamicContract;


	typedef SolContract
	<
		ResourceExample,

		SolFunction<ObjectID, void*, IEngine*>,
		SolAnyImpl<size_t>

	> StaticContract;




public:
	typedef SolInterface<Services, StaticContract, DynamicContract> SOL_INTERFACE;

	SOL_INTERFACE SOL;

	ResourceExample(IEngine* engine) :
		SOL(engine)
	{

		SOL.mapServices(
			std::make_pair("ResourceService", Services::ResourceService)
		);

		SOL.service(Services::ResourceService)->
			builder
			(
				std::make_pair("typeSize", sizeof(ResourceExample)),
				std::make_pair("isPooled", true),
				std::make_pair("typeName", std::string("ExampleResource")),
				std::make_pair("isDiskLoadable", true)
				//...
			);

		SOL.getDynamicContract().setAll
		(
			this,
			ContractSlot(&ResourceExample::testA, "testA"),
			ContractSlot(&ResourceExample::get_resource_ptr, "get_resource_ptr")

		);

		SOL.service(Services::ResourceService)->acceptAndVerifyRuntimeContract(&SOL.getDynamicContract());

	}

	static ObjectID create_resource(void* mem, IEngine* engine) {

		ResourceExample* instance = new(mem) ResourceExample(engine);
		ObjectID id = instance->SOL.service(Services::ResourceService)->ID();

		return id;
		
	}

	void get_resource_ptr(void** dest) {



		*dest = this;


	}

	static void initStaticContract() {

		SOL_INTERFACE::getStaticContractConcrete().setAll
		(
			ContractSlot(&ResourceExample::create_resource, "create_resource"),
			ContractSlot(size_t(sizeof(ResourceExample)), "size")
		);
	}

	void testA() {

		ObjectID foo = SOL.service(Services::ResourceService)->callService<ObjectID>("create_resource_instance", std::string("ExampleResource"), &std::vector<void*>());

		//On a scale from one to ten... How evil is this?
		ResourceExample* instance = nullptr;
		SOL.service(Services::ResourceService)->callService<void>("get_resource", &instance, foo);
	}

};

int main()
{
	ServiceManager serviceManager;
	IEngine* engine = new EngineInstance(serviceManager);

	ResourceFramework::ResourceService exampleService(engine);
	serviceManager.addService(&exampleService.SOL_SERVICE);

	engine->registerType<ResourceExample>("ExampleResource");

	return 1;

}

