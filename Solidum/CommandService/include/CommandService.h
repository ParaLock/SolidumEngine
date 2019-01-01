#include "EngineAPI.h"
#include "ResourceAPI.h"
#include "ServiceAPI.h"

#include "SolService.h"

#include "ObjectPool.h"
#include "CommandGraph.h"

#include <vector>
#include <string>
#include <thread>

struct SchedulingPolicy {

    int numCommandsPerCycle;

};

namespace CommandFramework {
    
    using namespace SolService;
    using namespace ObjectPool;


    struct NetworkNode {
        
        ObjectID                 clientID;

        SchedulingPolicy         policy;

        std::string              name;
        bool                     isParallel;
        std::vector<Command>     cmdQueue;
        std::vector<std::string> cmdHandlers;

    };

    struct BusData {

    };

    struct Command {

        std::string op;
        ObjectID    dataID;

        bool        deterministicTime;
    };

    class CommandService {
    private:

        Service<BusData, NetworkNode>      SOL_SERVICE;

        ObjectPool::Pool<Command, size_t>  commandPool;
        CommandGraph<NetworkNode>          commandGraph;

        std::thread                        sequencialScanningThread;

        void scanParallels() {

            // for(int i = 0; i < parallelNodes.size(); i++) {

            //     NetworkNode* node = parallelNodes[i];

            //     for(int j = 0; j < node->policy.numCommandsPerCycle; j++) {

            //         if(node->cmdQueue.size() > 0) {

            //             Command& cmd = node->cmdQueue.back();
            //             node->cmdQueue.pop_back();

            //             SOL_SERVICE.getClientState(node->clientID).getVal().proxy.getClientContract()
            //                 .invokeCachedCall(nullptr, cmd.op, cmd.dataID);

            //         }

            //     }
            // }
        }

        void scanSequentials() {

            //Depth first search
            //commandGraph.addNode();

        }

    protected:
    public:

        CommandService(IEngine* instance) : SOL_SERVICE(instance) {

            ContractBuilder& builder = SOL_SERVICE.getContractBuilder();
			
			builder.functions<CommandService>(
					{
						"__on_client_create",
                        "sequential_scan"
					}, 
					this,
                    &CommandService::onClientCreate,
                    &CommandService::scanSequentials
				);

            
            SOL_SERVICE.splitExecution("sequential_scan");

			SOL_SERVICE.setName("ResourceService");
        }

        void onClientCreate(ObjectID clientID) {

            SOL_SERVICE.getClientState(clientID).getVal().clientInfo.clientID = clientID;

            
        }
        
        void submitCommand(NetworkNode* callingNode, std::vector<std::string> elementNames, IArgPack* data) {
            
            

        }

        void submitCommandAsync(NetworkNode* callingNode, std::vector<std::string> elementNames, IArgPack* data) {
            
            

        }



        // ArgPack<int, float, char> cmdData = {2, 4.0f, 'c'};
        // callService(
        //             "submit_command", 
        //             "graphics_network", 
        //             "render_driver", 
        //             {
        //                 "my_int", 
        //                 "my_float", 
        //                 "a_magic_character"
        //             }, &cmdData);

    };

}

