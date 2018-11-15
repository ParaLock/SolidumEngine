
#include <iostream>
#include <atomic>
#include <memory>
#include <utility>

#include "SolidumAPI/include/ResourceAPI.h"
#include "SolidumAPI/include/ServiceAPI.h"
#include "SolidumAPI/include/EngineAPI.h"

#include "Solidum/EngineCore/include/ServiceManager.h"
#include "Solidum/EngineCore/include/EngineInstance.h"
#include "Solidum/EngineCore/include/SolService.h"


#include "Solidum/ResourceService/include/ResourceService.h"

class ResourceExample {

private:

	enum Services {
		ResourceService,
		Count
	};

	SolInterface<Services> SOL;

	float myVal = 7.3f;
public:


	char data[4096];

	ResourceExample(IEngine* engine) :
		SOL(engine)
	{

		SOL.mapServices(
			std::make_pair("ResourceService", Services::ResourceService)
		);

		ContractBuilder& builder = SOL.service(Services::ResourceService)->getContractBuilder();

		builder.attribs
			(
				std::make_pair("typeSize", sizeof(ResourceExample)),
				std::make_pair("isPooled", true),
				std::make_pair("typeName", std::string("ExampleResource")),
				std::make_pair("isDiskLoadable", true)
				//...
			);


		builder.functions<ResourceExample>(
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


	}

	void abc(int a, float b) {

	}

	static ObjectID create_resource(void* mem, IEngine* engine) {

		ResourceExample* instance = new(mem) ResourceExample(engine);
		ObjectID id = instance->SOL.service(Services::ResourceService)->ID();

		return id;
		
	}
	
	void get_resource_ptr(void** dest) {
		*dest = this;
	}

	bool testA(int a, float b) {

		auto serviceHandle = SOL.service(Services::ResourceService);

		ObjectID foo = serviceHandle->
			callService<ObjectID>("create_resource_instance", std::string("ResourceExample"), nullptr);

		ResourceExample* instance = nullptr;

		serviceHandle->callService<bool>("get_resource", &instance, foo);
		instance->doSomething();
		serviceHandle->callService<void>("testABC", std::string("This is a test"));

		serviceHandle->callService<void>("unload_resource", foo);

		return true;
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

class Application {
private:

	enum Services {
		ResourceService,
		Count
	};

	SolInterface<Services> SOL;
public:



	Application(IEngine* engine) :
		SOL(engine) 
	{

		SOL.mapServices(
			std::make_pair("ResourceService", Services::ResourceService)
		);
	}

	void run() {

		SOL.service(Services::ResourceService)->callService<void>("register_type", std::string("ResourceExample"), &ResourceExample::getStaticContract);

		ObjectID fooID = SOL.service(Services::ResourceService)->callService<ObjectID>("create_resource_instance", std::string("ResourceExample"), nullptr);
		
		ResourceExample* example = nullptr;
		SOL.service(Services::ResourceService)->callService<ObjectID>("get_resource", &example, fooID);

		int debug = 1;
		while (true) {

			example->testA(1, 3.4f);
			example->testA(1, 3.4f);
			example->testA(1, 3.4f);
			example->testA(1, 3.4f);

		}
	}

};

int main(int argc, char** argv)
{
	ServiceManager serviceManager;
	IEngine* engine = new EngineInstance(serviceManager);

	ResourceFramework::ResourceService exampleService(engine);
	serviceManager.addService(&exampleService.SOL_SERVICE);

	Application app(engine);
	app.run();

	int test = 1;
	return 0;

}