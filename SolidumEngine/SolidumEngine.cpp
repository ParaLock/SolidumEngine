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


//Don't worry, global is just for testing purposes.
static unsigned int TEST_COUNTER = 0;

//---------------------------------------ENGINE--------------------------------------------//

//Example service frontend.
namespace ServiceExample {

	//Each client of this service will have its own clientInfo instance.
	struct ClientInfo {
		std::string favoriteColor;
		std::string name;
	};

	struct ClientBuilder {

		std::string m_color;
		std::string m_name;

		SolContract
			<
				ClientBuilder,

				SolFunction<void, std::string>,
				SolFunction<void, std::string>
			> DynamicContract;

		//@TODO: Get rid of the need for this init function, perform contract init directly in constructor. Can't now due to lack of proper copy/move handlers.
		void init() {
			DynamicContract.setAll(
				this,
				ContractSlot(&ClientBuilder::setFavoriteColor, "myFavoriteColorIs"),
				ContractSlot(&ClientBuilder::setName, "myNameIs")
			);
		}

		ClientBuilder() {

		}

		~ClientBuilder() {
		}

		void setFavoriteColor(std::string color) {

			m_color = color;
		}

		void setName(std::string name) {
			m_name = name;
		}

	};


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

}

//------------------------------------------------------------------------------------------//

//----------------------------------------USER----------------------------------------------//
//Example engine service client.. @TODO: Optimize service call selection with enum id mapping in SolInterface.

class BasicClientExample {
private:

	float myVal = 7.3f;
	
	enum Services {
		ExampleService,
		Count
	};

	typedef SolContract
	<
		//Parent object type
		BasicClientExample,

		//Contract begin
		SolFunction<void>

	> DynamicContract;


	typedef SolContract
	<
		BasicClientExample,

		//Contract begin
		SolAnyImpl<std::string>


	> StaticContract;




public:
	typedef SolInterface<Services, StaticContract, DynamicContract> SOL_INTERFACE;

	SOL_INTERFACE SOL;

	BasicClientExample(IEngine* engine) :
		SOL(engine)
	{

		SOL.mapServices(
			std::make_pair("ExampleService", Services::ExampleService)
		);

		SOL.service(Services::ExampleService)->builder(
			std::make_pair("myFavoriteColorIs", std::string("orange")),
			std::make_pair("myNameIs", std::string("Client " + std::to_string(TEST_COUNTER++))));

		SOL.getDynamicContract().setAll
		(
			this,
			ContractSlot(&BasicClientExample::testA, "testA")

		);

	}

	static void initStaticContract() {

		SOL_INTERFACE::getStaticContractConcrete().setAll
		(
			ContractSlot(std::string("hello I am a static string"), "myStaticString")
		);
	}

	void testA() {

		SOL.service(Services::ExampleService)
			->callService<void>("printSomething");

		std::string result = SOL.service(Services::ExampleService)
			->callService<std::string>("doSomething", 2, std::string("hello!"));

		std::cout << result << std::endl;
	}

};

void reg_unreg_test(IEngine* engine) {

	BasicClientExample a(engine);
	BasicClientExample b(engine);

	a.testA();
	b.testA();

	BasicClientExample c(engine);
	
	c.testA();
}

int main()
{
	ServiceManager serviceManager;
	IEngine* engine = new EngineInstance(serviceManager);


	ServiceExample::ServiceExample exampleService;
	serviceManager.addService(&exampleService.SOL_SERVICE);


	auto staticContract = BasicClientExample::SOL_INTERFACE::getStaticContractConcrete();

	reg_unreg_test(engine);
	reg_unreg_test(engine);
	reg_unreg_test(engine);
	reg_unreg_test(engine);
	reg_unreg_test(engine);
	reg_unreg_test(engine);
	reg_unreg_test(engine);
	reg_unreg_test(engine);
	reg_unreg_test(engine);

	BasicClientExample2 d(engine);

	d.testA();

	return 1;

}

