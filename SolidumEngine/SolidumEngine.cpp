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


//Don't worry global is just for testing purposes.
static unsigned int TEST_COUNTER = 0;

//---------------------------------------ENGINE--------------------------------------------//

//Example service frontend.
namespace TaskService {

	struct ClientInfo {
		std::string favoriteColor;
		std::string name;
	};

	struct ClientBuilder {

		std::string m_color;
		std::string m_name;

		FunctionTable
			<
				ClientBuilder,
				SolFunction<void, std::string>,
				SolFunction<void, std::string>
			> FUNCTIONS;

		//@TODO: Get rid of this function, perform table init directly in constructor. Can't now due to lack of proper copy/move handlers.
		void init() {
			FUNCTIONS.setAll(
				TableSlot(objectBind(&ClientBuilder::setFavoriteColor, this), "myFavoriteColorIs"),
				TableSlot(objectBind(&ClientBuilder::setName, this), "myNameIs")
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


	class TaskServiceExample 
	{
	public:

		SolService::Service<ClientInfo, ClientBuilder> SOL_SERVICE;

		FunctionTable
		<
			TaskServiceExample,
			SolFunction<void, ClientInfo*, ClientBuilder*>,
			SolFunction<std::string, ClientInfo*, int, std::string>,
			SolFunction<void, ClientInfo*>

		> FUNCTIONS;
	public:


		TaskServiceExample() {

			FUNCTIONS.setAll(
				TableSlot(objectBind(&TaskServiceExample::taskBuilderFinalize, this), "BUILDER_FINALIZE"),
				TableSlot(objectBind(&TaskServiceExample::doSomething, this), "doSomething"),
				TableSlot(objectBind(&TaskServiceExample::printSomething, this), "printSomething")
			);

			SOL_SERVICE.registerFunctionTable(&FUNCTIONS);

			SOL_SERVICE.setName("TaskService");

		}

		void printSomething(ClientInfo* client) {

			std::cout << "Hello from Task Service client: " << client->name << std::endl;
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
	
	enum Services {
		TaskService,
		Count
	};

	SolInterface<Services> SOL;

public:

	BasicClientExample(IEngine* engine) :
		SOL(engine)
	{

		SOL.requestService<Services::TaskService>("TaskService")
			->finalizeInit();

		SOL.service(Services::TaskService)
			->clientBuilder("myFavoriteColorIs", std::string("orange"))
			->clientBuilder("myNameIs", std::string("Client " + std::to_string(TEST_COUNTER++)))
			->clientBuilderFinalize();

	}

	void testA() {

		SOL.service(Services::TaskService)
			->callService<void>("printSomething");

		std::string result = SOL.service(Services::TaskService)
			->callService<std::string>("doSomething", 2, std::string("hello!"));

		std::cout << result << std::endl;
	}
};

int main()
{
	ServiceManager serviceManager;
	IEngine* engine = new EngineInstance(serviceManager);


	TaskService::TaskServiceExample taskService;
	serviceManager.addService(&taskService.SOL_SERVICE);


	BasicClientExample example1(engine);
	BasicClientExample example2(engine);

	//UNIT TESTS

	example1.testA();
	example2.testA();

	return 1;

}

