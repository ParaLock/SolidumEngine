#include "../include/CommandService.h"

using namespace CommandFramework;
using namespace SolService;
using namespace ObjectPool;

void CommandService::scanSequentials() {

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

void CommandService::submitTargetedCmd(NetworkNode* callingNode, std::string req, ObjectID node, IArgPack* data, bool isOut) {

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


CommandService::CommandService(IBus& bus) : SOL_SERVICE(bus) {

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

void CommandService::reloadClient(NetworkNode* info, Contract* clientContract) {

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

void CommandService::generateOrder(std::string req, std::vector<unsigned int>& order, std::vector<std::string>& names, NetworkNode* node) {

    //Generate order
    for(int i = 0; i < names.size(); i++) {

        order.push_back(node->cmdDataOrder.at(req + "+" + names[i]));
    }
}

void CommandService::submitBroadcastedOutCommand(NetworkNode* callingNode, std::string req, IArgPack* data) {
            
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

void CommandService::submitTargetedOutCommand(NetworkNode* callingNode, std::string req, ObjectID node, IArgPack* data) {
    
    submitTargetedCmd(callingNode, req, node, data, false);
}

void CommandService::submitTargetedInCommand(NetworkNode* callingNode, std::string req, ObjectID node, IArgPack* data) {

    submitTargetedCmd(callingNode, req, node, data, true);
}

void CommandService::submitTargetedInCommandByNodeName(NetworkNode* callingNode, std::string req, std::string name, IArgPack* data) {

    ObjectID node = nodeNameHash.at(name);

    submitTargetedInCommand(callingNode, req, node, data);
}

void CommandService::submitTargetedOutCommandByNodeName(NetworkNode* callingNode, std::string req, std::string name, IArgPack* data) {

    ObjectID node = nodeNameHash.at(name);

    submitTargetedOutCommand(callingNode, req, node, data);
}

void CommandService::linkNodes(NetworkNode* callingNode, std::string a, std::string b) {
    
    ObjectID aID = nodeNameHash.at(a);
    ObjectID bID = nodeNameHash.at(b);

    commandGraph.linkNodes(aID, bID);
}
