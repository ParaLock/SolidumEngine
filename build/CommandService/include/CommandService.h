#include "../../../SolidumAPI/include/ResourceAPI.h"

#include "../../../SolidumAPI/include/EngineAPI.h"

#include "../../EngineCore/include/IBus.h"
#include "../../EngineCore/include/SolService.h"

#include "../include/CommandGraph.h"

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

        void scanSequentials();

        void submitTargetedCmd(NetworkNode* callingNode, std::string req, ObjectID node, IArgPack* data, bool isOut);

    protected:
    public:

        CommandService(IBus& bus);
        
        void reloadClient(NetworkNode* info, Contract* clientContract);
        void generateOrder(std::string req, std::vector<unsigned int>& order, std::vector<std::string>& names, NetworkNode* node);

        void submitBroadcastedOutCommand(NetworkNode* callingNode, std::string req, IArgPack* data);
        void submitTargetedOutCommand(NetworkNode* callingNode, std::string req, ObjectID node, IArgPack* data);
        void submitTargetedInCommand(NetworkNode* callingNode, std::string req, ObjectID node, IArgPack* data);
        void submitTargetedInCommandByNodeName(NetworkNode* callingNode, std::string req, std::string name, IArgPack* data);
        void submitTargetedOutCommandByNodeName(NetworkNode* callingNode, std::string req, std::string name, IArgPack* data);

        void linkNodes(NetworkNode* callingNode, std::string a, std::string b);
    };

}


