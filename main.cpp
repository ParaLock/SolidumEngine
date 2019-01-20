
#include <iostream>
#include <atomic>
#include <thread>
#include <memory>
#include <utility>

#include "SolidumAPI/include/ResourceAPI.h"
#include "SolidumAPI/include/ServiceAPI.h"
#include "SolidumAPI/include/EngineAPI.h"

#include "Solidum/EngineCore/include/ServiceManager.h"
#include "Solidum/EngineCore/include/EngineInstance.h"
#include "Solidum/EngineCore/include/SolService.h"
#include "Solidum/EngineCore/include/RoundRobinScheduler.h"

#include "Solidum/CommandService/include/CommandService.h"

#include "Solidum/ResourceService/include/ResourceService.h"

using namespace SolService;

class ResourceExample {

private:

	enum Services {
		ResourceService,
		CommandService,
		Count
	};

	SolInterface<Services> SOL;

	float myVal;
public:


	char data[4096];

	ResourceExample() {
	}

	ResourceExample(IEngine* engine) :
		SOL(engine)
	{

		SOL.mapServices(
			std::make_pair("ResourceService", Services::ResourceService),
			std::make_pair("CommandService",  Services::CommandService)
			//...
		);

		ContractBuilder& resBuilder = SOL.service(Services::ResourceService)->getContractBuilder();

		resBuilder.attribs
			(
				std::make_pair("typeSize", sizeof(ResourceExample)),
				std::make_pair("isPooled", true),
				std::make_pair("typeName", std::string("ExampleResource")),
				std::make_pair("isDiskLoadable", true)
				//...
			);


		resBuilder.functions<ResourceExample>(
			{
				"testA",
				"doSomething",
				"get_resource_ptr"
			}, 
			this,
			&ResourceExample::testA,
			&ResourceExample::doSomething,
			&ResourceExample::get_resource_ptr
			);

		SOL.service(Services::ResourceService)->finalize();


		ContractBuilder& cmdBuilder = SOL.service(Services::CommandService)->getContractBuilder();

		cmdBuilder.functions<ResourceExample>(
			{
				"cmd_handlerA",
				"cmd_handlerB"
			},
			this,
			&ResourceExample::cmd_handlerA,
			&ResourceExample::cmd_handlerB
		);

		cmdBuilder.attribs
			(
				std::make_pair("name", std::string("resource_example")),
				std::make_pair("order+cmd_handlerA+myInt+myFloat", true),
				std::make_pair("order+cmd_handlerB+myString", true)
			);

		SOL.service(Services::CommandService)->finalize();

		myVal = 7.3f;

	}

	static ObjectID create_resource(void* mem, IEngine* engine) {

		ResourceExample* instance = new(mem) ResourceExample(engine);
		ObjectID id = instance->SOL.service(Services::ResourceService)->ID();

		return id;
		
	}
	
	void cmd_handlerA(int a, float b) {

		std::cout << "Command A Handler: " << a << " " << b << std::endl;
	}

	void cmd_handlerB(std::string data) {

		std::cout << "Command B Handler: " << data << std::endl;
	}


	void get_resource_ptr(void** dest) {

		*dest = this;
	}

	bool testA(int a, float b) {

		auto serviceHandle = SOL.service(Services::ResourceService);

		ObjectID foo = serviceHandle->callService<ObjectID>
							(
								"create_resource_instance", 
								std::string("ResourceExample"), 
								nullptr
							);

		ResourceExample* instance = nullptr;

		serviceHandle->callService<bool>
							(
								"get_resource", 
								&instance, 
								foo
							);

		instance->doSomething();

		serviceHandle->callService<void>
							(
								"testABC", 
								std::string("This is a test")
							);

		serviceHandle->callService<void>
							(
								"unload_resource", 
								foo
							);

		return true;
	}

	void testB() {

        ArgPack<int> cmdData = {std::make_pair(42, "magic")};

        SOL.service(Services::CommandService)->callService<void>
							(
								"submit_targeted_out_cmd", 
								std::string("my_cmd_handler"), 
								std::string("another_example"), 
								&cmdData
							);
	}

	void doSomething() {

		std::cout << "Hello From ResourceExample" << std::endl;
	}

	static size_t getSize() {
		
		return sizeof(ResourceExample);
	}

	static void getStaticContract(ContractBuilder& builder) {

		builder.staticFunctions<ResourceExample>(
				{
					"get_size",
					"create_resource"
				},
				&ResourceExample::getSize,
				&ResourceExample::create_resource
			);
	}
};

class AnotherExample {
private:
	enum Services {
		CommandService,
		Count
	};

	SolInterface<Services> SOL;

public:

	AnotherExample() {
		
	}

	AnotherExample(IEngine* engine) :
		SOL(engine)
	{

		SOL.mapServices(
			std::make_pair("CommandService",  Services::CommandService)
			//...
		);

		ContractBuilder& cmdBuilder = SOL.service(Services::CommandService)->getContractBuilder();

		cmdBuilder.functions<AnotherExample>(
			{
				"my_cmd_handler"
			},
			this,
			&AnotherExample::myCmdHandler
		);

		cmdBuilder.attribs
			(
				std::make_pair("name", std::string("another_example")),
				std::make_pair("order+my_cmd_handler+magic", true)
			);

		SOL.service(Services::CommandService)->finalize();
	}

	void myCmdHandler(int magic) {

		std::cout << "My Cmd Handler: " << magic << std::endl;
	}

	void testB() {

        ArgPack<int, float> cmd1Data = 
			{
				std::make_pair(7, "myInt"),
				std::make_pair(9.96f, "myFloat")
			};

        SOL.service(Services::CommandService)->callService<void>
			(
				"submit_targeted_in_cmd", 
                std::string("cmd_handlerA"), 
				std::string("resource_example"), 
				&cmd1Data
			);

		ArgPack<std::string> cmd2Data = 
			{
				std::make_pair(std::string("Hello, World!"), "myString")
			};

		SOL.service(Services::CommandService)->callService<void>
			(
				"submit_targeted_in_cmd",
				std::string("cmd_handlerB"),
				std::string("resource_example"), 
				&cmd2Data
			);


	}

};

class Application {
private:

	enum Services {
		ResourceService,
		CommandService,
		Count
	};

	SolInterface<Services> SOL;

	ResourceExample exampleA;
	AnotherExample exampleB;
public:



	Application(IEngine* engine) :
		SOL(engine), exampleB(engine), exampleA(engine)
	{

		SOL.mapServices(
			std::make_pair("ResourceService", Services::ResourceService),
			std::make_pair("CommandService", Services::CommandService)
		);

		//Note, cmd graph should be loaded from disk... -> That would be an interesting example of inter-service communication.
		SOL.service(Services::CommandService)->callService<void>
			(
				"link_nodes",
				std::string("root"),
				std::string("another_example")
			);

		SOL.service(Services::CommandService)->callService<void>
			(
				"link_nodes",
				std::string("another_example"), 
				std::string("resource_example")	
			);
	}

	void run() {

		// SOL.service(Services::ResourceService)->callService<void>
		// 					(
		// 						"register_type", 
		// 						std::string("ResourceExample"), 
		// 						&ResourceExample::getStaticContract
		// 					);

		// ObjectID fooID = SOL.service(Services::ResourceService)->callService<ObjectID>
		// 					(
		// 						"create_resource_instance", 
		// 						std::string("ResourceExample"), 
		// 						nullptr
		// 					);
		
		// ResourceExample* example = nullptr;
		// SOL.service(Services::ResourceService)->callService<ObjectID>
		// 					(
		// 						"get_resource", 
		// 						&example, 
		// 						fooID
		// 					);

		exampleA.testB();
		exampleB.testB();


		while(true) {


			
		};
		//example->testA(7, 3.3f);

		int test = 1;
	}

};

int main(int argc, char** argv)
{


	// CommandGraph<std::string> testGraph;

	// ObjectID& start = testGraph.getStart();
	// ObjectID a = testGraph.addNode(std::string("a"));
	// ObjectID b = testGraph.addNode(std::string("b"));
	// ObjectID c = testGraph.addNode(std::string("c"));
	// ObjectID d = testGraph.addNode(std::string("d"));
	// ObjectID e = testGraph.addNode(std::string("e"));
	// ObjectID f = testGraph.addNode(std::string("f"));
	// ObjectID g = testGraph.addNode(std::string("g"));
	// ObjectID h = testGraph.addNode(std::string("h"));

	// testGraph.updateNodePriority(1, a);
	// testGraph.updateNodePriority(2, b);
	// testGraph.updateNodePriority(3, c);
	// testGraph.updateNodePriority(4, d);
	// testGraph.updateNodePriority(5, e);
	// testGraph.updateNodePriority(6, f);
	// testGraph.updateNodePriority(7, g);
	// testGraph.updateNodePriority(8, h);


	// testGraph.linkNodes(start, a);
	// testGraph.linkNodes(start, b);
	// testGraph.linkNodes(b, c);
	// testGraph.linkNodes(b, d);
	// testGraph.linkNodes(b, e);
	// testGraph.linkNodes(e, f);
	// testGraph.linkNodes(e, g);

	// std::vector<std::string*> nodes;
	
	// testGraph.performScan(nodes);

	// for(int i = 0; i < 8; i++) {
	// 	std::cout << *nodes[i] << std::endl;
	// }

	RoundRobinScheduler scheduler;

	EngineInstance engine(scheduler);

	ResourceFramework::ResourceService resourceService(engine.getServiceBus());
	CommandFramework::CommandService   commandService(engine.getServiceBus());

	std::thread engineThread(std::bind(&EngineInstance::start, engine));

	Application app(&engine);
	app.run();

	return 0;

}