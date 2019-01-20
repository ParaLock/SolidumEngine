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


    struct Command {

        Command(std::string req, ObjectID buff, ObjectID targetid) {
            
            target = targetid; 
            op     = req;
            dataID = buff;
        }

        std::string op;

        ObjectID     dataID;
        ObjectID     target;

        bool        broadcast;
        bool        deterministicTime;
    };

    struct NetworkNode {
        
        ObjectID                 clientID;
        ObjectID                 nodeID;

        SchedulingPolicy         policy;

        std::string              test;

        std::string              name;
        bool                     isParallel;
        std::vector<Command>     cmdQueue;
        std::vector<std::string> cmdHandlers;

        std::map<std::string, unsigned int> cmdDataOrder;

        NetworkNode() {

            test = "hello world i hope that this is not broken... abcdefghijklmnopqrstuvwxyz";
        }

    };

    struct BusData {

    };

    class CommandService {
    private:

        Service<BusData, NetworkNode>      SOL_SERVICE;

        //Debug
        int                                lastPriority;

        ObjectPool::Pool<Command, size_t>  commandPool;
        CommandGraph<NetworkNode*>         commandGraph;

        std::map<std::string, ObjectID>    nodeNameHash;

        std::vector<std::string> split (const std::string &s, char delim) {

            std::vector<std::string> result;
            std::stringstream ss (s);
            std::string item;

            while (getline (ss, item, delim)) {
                result.push_back (item);
            }

            return result;
        }

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
            std::vector<NetworkNode*> nodes;

            commandGraph.performScan(nodes);

            for(int i = 0; i < nodes.size(); i++) {
                
                if(nodes[i]->cmdQueue.size() == 0) {
                    return;
                }

                //Apply scheduling rules here.
                Command cmd = nodes[i]->cmdQueue.back();
                nodes[i]->cmdQueue.pop_back();

                SOL_SERVICE.getClientContract(cmd.target).invokeCachedCall(nullptr, cmd.op, cmd.dataID);                
            }
        }

        void submitTargetedCmd(NetworkNode* callingNode, std::string req, ObjectID node, IArgPack* data, bool isOut) {

            auto& edges = commandGraph.getInEdges(callingNode->nodeID);
           
            //Calling node is emiting node.
            if(isOut) {
                edges = commandGraph.getOutEdges(callingNode->nodeID);
            }

            for(int i = 0; i < edges.size(); i++) {
                
                std::vector<unsigned int> order;
                generateOrder(req, order, data->getNames(), edges[i]->data);

                if(edges[i]->data->nodeID == node) {

                    ObjectID cmdBuff = SOL_SERVICE.getClientState(edges[i]->data->clientID).getVal().proxy.getClientContract()
                             .cacheArgs(req, data->getArgs(), order);

                    edges[i]->data->cmdQueue.emplace_back(req, cmdBuff, edges[i]->data->clientID);
                }
            }
        }

    protected:
    public:

        CommandService(IBus& bus) : SOL_SERVICE(bus) {

            ContractBuilder& builder = SOL_SERVICE.getContractBuilder();
			
			builder.functions<CommandService>(
					{
                        "sequential_scan",
                        "meta+client_reload",
                        "submit_targeted_out_cmd",
                        "submit_targeted_in_cmd",
                        "link_nodes"
					}, 
					this,
                    &CommandService::scanSequentials,
                    &CommandService::reloadClient,
                    &CommandService::submitTargetedOutCommandByNodeName,
                    &CommandService::submitTargetedInCommandByNodeName,
                    &CommandService::linkNodes
				);

            
            SOL_SERVICE.splitExecution("sequential_scan");

			SOL_SERVICE.setName("CommandService");

            nodeNameHash.insert({"root", commandGraph.getStart()});

            lastPriority = 0;
            
        }
        
        void reloadClient(NetworkNode* info, Contract* clientContract) {

            std::vector<std::string> keys = clientContract->getContractKeys();
            std::vector<std::string> handler;

            //@TODO: Factor key group stuff into Contract...
            for(int i = 0; i < keys.size(); i++) {

                handler = split(keys[i], '+');
                if(handler.size() >= 2) {

                    if(handler[0] == "order") {
                        
                        for(int i = 2; i < handler.size(); i++) {
                            
                            std::string cmdArg = handler[1] + "+" +handler[i];
                            info->cmdDataOrder.insert({cmdArg, i - 2});
                        }
                    }
                }
            }

            //Add client to graph pool...
            info->nodeID = commandGraph.addNode(info);
            commandGraph.updateNodePriority(lastPriority++, info->nodeID);

            std::string nodeName = clientContract->getMember<std::string>("name");
            info->name           = nodeName;

            nodeNameHash.insert({nodeName, info->nodeID});
		}

        void generateOrder(std::string req, std::vector<unsigned int>& order, std::vector<std::string>& names, NetworkNode* node) {

            //Generate order
            for(int i = 0; i < names.size(); i++) {

                order.push_back(node->cmdDataOrder.at(req + "+" + names[i]));
            }
        }

        void submitBroadcastedOutCommand(NetworkNode* callingNode, std::string req, IArgPack* data) {
                 
            //Calling node is emiting node.
            auto& edges = commandGraph.getOutEdges(callingNode->nodeID);
            for(int i = 0; i < edges.size(); i++) {
                
                std::vector<unsigned int> order;
                generateOrder(req, order, data->getNames(), edges[i]->data);

                ObjectID cmdBuff = SOL_SERVICE.getClientContract(edges[i]->data->clientID)
                             .cacheArgs(req, data->getArgs(), order);

                edges[i]->data->cmdQueue.emplace_back(req, cmdBuff, edges[i]->data->clientID);
            }

        }

        void submitTargetedOutCommand(NetworkNode* callingNode, std::string req, ObjectID node, IArgPack* data) {
            
            submitTargetedCmd(callingNode, req, node, data, false);
        }

        void submitTargetedInCommand(NetworkNode* callingNode, std::string req, ObjectID node, IArgPack* data) {

            submitTargetedCmd(callingNode, req, node, data, true);
        }
        
        void submitTargetedInCommandByNodeName(NetworkNode* callingNode, std::string req, std::string name, IArgPack* data) {

            ObjectID node = nodeNameHash.at(name);

            submitTargetedInCommand(callingNode, req, node, data);
        }

        void submitTargetedOutCommandByNodeName(NetworkNode* callingNode, std::string req, std::string name, IArgPack* data) {

            ObjectID node = nodeNameHash.at(name);

            submitTargetedOutCommand(callingNode, req, node, data);
        }

        void linkNodes(NetworkNode* callingNode, std::string a, std::string b) {
            
            ObjectID aID = nodeNameHash.at(a);
            ObjectID bID = nodeNameHash.at(b);

            commandGraph.linkNodes(aID, bID);
        }

        void submitCommandAsync(NetworkNode* callingNode, IArgPack* data) {
            
            
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


