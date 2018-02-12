/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// stl:
#include <vector>
#include <unordered_map>

// fw:
#include <framework/logging/Logger.h>

#include <tracking/trackFindingVXD/segmentNetwork/DirectedNode.h>


namespace Belle2 {

  template<typename EntryType, typename MetaInfoType>
  class DirectedNodeNetwork {
  public:
    /** typedef for more readable Node-Type */
    typedef DirectedNode<EntryType, MetaInfoType> Node;
    typedef std::int64_t NodeID; // NodeID should be some unique integer

  protected:

    /** ************************* DATA MEMBERS ************************* */

    /** carries all nodes */
    std::unordered_map<NodeID, Node*> m_nodeMap;

    /** temporal storage for last outer node added, used for speed-up */
    NodeID m_lastOuterNodeID;


    /** temporal storage for last inner node added, used for speed-up */
    NodeID m_lastInnerNodeID;

    /** keeps track of the state of the network to fill the vectors m_nodes, m_outerEnds, m_innerEnds only if required */
    bool m_isFinalized;

    /** After the network is finalized this vector will also carry all nodes to be able to keep the old interface.
     * This shouldn't affect the performance drastically in comparison to directly accessing the nodeMap.
     */
    std::vector<Node*> m_nodes;

    /** keeps track of current outerEnds (nodes which have no outerNodes) - entries are the NodeIDs of the nodes which currently form an outermost node */
    std::vector<Node*> m_outerEnds;


    /** keeps track of current innerEnds (nodes which have no innerNodes) - entries are the NodeIds of the nodes which currently form an innermost node */
    std::vector<Node*> m_innerEnds;


    /** ************************* INTERNAL MEMBER FUNCTIONS ************************* */


    /** links nodes with each other. returns true if everything went well, returns false, if not  */
    static bool createLink(Node& outerNode, Node& innerNode)
    {
      // not successful if one of them was already added to the other one:
      if (std::find(outerNode.getInnerNodes().begin(), outerNode.getInnerNodes().end(), &innerNode) != outerNode.getInnerNodes().end()) { return false; }
      if (std::find(innerNode.getOuterNodes().begin(), innerNode.getOuterNodes().end(), &outerNode) != innerNode.getOuterNodes().end()) { return false; }

      outerNode.addInnerNode(innerNode);
      innerNode.addOuterNode(outerNode);
      return true;
    }

  public:


    /** ************************* CONSTRUCTOR/DESTRUCTOR ************************* */


    DirectedNodeNetwork() :
      m_lastOuterNodeID(),
      m_lastInnerNodeID(),
      m_isFinalized(false) {}

    /** destructor taking care of cleaning up the pointer-mess - WARNING only needed when using classic pointers for the nodes! */
    ~DirectedNodeNetwork()
    {
      for (auto nodePointer : m_nodeMap) {
        delete nodePointer.second;
      }
      m_nodeMap.clear();
    }


    /** ************************* PUBLIC MEMBER FUNCTIONS ************************* */

    bool addNode(NodeID nodeID, EntryType& newEntry)
    {
      Node* tmpNode = new Node(newEntry);
      // use the specific emplace function instead of std::unordered_map::insert
      auto insertOp = m_nodeMap.emplace(nodeID, tmpNode);
      if (insertOp.second) {
        m_isFinalized = false;
        return true;
      } else {
        delete tmpNode;
        return false;
      }
    }


    /** to the last outerNode added, another innerNode will be attached */
    void addInnerToLastOuterNode(NodeID innerNodeID)
    {
      // check if entry does not exist, constructed with ID=-1
      if (m_lastOuterNodeID < 0) { B2WARNING("addInnerToLastOuterNode() last OuterNode is not yet in this network! CurrentNetworkSize is: " << size()); return;}
      // check if entries are identical (catch loops):
      if (m_lastOuterNodeID == innerNodeID) {
        B2WARNING("DirectedNodeNetwork::addInnerToLastOuterNode(): lastOuterNode and innerEntry are identical! Aborting linking-process");
        return;
      }

      bool wasSuccessful = linkNodes(m_lastOuterNodeID, innerNodeID);

      if (wasSuccessful) {
        B2DEBUG(250, "DirectedNodeNetwork::addInnerToLastOuterNode(): linking was successful!");
        return;
      }
      B2WARNING("DirectedNodeNetwork::addInnerToLastOuterNode(): last OuterNode and innerEntry were already in the network and were already connected. This is a sign for unintended behavior!");
    }


    /** to the last innerNode added, another outerNode will be attached */
    void addOuterToLastInnerNode(NodeID outerNodeID)
    {
      // check if entry does not exist, constructed with ID=-1
      if (m_lastInnerNodeID < 0) { B2WARNING("addOuterToLastInnerNode() last InnerNode is not yet in this network! CurrentNetworkSize is: " << size()); return; }
      // check if entries are identical (catch loops):
      if (outerNodeID == m_lastInnerNodeID) {
        B2WARNING("DirectedNodeNetwork::addOuterToLastInnerNode(): outerEntry and lastInnerNode are identical! Aborting linking-process");
        return;
      }

      bool wasSuccessful = linkNodes(outerNodeID, m_lastInnerNodeID);

      if (wasSuccessful) {
        B2DEBUG(250, "DirectedNodeNetwork::addOuterToLastInnerNode(): linking was successful!");
        return;
      }
      B2WARNING("DirectedNodeNetwork::addOuterToLastInnerNode(): last InnerNode and outerEntry were already in the network and were already connected. This is a sign for unintended behavior!");
    }


    /** takes two entry IDs and weaves them into the network */
    bool linkNodes(NodeID outerNodeID, NodeID innerNodeID)
    {
      m_isFinalized = false;
      // check if entries are identical (catch loops):
      if (outerNodeID == innerNodeID) {
        B2WARNING("DirectedNodeNetwork::linkNodes(): outerNodeID and innerNodeID are identical! Aborting linking-process");
        return false;
      }
      if (m_nodeMap.count(innerNodeID) == 0 or m_nodeMap.count(outerNodeID) == 0) {
        B2WARNING("DirectedNodeNetwork::linkNodes(): Trying to link Nodes that are not present yet");
        return false;
      }
      B2DEBUG(10, "DNN:linkEntriesDEBUG: outer: " << outerNodeID << ", inner: " << innerNodeID);

      m_lastOuterNodeID = outerNodeID;
      m_lastInnerNodeID = innerNodeID;

      return createLink(*(m_nodeMap[outerNodeID]), *(m_nodeMap[innerNodeID]));
    }

    /** Clear directed node network
     * Called to clear the directed node network if its size grows to large.
     * This is necessary to prevent to following modules from processing events with only partly filled networks.
     */
    void clear()
    {
      for (auto* node : m_nodes) { delete node; }
      m_nodes.clear();

      // Clearing the unordered_map is important as the following modules will process the event
      // if it still contains entries.
      for (auto nodePointer : m_nodeMap) {
        delete nodePointer.second;
      }
      m_nodeMap.clear();
    }

    /// getters:
    /** returns all nodes which have no outer nodes (but inner ones) and therefore are outer ends of the network */
    std::vector<Node*> getOuterEnds()
    {
      if (!m_isFinalized) finalize();
      return m_outerEnds;
    }


    /** returns all nodes which have no inner nodes (but outer ones) and therefore are inner ends of the network */
    std::vector<Node*> getInnerEnds()
    {
      if (!m_isFinalized) finalize();
      return m_innerEnds;
    }


    /** returns pointer to the node carrying the entry which is equal to given parameter. If no fitting entry was found, nullptr is returned. */
    Node* getNode(NodeID toBeFound)
    {
      if (m_nodeMap.count(toBeFound)) return m_nodeMap.at(toBeFound);
      else return nullptr;
    }

    /** returns all nodes of the network */
    std::vector<Node* >& getNodes()
    {
      if (!m_isFinalized) finalize();
      return m_nodes;
    }


    /** returns iterator for container: begin */
    typename std::vector<Node* >::iterator begin()
    {
      if (!m_isFinalized) finalize();
      return m_nodes.begin();
    }


    /** returns iterator for container: end */
    typename std::vector<Node* >::iterator end()
    {
      if (!m_isFinalized) finalize();
      return m_nodes.end();
    }


    /** returns number of nodes to be found in the network */
    unsigned int size() const { return m_nodeMap.size(); }


    /** check if a given entry is already in the network */
    bool isNodeInNetwork(const NodeID nodeID) const
    {
      return m_nodeMap.count(nodeID);
    }
  protected:

    void finalize()
    {
      if (m_isFinalized) return;
      m_nodes.clear();
      m_nodes.reserve(m_nodeMap.size());
      m_innerEnds.clear();
      m_outerEnds.clear();
      for (const auto& item : m_nodeMap) {
        m_nodes.push_back(item.second);
        if (item.second->getInnerNodes().empty()) m_innerEnds.push_back(item.second);
        if (item.second->getOuterNodes().empty()) m_outerEnds.push_back(item.second);
      }
      m_isFinalized = true;
    }
  };
}
