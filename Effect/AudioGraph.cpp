/*
 * AudioGraph.cpp
 *
 *  Created on: Dec. 22, 2020
 *      Author: blackaddr
 */
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "Util/ErrorMessage.h"
#include "Effect/AudioGraph.h"

namespace stride {

bool Edge::operator==(const Edge& e)
{
    if (srcNodePtr != e.srcNodePtr) return false;
    if (destNodePtr != e.destNodePtr) return false;
    if (srcChannel != e. srcChannel) return false;
    if (destChannel != e.destChannel) return false;
    return true;
}

std::shared_ptr<Edge> Node::getInput(unsigned inputChannelId) {
    if (m_numInputs == 0) { return nullptr; }

    if (inputChannelId < m_numInputs) {
        for (auto& edge : m_inputs) {
            if (edge) {
                if (edge->destChannel == inputChannelId) { return edge; }
            }
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Edge>> Node::getOutput(unsigned outputChannelId) {
    std::vector<std::shared_ptr<Edge>> edges;

    if (m_numOutputs == 0) { return edges; }

    if (outputChannelId < m_numOutputs) {

        for (auto& edge : m_outputs) {
            if (edge) {
                if (edge->srcChannel == outputChannelId) { edges.push_back(edge); }
            }
        }
    }
    return edges;
}

void Node::addInputConnection(std::shared_ptr<Node> nodePtr, unsigned inputChannelId, std::shared_ptr<Edge> edgePtr) {

    if (!nodePtr) { return; }
    if (nodePtr->numInputs() == 0) { return; }

    if (inputChannelId < nodePtr->numInputs()) {
        // check to make sure the input doesn't already have an edge
        for (auto& edge : nodePtr->m_inputs) {
            if (edge->destChannel == inputChannelId) { return; }  // input already had a driver
        }

        edgePtr->destNodePtr = nodePtr;
        edgePtr->destChannel = inputChannelId;

        nodePtr->m_inputs.push_back(edgePtr);
    }
}

void Node::addOutputConnection(std::shared_ptr<Node> nodePtr, unsigned outputChannelId, std::shared_ptr<Edge> edgePtr) {

    if (!nodePtr) { return; }
    if (nodePtr->numOutputs() == 0) { return; }

    if (outputChannelId < nodePtr->numOutputs()) {
        // check to make sure the exact same edge doesn't already exist
        for (auto& edge : nodePtr->m_outputs) {
            if (edge == edgePtr) { return; }
        }
        edgePtr->srcNodePtr  = nodePtr;
        edgePtr->srcChannel  = outputChannelId;
        nodePtr->m_outputs.push_back(edgePtr);
    }
}

void Node::removeInputConnection(unsigned channelId) {
    if (m_numInputs == 0) { return; }

    bool restart;
    do {
        restart = false;
        if (m_inputs.size() > 0) {
            for (auto it = m_inputs.begin(); it != m_inputs.end(); ++it) {
                std::shared_ptr<Edge> edge = *it;
                if (edge->destChannel == channelId) {  // found an edge maching the channelid
                    // Find the Edge in the src node list of outputs and delete it
                    if (edge->srcNodePtr) { // this is the src node of the Edge we are removing
                        // a single output channel of a Node can drive multiple edges, so we get a vector back.
                        // we need the iterator to the edge so we can remove it from the src node's vector.
                        std::vector<std::shared_ptr<Edge>>& edgeVec = edge->srcNodePtr->getOutputConnections();
                        if (edgeVec.size() > 0) {
                            for (auto it2 = edgeVec.begin(); it2 != edgeVec.end(); ++it2) {
                                std::shared_ptr<Edge> edgePtr = *it2;
                                // check if this is the edge pointing to this node
                                if (edgePtr) {
                                    if ((edgePtr->destNodePtr.get() == this) && (edgePtr->destChannel == channelId) ) {  // found it
                                        edgePtr->destNodePtr = nullptr;
                                        edgePtr->srcNodePtr  = nullptr;
                                        edgeVec.erase(it2);  // remover the edge from the src Node outputs
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    edge->destNodePtr = nullptr;
                    edge->srcNodePtr  = nullptr;
                    m_inputs.erase(it);  // delete the Edge from thhis node's inputs
                    restart = true;
                    break;
                }
            }
        }
    } while(restart);
}

void Node::removeInputConnection(std::shared_ptr<Edge> edgeToRemove)
{
    if (!edgeToRemove) { return; }
    bool restart;
    do {
        restart = false;
        if (m_inputs.size() > 0) {
            for (auto it = m_inputs.begin(); it != m_inputs.end(); ++it) {
                std::shared_ptr<Edge> inputEdge = *it;
                if ((inputEdge.get() == edgeToRemove.get()) || (inputEdge == edgeToRemove)) {
                    m_inputs.erase(it);
                    inputEdge->destNodePtr = nullptr;
                    restart = true;

                    std::shared_ptr<Node> srcNodePtr = inputEdge->srcNodePtr;
                    if (srcNodePtr) {
                        srcNodePtr->removeOutputConnection(inputEdge);
                        inputEdge->srcNodePtr  = nullptr;
                    }
                    break;
                }
            }
        }
    } while(restart);
}

void Node::removeOutputConnection(unsigned channelId) {
    if (m_numOutputs == 0) { return; }

    bool restart;
    do {
        restart = false;
        if (m_outputs.size() > 0) {
            for (auto it = m_outputs.begin(); it != m_outputs.end(); ++it) {
                std::shared_ptr<Edge> edge = *it;
                if (edge->srcChannel== channelId) {

                    if (edge->destNodePtr) {
                        // remove the Edge from the dest Node input list
                        std::vector<std::shared_ptr<Edge>>& edgeVec = edge->destNodePtr->getInputConnections();
                        if (edgeVec.size() > 0) {
                            for (auto it2 = edgeVec.begin(); it2 != edgeVec.end(); ++it2) {
                                std::shared_ptr<Edge> edgePtr = *it2;
                                // check if this is the edge coming from this node
                                if (edgePtr) {
                                    if ((edgePtr->srcNodePtr.get() == this) && (edgePtr->srcChannel == channelId) ) {  // found it
                                        edgePtr->srcNodePtr = nullptr;
                                        edgeVec.erase(it2);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    edge->srcNodePtr = nullptr;
                    m_outputs.erase(it);
                    restart = true;
                    break;
                }
            }
        }
    } while(restart);
}

void Node::removeOutputConnection(std::shared_ptr<Edge> edgeToRemove)
{
    if (!edgeToRemove) { return; }
    bool restart;
    do {
        restart = false;
        if (m_outputs.size() > 0) {
            for (auto it = m_outputs.begin(); it != m_outputs.end(); ++it) {
                std::shared_ptr<Edge> outputEdge = *it;
                if ((outputEdge.get() == edgeToRemove.get()) || (outputEdge == edgeToRemove)) {
                    // remove from the destination node
                    m_outputs.erase(it);
                    outputEdge->srcNodePtr  = nullptr;
                    restart = true;

                    // Now remove the Edge reference on the dest Node
                    std::shared_ptr<Node> destNodePtr = outputEdge->destNodePtr;
                    if (destNodePtr) {
                        destNodePtr->removeInputConnection(outputEdge);
                        outputEdge->destNodePtr = nullptr;
                    }
                    break;
                }
            }
        }
    } while(restart);
}

void Node::removeAllConnections() {
    bool restart;
    do {
        restart = false;
        if (m_inputs.size() > 0) {
            for (auto& inputEdge : m_inputs) {
                if (inputEdge) {
                    // get the Node driving the edge to this Node
                    std::shared_ptr<Node> nodeToRemove = inputEdge->srcNodePtr;
                    nodeToRemove->removeOutputConnection(inputEdge);
                    restart = true;
                    break;
                }
            }
        }
    } while(restart);
    m_inputs.clear();

    do {
        restart = false;
        if (m_outputs.size() > 0) {
            for (auto& outputEdge : m_outputs) {
                if (outputEdge) {
                    // get the Node driven by this edge
                    std::shared_ptr<Node> nodeToRemove = outputEdge->destNodePtr;
                    nodeToRemove->removeInputConnection(outputEdge);
                    restart = true;
                    break;
                }
            }
        }
    } while(restart);
    m_outputs.clear();
}

void AudioGraph::reset() {
    m_nodes.clear();
}

void AudioGraph::addNode(std::shared_ptr<Node> nodePtr) {

    if (!nodePtr) { return; }

    m_nodes.push_back(nodePtr);
}

void AudioGraph::removeNode(std::shared_ptr<Node> nodePtr) {
    if (!nodePtr) { return; }

    bool restart;
    do {
        restart = false;
        if (m_nodes.size() > 0) {
            for (auto nodeIt = m_nodes.begin(); nodeIt != m_nodes.end(); ++nodeIt) {
                if ( nodeIt->get() == nodePtr.get()) {
                    // found a match
                    // Remove any input or output connections
                    (*nodeIt)->removeAllConnections();

                    m_nodes.erase(nodeIt);
                    restart = true;
                    break;
                }
            }
        }
    } while(restart);
}

void AudioGraph::addConnection(std::shared_ptr<Node> srcNodePtr,  unsigned outputChannelId,
                               std::shared_ptr<Node> destNodePtr, unsigned inputChannelId)
{
    if (!srcNodePtr || !destNodePtr) { return; }

    std::shared_ptr<Edge> edgePtr = std::make_shared<Edge>();

    Node::addOutputConnection(srcNodePtr, outputChannelId, edgePtr);
    std::string msg = "***ADDING OUTPUT EDGE from " + srcNodePtr->name + ":" + std::to_string(outputChannelId) + " to " + destNodePtr->name + ":" + std::to_string(inputChannelId);
    noteMessage(msg);
    msg = "It now has " + std::to_string(srcNodePtr->getOutputConnections().size()) + " output edges";
    noteMessage(msg);

    Node::addInputConnection(destNodePtr, inputChannelId, edgePtr);
}

void AudioGraph::removeInputConnection(std::shared_ptr<Node> destNodePtr, unsigned inputChannelId)
{
    if (!destNodePtr) { return; }

    // First, get the edge on the input of the node
    std::shared_ptr<Edge> edgePtr    = destNodePtr->getInput(inputChannelId);
    std::shared_ptr<Node> srcNodePtr = nullptr;

    // use the edge to get the srcnode that drives the edge
    if (edgePtr) { srcNodePtr = edgePtr->srcNodePtr; }

    // Disconnect the output channel on the SRC NODE
    if (srcNodePtr) { srcNodePtr->removeOutputConnection(edgePtr->srcChannel); }

    // Disconnect the input channel on the DEST NODE
    destNodePtr->removeInputConnection(inputChannelId);
}

void AudioGraph::removeOutputConnection(std::shared_ptr<Node> srcNodePtr, unsigned outputChannelId)
{
    if (!srcNodePtr) { return; }

    std::shared_ptr<Node> destNodePtr = nullptr;

    // First, get the edge on teh output of the node
    std::vector<std::shared_ptr<Edge>> edgeVec = srcNodePtr->getOutput(outputChannelId);

    for (auto& edgePtr : edgeVec ) {
        // use the edge to get the destNode that the edge drives
        if (edgePtr) { destNodePtr = edgePtr->destNodePtr; }

        // Disconnect the input channel on the DEST NODE
        if (destNodePtr) { destNodePtr->removeInputConnection(edgePtr->destChannel); }

        // Disconnect the ouptut channel on the SRC NODE
        srcNodePtr->removeOutputConnection(edgePtr->srcChannel);
    }
}

void AudioGraph::debugPrintGraph() const {

    printf("\n***AudioGraph::debugPrintGrapth():\n");

    if (m_nodes.size() < 1) { std::cout << "AudioGraph::debugPrintGraph(): No nodes!" << std::endl; }

    for (auto& currentNode : m_nodes) {
        unsigned numInputs  = currentNode->numInputs();
        unsigned numOutputs = currentNode->numOutputs();
        printf("NODE: %s Inputs: %d Outputs:%d \n", currentNode->name.c_str(), numInputs, numOutputs);

        for (unsigned channelId = 0; channelId < numInputs; channelId++) {
            auto input = currentNode->getInput(channelId);
            if (input) {
                if (!(input->srcNodePtr)) {
                    std::string msg = "AudioGraph::debugPrintGraph(): an edge was found with no source\n";
                    if (input->destNodePtr) { msg += "destNodePtr: " + input->destNodePtr->name + " destChannel: " + std::to_string(input->destChannel); }
                    else { msg +=" destNodePtr: nullptr"; }
                    errorMessage(msg);
                } else {
                    printf("\tInput %d: %s:%d\n", channelId, input->srcNodePtr->name.c_str(), input->srcChannel);
                }
                //printf("\tInput %d: %s:%d\n", channelId, input->srcNodePtr->name.c_str(), input->srcChannel);
            }
            else { printf("\tInput %d: unconnected\n", channelId); }
        }

        for (unsigned channelId = 0; channelId < numOutputs; channelId++) {
            std::vector<std::shared_ptr<Edge>> outputVec = currentNode->getOutput(channelId);
            if (outputVec.size() > 0) {
                for (auto& edge : outputVec) {
                    if (edge) {
                        if (!(edge->destNodePtr)) {
                            std::string msg = "AudioGraph::debugPrintGraph(): an edge was found with no destination\n";
                            if (edge->srcNodePtr) { msg += "srcNodePtr: " + edge->srcNodePtr->name + " srcChannel: " + std::to_string(edge->srcChannel); }
                            else { msg +=" srcNodePtr: nullptr"; }
                            errorMessage(msg);
                        } else {
                            printf("\tOutput %d: %s:%d\n", channelId, edge->destNodePtr->name.c_str(), edge->destChannel);
                        }
                    } else { printf("\tOutput %d: unconnected\n", channelId); }
                }
            }
            else { printf("\tOutput %d: unconnected\n", channelId); }
        }
    }
}

bool Node::isStarter()
{
    if (m_inputs.size() < 1 && m_outputs.size() > 0) { return true; }
    else { return false; }
}

bool Node::isEnder() {
    if (m_outputs.size() < 1 && m_inputs.size() > 0) { return true; }
    else { return false; }
}

bool Node::isStranded() {
    if (m_outputs.size() < 1 && m_inputs.size() < 1) { return true; }
    else { return false; }
}

std::list<std::shared_ptr<Node>> AudioGraph::getTraversalList()
{
    std::list<std::shared_ptr<Node>> traversalList;
#if defined (JUCE_DEBUG)
    constexpr bool CHAIN_DEBUG = true;
#else
    constexpr bool CHAIN_DEBUG = false;
#endif

    if (CHAIN_DEBUG) debugPrintGraph();

    // First create a list of all starter node chains
    if (CHAIN_DEBUG) { std::cout << "*** Looking for Starter Nodes ***" << std::endl; }
    std::vector<std::list<std::shared_ptr<Node>>> chainsVec;
    for (std::shared_ptr<Node> node : m_nodes) {
        // If the node has no connected inputs or outputs, then mark it as visted because
        // it will be skipped.
        if (node->isStranded()) {
            node->visited = true;
            if (CHAIN_DEBUG) { std::cout << "Found stranded node: " << node->name << std::endl; }
        }
        else {
            if (node->isStarter()) {
                std::list<std::shared_ptr<Node>> newChain;
                newChain.push_back(node);
                chainsVec.push_back(newChain);
                if (CHAIN_DEBUG) { std::cout << "Found starter node: " << node->name << std::endl; }
            }
            node->visited = false;
        }
    }

    if (CHAIN_DEBUG) {
        std::cout << "*** Starter Nodes ***" << std::endl;
        for (auto& chains : chainsVec) {
            std::cout << chains.front()->name << std::endl;
        }
    }

    // Now begin a round-robin traversal through the chains
    if (CHAIN_DEBUG) { std::cout << "*** Node Traversal ***" << std::endl; }
    unsigned loopLimit = 100;
    while( (loopLimit--) > 0) {
        bool unvisitedInputsRemaining = false;
        unsigned visitedNodesCount = 0;

        unsigned idx = 0;
        if (chainsVec.size() < 1) { break; }
        for (auto& chain : chainsVec) { // loop over the chains
            if (CHAIN_DEBUG) { std::cout << "*** " << idx << ": chain size:" << chain.size() << std::endl; }
            idx++;
            if (chain.size() < 1) { continue; }

            for (auto currentNodePtr : chain) {
                if (!currentNodePtr) {
                    errorMessage("AudioGraph::getTraversalList(): a nullptr current Node was encountered during chain traversal"); continue;
                } else {
                    if (CHAIN_DEBUG) { std::cout << "processing " << currentNodePtr->name << " visited:" << currentNodePtr->visited << std::endl; }
                }

                // check to see if all it's inputs have been visted
                bool unvisitedInput = false;
                for (std::shared_ptr<Edge> inputEdge : currentNodePtr->getInputConnections()) {
                    // if an unvisited input is found, set the flag
                    // The dest for the Edge is this node, the src is the Node driving the input
                    if (!inputEdge) {
                        continue;
                    }
                    if (!inputEdge->srcNodePtr->visited) {
                        if (CHAIN_DEBUG) { std::cout << "Found and unvisited input, breaking input edge for loop" << std::endl; }
                        unvisitedInput = true; break;
                    }
                }
                if (unvisitedInput) { if (CHAIN_DEBUG) { printf("!!! continuing chain for loop\n");} unvisitedInputsRemaining = true; continue; }

                if (currentNodePtr->visited) {
                    continue;
                }

                // If all it's inputs have been visited we can add the current node to the traversal list
                // if it's not already added
                traversalList.push_back(currentNodePtr);
                currentNodePtr->visited = true;
                visitedNodesCount++;
                std::string msg = "ADDED " + currentNodePtr->name + " to the traversalList";
                noteMessage(msg);

                // Add all of it's outputs to the chain
                if (CHAIN_DEBUG) { std::cout << "This node has " << currentNodePtr->getOutputConnections().size() << " output edges" << std::endl; }
                for (std::shared_ptr<Edge> outputEdge : currentNodePtr->getOutputConnections()) {
                    if (!outputEdge) {
                        continue;
                    }
                    // For an output edge, the src is this node, the dest is the Node the output is driving
                    std::shared_ptr<Node> nextOutputNodePtr = outputEdge->destNodePtr;
                    if (!nextOutputNodePtr) {
                        std::string msg = "AudioGraph::getTraversalList(): " + currentNodePtr->name + ": a nullptr output Node was encountered during chain traversal";
                        errorMessage(msg); continue;
                    }

                    if (CHAIN_DEBUG) { std::cout << "Adding " << nextOutputNodePtr->name << " to the chain" << std::endl; }
                    chain.push_back(nextOutputNodePtr);
                }

                // remove the current node from the chain
                msg = "Removing " + chain.front()->name + " from the chain";
                noteMessage(msg);
                chain.pop_front();
                break; // we must break out everytime we modify the chain to restart it.
            }
        }

        if ((visitedNodesCount == 0) && !unvisitedInputsRemaining) { break; }  // we are finished when a pass through the chains results in no new visited nodes

    }

    if (CHAIN_DEBUG) { std::cout << "*** Traversal List ***" << std::endl; }
    for (auto nodePtr : traversalList) {
        if (!nodePtr) { errorMessage("ERROR: encountered nullptr Node in traversal list"); }
        if (CHAIN_DEBUG) { std::cout << nodePtr->name << std::endl; }
    }

    return traversalList;
}

}
