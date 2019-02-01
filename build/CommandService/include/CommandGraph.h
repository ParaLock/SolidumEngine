#include "../../Containers/include/ObjectPool.h"

namespace CommandFramework {


template<typename T>
class CommandGraph {
private:
    struct Node {
        
        Node() {

            isRoot = false;
            isLeaf = false;
        }

        bool isRoot;
        bool isLeaf;

        T    data;

        int  priority;

        std::vector<Node*> outEdges;
        std::vector<Node*> inEdges;
    };

    ObjectPool::Pool<Node, std::string> nodePool;

    ObjectID rootID;

    void scanDependencies(std::vector<T>& nodes, Node& node) {

        if(node.isLeaf) {

            nodes.push_back(node.data);

            return;
        }

        for(int i = 0; i < node.outEdges.size(); i++) {

            scanDependencies(nodes, *node.outEdges[i]);
        }

        if(!node.isRoot)
            nodes.push_back(node.data);
            
    }

    void sortEdges(std::vector<Node*>& edges) {

        std::sort(edges.begin(), edges.end(), [](Node* a, Node* b) {
            return a->priority > b->priority;   
        });
    }

public:

    CommandGraph() {

        auto& root = nodePool.getFree(0);
        root.getVal().isRoot = true;

        rootID = root.ID;

    }

    void performScan(std::vector<T>& nodes) {

        scanDependencies(nodes, nodePool.getObject(rootID).getVal());
    }

    void linkNodes(ObjectID& nodeA, ObjectID& nodeB) {

        auto& a = nodePool.getObject(nodeA).getVal();
        auto& b = nodePool.getObject(nodeB).getVal();

        if(!b.isRoot) {
            
            a.outEdges.push_back(&b);
            sortEdges(a.outEdges);
        }

        if(!a.isRoot) {
            
            b.inEdges.push_back(&a);
            sortEdges(b.inEdges);
        }

        a.isLeaf = (a.outEdges.size() == 0);
        b.isLeaf = (b.outEdges.size() == 0);
    }

    ObjectID addNode(T& data) {

        auto& newNode = nodePool.getFree(0);

        newNode.getVal().data     = data;

        return newNode.ID;
    }

    void updateNodePriority(int priority, ObjectID nodeid) {

        auto& node = nodePool.getObject(nodeid).getVal();
        node.priority = priority;
    }

    std::vector<Node*>& getOutEdges(ObjectID nodeID) {

        return nodePool.getObject(nodeID).getVal().outEdges;
    }

    std::vector<Node*>& getInEdges(ObjectID nodeID) {

        return nodePool.getObject(nodeID).getVal().inEdges;
    }

    ObjectID& getStart() { return rootID; };

};

}
