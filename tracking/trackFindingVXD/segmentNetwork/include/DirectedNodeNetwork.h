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

// in fw:
#include <framework/logging/Logger.h>

#include <tracking/trackFindingVXD/segmentNetwork/DirectedNode.h>

// stl:
#include <set>
#include <unordered_map>



namespace Belle2 {

  template<typename EntryType, typename MetaInfoType>
  class DirectedNodeNetwork {
  public:
    /** typedef for more readable Node-Type */
    typedef DirectedNode<EntryType, MetaInfoType> Node;
    typedef std::string NodeID;

  protected:

    /** ************************* DATA MEMBERS ************************* */

    /** carries all nodes */
    std::unordered_map<NodeID, Node*> m_nodeMap;

    /** temporal storage for last outer node added, used for speed-up */
    NodeID m_lastOuterNodeID;


    /** temporal storage for last inner node added, used for speed-up */
    NodeID m_lastInnerNodeID;


    /** keeps track of current outerEnds (nodes which have no outerNodes) - entries are the index numbers of the nodes which currently form an outermost node */
    std::set<NodeID> m_outerEnds;


    /** keeps track of current innerEnds (nodes which have no innerNodes) - entries are the index numbers of the nodes which currently form an innermost node */
    std::set<NodeID> m_innerEnds;


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
      m_lastInnerNodeID() {}

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
      auto insertOp = m_nodeMap.insert({nodeID, tmpNode});
      if (insertOp.second) {
        return true;
      } else {
        delete tmpNode;
        return false;
      }
    }


    /** to the last outerNode added, another innerNode will be attached */
    void addInnerToLastOuterNode(NodeID innerNodeID)
    {
//    B2INFO("addInnerToLastOuterNode: inner: " << innerEntry); // TODO Jan8_2016: remove!
      if (m_lastOuterNodeID.empty()) { B2FATAL("addInnerToLastOuterNode() last OuterNode is not yet in this network! CurrentNetworkSize is: " << size()); }

      // check if entries are identical (catch loops):
      if (m_lastOuterNodeID == innerNodeID) {
        B2ERROR("DirectedNodeNetwork::addInnerToLastOuterNode(): lastOuterNode and innerEntry are identical! Aborting linking-process");
        return;
      }

      bool wasSuccessful = linkNodes(m_lastOuterNodeID, innerNodeID);

      if (wasSuccessful) {
        B2DEBUG(250, "DirectedNodeNetwork::addInnerToLastOuterNode(): linking was successful!");
        return;
      }
      B2ERROR("DirectedNodeNetwork::addInnerToLastOuterNode(): last OuterNode and innerEntry were already in the network and were already connected. This is a sign for unintended behavior!");
    }


    /** to the last innerNode added, another outerNode will be attached */
    void addOuterToLastInnerNode(NodeID outerNodeID)
    {
      if (m_lastInnerNodeID.empty()) { B2FATAL("addOuterToLastInnerNode() last InnerNode is not yet in this network! CurrentNetworkSize is: " << size()); }
      // check if entries are identical (catch loops):
      if (outerNodeID == m_lastInnerNodeID) {
        B2ERROR("DirectedNodeNetwork::addOuterToLastInnerNode(): outerEntry and lastInnerNode are identical! Aborting linking-process");
        return;
      }

      bool wasSuccessful = linkNodes(outerNodeID, m_lastInnerNodeID);

      if (wasSuccessful) {
        B2DEBUG(250, "DirectedNodeNetwork::addOuterToLastInnerNode(): linking was successful!");
        return;
      }
      B2ERROR("DirectedNodeNetwork::addOuterToLastInnerNode(): last InnerNode and outerEntry were already in the network and were already connected. This is a sign for unintended behavior!");
    }


    /** takes two entries and weaves them into the network TODO: UMBENENNEN! */
    bool linkNodes(NodeID outerNodeID, NodeID innerNodeID)
    {
      // check if entries are identical (catch loops):
      if (outerNodeID == innerNodeID) {
        B2ERROR("DirectedNodeNetwork::linkNodes(): outerNodeID and innerNodeID are identical! Aborting linking-process");
        return false;
      }
      if (m_nodeMap.count(innerNodeID) == 0 or m_nodeMap.count(outerNodeID) == 0) {
        B2ERROR("DirectedNodeNetwork::linkNodes(): Trying to link Nodes that are not present yet");
        return false;
      }
      B2DEBUG(10, "DNN:linkEntriesDEBUG: outer: " << outerNodeID << ", inner: " << innerNodeID);

      m_lastOuterNodeID = outerNodeID;
      m_lastInnerNodeID = innerNodeID;
      // update inner nodes list
      m_innerEnds.erase(outerNodeID);
      if (m_nodeMap[innerNodeID]->getInnerNodes().empty()) {m_innerEnds.insert(innerNodeID);}
      // update outer nodes list
      m_outerEnds.erase(innerNodeID);
      if (m_nodeMap[outerNodeID]->getOuterNodes().empty()) {m_outerEnds.insert(outerNodeID);}

      return createLink(*(m_nodeMap[outerNodeID]), *(m_nodeMap[innerNodeID]));
    }

/// getters:


    /** returns all nodes which have no outer nodes (but inner ones) and therefore are outer ends of the network */
    std::vector<Node*> getOuterEnds()
    {
      std::vector<Node*> outerEnds;
      outerEnds.reserve(m_outerEnds.size());
      for (NodeID nodeID : m_outerEnds) {
        outerEnds.push_back(m_nodeMap[nodeID]);
      }
      return outerEnds;
    }


    /** returns all nodes which have no inner nodes (but outer ones) and therefore are inner ends of the network */
    std::vector<Node*> getInnerEnds()
    {
      std::vector<Node*> innerEnds;
      innerEnds.reserve(m_innerEnds.size());
      for (NodeID nodeID : m_innerEnds) {
        innerEnds.push_back(m_nodeMap[nodeID]);
      }
      return innerEnds;
    }


    /** returns pointer to the node carrying the entry which is equal to given parameter. If no fitting entry was found, nullptr is returned. */
    Node* getNode(NodeID toBeFound)
    {
      if (m_nodeMap.count(toBeFound)) return m_nodeMap.at(toBeFound);
      else return nullptr;
    }

    //  std::vector<std::unique_ptr<Node> >& getNodes() { return m_nodes; }
    /** returns all nodes of the network */
    std::unordered_map<NodeID, Node* >& getNodes() { return m_nodeMap; }


    /** returns iterator for container: begin */
    //typename std::unordered_map< NodeID, Node* >::iterator begin() { return m_nodeMap.begin(); }


    /** returns iterator for container: end */
    //typename std::unordered_map< NodeID, Node* >::iterator end() { return m_nodeMap.end(); }


    /** returns number of nodes to be found in the network */
    unsigned int size() const { return m_nodeMap.size(); }


    /** check if a given entry is already in the network */
    bool isNodeInNetwork(const NodeID nodeID) const
    {
      return m_nodeMap.count(nodeID);
    }
  };
}
