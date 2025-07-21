/*
 * AudioGraph.h
 *
 *  Created on: Dec. 22, 2020
 *      Author: blackaddr
 */

#ifndef SOURCE_AUDIOGRAPH_H_
#define SOURCE_AUDIOGRAPH_H_

#include <string>
#include <vector>
#include <list>
#include <memory>
#include <cstddef> // for size_t

namespace stride {

class Node; // Forward declaration
class AudioGraph; // Forward declaration

struct Edge {

    std::shared_ptr<Node> srcNodePtr = nullptr;
    unsigned srcChannel;

    std::shared_ptr<Node> destNodePtr = nullptr;
    unsigned destChannel;

    bool operator==(const Edge& e);
};

class Node {
public:

    enum class NodeType : unsigned {
        INTERNAL_NODE = 0,
        ROOT_NODE,
        END_NODE
    };

    Node() = delete;

    Node(unsigned numInputs, unsigned numOutputs, int indexId = 0, NodeType node = NodeType::INTERNAL_NODE)
    : m_numInputs(numInputs), m_numOutputs(numOutputs), m_indexId(indexId), m_nodeType(node)
    {

    }

    std::shared_ptr<Edge> getInput(unsigned inputChannelId);

    std::vector<std::shared_ptr<Edge>> getOutput(unsigned outputChannelId);

    static void addInputConnection(std::shared_ptr<Node> nodePtr, unsigned inputChannelId, std::shared_ptr<Edge> edgePtr);

    static void addOutputConnection(std::shared_ptr<Node> nodePtr, unsigned outputChannelId, std::shared_ptr<Edge> edgePtr);

    void removeInputConnection(unsigned channelId);
    void removeInputConnection(std::shared_ptr<Edge> edgeToRemove);

    // note this will remove all output connections
    void removeOutputConnection(unsigned channelId);
    void removeOutputConnection(std::shared_ptr<Edge> edgeToRemove);

    void removeAllConnections();

    std::vector<std::shared_ptr<Edge>>& getInputConnections()  { return m_inputs; }
    std::vector<std::shared_ptr<Edge>>& getOutputConnections() { return m_outputs; }

    unsigned getNumInputConnections() { return m_inputs.size(); }
    unsigned getNumOutputConnections() { return m_outputs.size(); }

    const unsigned numInputs()  const { return m_numInputs; }
    const unsigned numOutputs() const { return m_numOutputs; }

    int getIndexId() const { return m_indexId; }
    NodeType getType() const { return m_nodeType; }

    bool isStarter();
    bool isEnder();
    bool isStranded();

    std::string name;
    bool        visited = false;  // user accessible flag

private:
    const unsigned m_numInputs;
    const unsigned m_numOutputs;
    int            m_indexId = 0;
    NodeType       m_nodeType;

    std::vector<std::shared_ptr<Edge>> m_inputs;
    std::vector<std::shared_ptr<Edge>> m_outputs;

    friend AudioGraph;
};

class AudioGraph
{
public:


    void reset();

    void addNode(std::shared_ptr<Node> nodePtr);

    void removeNode(std::shared_ptr<Node> nodePtr);

    void addConnection(std::shared_ptr<Node> srcNodePtr,  unsigned outputChannelId,
                       std::shared_ptr<Node> destNodePtr, unsigned inputChannelId);

    void removeInputConnection(std::shared_ptr<Node> destNodePtr, unsigned inputChannelId);

    void removeOutputConnection(std::shared_ptr<Node> srcNodePtr, unsigned outputChannelId);

    void debugPrintGraph() const;

    size_t getNumNodes() const { return m_nodes.size(); }

    std::list<std::shared_ptr<Node>> getTraversalList();

private:
    std::list<std::shared_ptr<Node>> m_nodes;

};

}

#endif /* SOURCE_AUDIOGRAPH_H_ */
