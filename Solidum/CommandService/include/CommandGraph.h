#include "ObjectPool.h"

template<typename T>
class CommandGraph {
private:
    struct Node {
        
        T    data;

        int  priority;

        std::vector<Node*> edges;
    };

    ObjectPool::Pool<Node, std::string> nodePool;

    ObjectID root;

    void scanDependencies(std::vector<T*>& nodes, Node& node) {

        for(int i = 0; i < node.edges.size(); i++) {

            scanDependencies(nodes, *node.edges[i]);
        }

        nodes.push_back(&node.data);        
    }

    void sortEdges(std::vector<Node*>& edges) {

        std::sort(edges.begin(), edges.end(), [](Node* a, Node* b) {
            return a->priority > b->priority;   
        });
    }

public:

    CommandGraph() {

        root = nodePool.getFree(0).ID;
    }

    void performScan(std::vector<T*>& nodes) {

        scanDependencies(nodes, nodePool.getObject(root).getVal());

    }

    void linkNodes(ObjectID& nodeA, ObjectID& nodeB) {

        auto& a = nodePool.getObject(nodeA).getVal();
        auto& b = nodePool.getObject(nodeB).getVal();

        a.edges.push_back(&b);

        sortEdges(a.edges);
    }

    ObjectID addNode(int priority, T data) {

        auto& newNode = nodePool.getFree(0);

        newNode.getVal().data     = data;
        newNode.getVal().priority = priority;

        return newNode.ID;
    }

    ObjectID& getStart() { return root; };

};