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
#include <vector>
#include <algorithm>    // std::find
// #include <memory> // std::unique_ptr, std::shared_ptr // * WARNING This part is commented out until ROOT 6 is there!



namespace Belle2 {

  /// TODO optimize: the way this code is written, there is a high probability that Segments are recreated (and detected by the network) several times.
  /// since the network can handle them, it is not utterly wrong (because the results are fine and no clones will be stored) but it definitely costs some time.

  /** A network container where the nodes can carry any entryType.
   *
   * Features:
   * - All nodes store the same entryType and links can not carry any extra information.
   * - nodes can not be deactivated afterwards and links not deleted.
   * - can be walked through in a directed way (each node knows its inner and outer partners)
   * - inner and outer end-points of the networks are always automatically updated when filling the network
   * - one can not add single nodes, one always has to add pairs of nodes (this defines the link between them and which of them is the outer one).
   */
  template<typename EntryType, typename MetaInfoType>
  class DirectedNodeNetwork {
  public:
    /** ************************* PUBLIC TYPEDEFS ************************* */

    /** typedef for more readable Node-Type */
    typedef DirectedNode<EntryType, MetaInfoType> Node;

  protected:

    /** ************************* DATA MEMBERS ************************* */

    //  std::vector<std::unique_ptr<Node> > m_nodes;
    /** carries all nodes */
    std::vector< Node* > m_nodes;


    /** temporal storage for last outer node added, used for speed-up */
    Node* m_lastOuterNode;


    /** temporal storage for last inner node added, used for speed-up */
    Node* m_lastInnerNode;


    /** keeps track of current outerEnds (nodes which have no outerNodes) - entries are the index numbers of the nodes which currently form an outermost node */
    std::vector<unsigned int> m_outerEnds;


    /** keeps track of current innerEnds (nodes which have no innerNodes) - entries are the index numbers of the nodes which currently form an innermost node */
    std::vector<unsigned int> m_innerEnds;


    /** ************************* INTERNAL MEMBER FUNCTIONS ************************* */


    /** DEBUG: mini helper function for printing. */
//  template<class T>
//  static std::string miniPrinter(const std::vector<T>& vec)
//  {
//    std::string out = "had entries: ";
//    for (const T entry : vec) { out += std::to_string(entry) + " "; }
//    return out;
//  }


    /** make_unique, same as make_shared, but for unique_ptr. This is only needed because some people forgot to put make_unique into C++11.
     * WARNING This part is commented out until ROOT 6 is there! */
//  template<typename T, typename ...Args>
//  std::unique_ptr<T> make_unique( Args&& ...args )
//  {
//    return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
//  }


    /** internal function for adding Nodes */
    Node& addNode(EntryType& entry)
    {
      unsigned int index = m_nodes.size();
      //    m_nodes.push_back(make_unique<Node >(entry, index) );
      m_nodes.push_back(new Node(entry, index));
      return *m_nodes[index];
    }


    /** checks whether given 'toBeFound' is already in the vector.
    * returns iterator of toBeFound if found, returns givenVector.rend() if not
     */
    template<class T, class AnEntry>
    static typename std::vector<AnEntry >::reverse_iterator isInVector(const T& toBeFound, std::vector<AnEntry >& givenVector)
    {
      return std::find(givenVector.rbegin(), givenVector.rend(), toBeFound);
    }


    /** checks whether given 'toBeFound' is already in the network.
    * returns iterator of toBeFound if found, returns m_nodes.rend() if not
    * WARNING This part is commented out until ROOT 6 is there! */
//  template<class AnyType>
//  typename std::vector<std::unique_ptr<Node> >::reverse_iterator isInNetwork(const AnyType& toBeFound)
//  {
//    return std::find_if(m_nodes.rbegin(),
//              m_nodes.rend(),
//              [&] (const std::unique_ptr<Node>& vecEntry) -> bool {
//              Node& entryReference = *vecEntry;
//              return entryReference == toBeFound;
//              }
//    );
//  }


    /** checks whether given 'toBeFound' is already in the network.
    * returns iterator of toBeFound if found, returns m_nodes.rend() if not */
    template<class AnyType>
    typename std::vector< Node* >::reverse_iterator isInNetwork(const AnyType& toBeFound)
    {
      return std::find_if(m_nodes.rbegin(),
                          m_nodes.rend(),
                          [&toBeFound](const Node * vecEntry) -> bool
      { return *vecEntry == toBeFound; }
                         );
    }


    /** links nodes with each other. returns true if everything went well, returns false, if not  */
    static bool linkNodes(Node& outerNode, Node& innerNode)
    {
      // not successful if one of them was already added to the other one:
      if (std::find(outerNode.getInnerNodes().begin(), outerNode.getInnerNodes().end(), &innerNode) != outerNode.getInnerNodes().end()) { return false; }
      if (std::find(innerNode.getOuterNodes().begin(), innerNode.getOuterNodes().end(), &outerNode) != innerNode.getOuterNodes().end()) { return false; }

      outerNode.addInnerNode(innerNode);
      innerNode.addOuterNode(outerNode);
      return true;
    }


    /** adds newNode as a new end to the endVector and replaces old one if necessary. Returns true if network has been updated */
    static bool updateNetworkEnd(Node& oldNode,
                                 Node& newNode,
                                 std::vector<unsigned int>& endVector)
    {
      bool newIsInner = false;
      if (newNode.getInnerNodes().empty()) { newIsInner = true; }
//    B2WARNING("DirectedNodeNetwork::updateNetworkEnd(): before doing anything: endVector " << miniPrinter(endVector) << ". oldNodeIndex/newNodeIndex: " << oldNode.getIndex() << "/" << newNode.getIndex() << ", newNode isInner: " << (newIsInner ? "true" : "false") )

      auto iter = isInVector(oldNode.getIndex(), endVector);
      if (iter != endVector.rend()) { // old node was in endVector
        unsigned int oldIndex = *iter;
        *iter = newNode.getIndex();
        B2DEBUG(250, "DirectedNodeNetwork::updateNetworkEnd(): oldNode has index " << oldNode.getIndex() <<
                ", newNode (wasInnerNode: " << (newIsInner ? "true" : "false") <<
                ") has index: " << newNode.getIndex() <<
                ", oldNode was part of endVector and will therefore replaced (oldIndex was " << oldIndex <<
                " and newIndex is " << *iter <<
                ")!");
        return true;
      }
      B2DEBUG(250, "DirectedNodeNetwork::updateNetworkEnd(): oldNode has index " << oldNode.getIndex() <<
              ", newNode has (wasInnerNode: " << (newIsInner ? "true" : "false") <<
              ") index: " << newNode.getIndex() <<
              ", oldNode is not part of endVector and will therefore not replaced (endVector[0]/size() is: " << endVector.at(0) <<
              "/" << endVector.size() <<
              "), newNode will be added instead!");
      endVector.push_back(newNode.getIndex());
      return false;
    }


    /** to a given existing node, the newEntry will be added to the network if not there yet and they will be linked (returning true) if they weren't linked yet.
    * returns false if linking didn't work */
    bool addToExistingNode(Node* existingNode, EntryType& newEntry, bool newIsInner)
    {
      // check if entry is already in network.
      auto nodeIter = isInNetwork(newEntry);

      Node* outerNode = nullptr;
      Node* innerNode = nullptr;

      // assign nodePointers, create new nodes if needed:
      if (newIsInner) {
        outerNode = existingNode;
        if (nodeIter == m_nodes.rend()) { // new node was not in network yet
          innerNode = &addNode(newEntry);

          auto iterPos = std::find(m_innerEnds.begin(), m_innerEnds.end(), outerNode->getIndex());
          if (iterPos != m_innerEnds.end()) { *iterPos = innerNode->getIndex(); }
          else { m_innerEnds.push_back(innerNode->getIndex()); }

          B2DEBUG(250, "DirectedNodeNetwork::addToExistingNode(): newEntry is inner and new with index: " << innerNode->getIndex());
        } else {
          innerNode = *nodeIter;
        }
      } else {
        innerNode = existingNode;

        if (nodeIter == m_nodes.rend()) {
          outerNode = &addNode(newEntry);

          auto iterPos = std::find(m_outerEnds.begin(), m_outerEnds.end(), innerNode->getIndex());
          if (iterPos != m_outerEnds.end()) { *iterPos = outerNode->getIndex(); }
          else { m_outerEnds.push_back(outerNode->getIndex()); }

          B2DEBUG(250, "DirectedNodeNetwork::addToExistingNode(): newEntry is outer and new with index: " << outerNode->getIndex());
        } else {
          outerNode = *nodeIter;
        }
      }

      m_lastInnerNode = innerNode;
      m_lastOuterNode = outerNode;
      return linkNodes(*outerNode, *innerNode);
    }

  public:


    /** ************************* CONSTRUCTOR/DESTRUCTOR ************************* */


    /** standard constructor for ROOT IO */
    DirectedNodeNetwork() :
      m_lastOuterNode(nullptr),
      m_lastInnerNode(nullptr) {}


    /** destructor taking care of cleaning up the pointer-mess - WARNING only needed when using classic pointers for the nodes! */
    ~DirectedNodeNetwork()
    {
      for (Node* nodePointer : m_nodes) {
        delete nodePointer;
      }
      m_nodes.clear();
    }


    /** ************************* PUBLIC MEMBER FUNCTIONS ************************* */


    /** to the last outerNode added, another innerNode will be attached */
    void addInnerToLastOuterNode(EntryType& innerEntry)
    {
//    B2INFO("addInnerToLastOuterNode: inner: " << innerEntry); // TODO Jan8_2016: remove!
      if (m_lastOuterNode == nullptr) { B2FATAL("addInnerToLastOuterNode() last OuterNode is not yet in this network! CurrentNetworkSize is: " << size()); }

      // check if entries are identical (catch loops):
      if (*m_lastOuterNode == innerEntry) {
        B2ERROR("DirectedNodeNetwork::addInnerToLastOuterNode(): lastOuterNode and innerEntry are identical! Aborting linking-process");
        return;
      }

      bool wasSuccessful = addToExistingNode(m_lastOuterNode, innerEntry, true);

      if (wasSuccessful) {
        B2DEBUG(250, "DirectedNodeNetwork::addInnerToLastOuterNode(): linking was successful!");
        return;
      }
      B2ERROR("DirectedNodeNetwork::addInnerToLastOuterNode(): last OuterNode and innerEntry were already in the network and were already connected. This is a sign for unintended behavior!");
    }


    /** to the last innerNode added, another outerNode will be attached */
    void addOuterToLastInnerNode(EntryType& outerEntry)
    {
      if (m_lastOuterNode == nullptr) { B2FATAL("addOuterToLastInnerNode() last InnerNode is not yet in this network! CurrentNetworkSize is: " << size()); }
      // check if entries are identical (catch loops):
      if (outerEntry == *m_lastInnerNode) {
        B2ERROR("DirectedNodeNetwork::addOuterToLastInnerNode(): outerEntry and lastInnerNode are identical! Aborting linking-process");
        return;
      }

      bool wasSuccessful = addToExistingNode(m_lastInnerNode, outerEntry, false);

      if (wasSuccessful) {
        B2DEBUG(250, "DirectedNodeNetwork::addOuterToLastInnerNode(): linking was successful!");
        return;
      }
      B2ERROR("DirectedNodeNetwork::addOuterToLastInnerNode(): last InnerNode and outerEntry were already in the network and were already connected. This is a sign for unintended behavior!");
    }


    /** takes two entries and weaves them into the network TODO: UMBENENNEN! */
    void linkTheseEntries(EntryType& outerEntry, EntryType& innerEntry)
    {
      // check if entries are identical (catch loops):
      if (outerEntry == innerEntry) {
        B2ERROR("DirectedNodeNetwork::linkTheseEntries(): outerEntry and innerEntry are identical! Aborting linking-process");
        return;
      }
      B2DEBUG(10, "DNN:linkEntriesDEBUG: outer: " << outerEntry << ", inner: " << outerEntry);

      // check if entries are already in network.
      auto outerNodeIter = isInNetwork(outerEntry);
      auto innerNodeIter = isInNetwork(innerEntry);
//    B2INFO("linkTheseEntries: outer: " << outerEntry << ", inner: " << innerEntry); // TODO Jan8_2016: remove!

      /** case 1: none of the entries are added yet:
       *  create nodes for both and link with each other, where outerEntry will be carried by outer node and inner entry by inner node
       *  outerNode will be added to outerEnds, and innerNode will be added to innerEnds.
       * */
      if (outerNodeIter == m_nodes.rend() and innerNodeIter == m_nodes.rend()) {
        Node& newOuterNode = addNode(outerEntry);
        Node& newInnerNode = addNode(innerEntry);

        linkNodes(newOuterNode, newInnerNode);
        m_lastInnerNode = &newInnerNode;
        m_lastOuterNode = &newOuterNode;

        m_outerEnds.push_back(newOuterNode.getIndex());
        m_innerEnds.push_back(newInnerNode.getIndex());

        B2DEBUG(250, "DirectedNodeNetwork::linkTheseEntries(): outerNode is new with index " << newOuterNode.getIndex() <<
                ", innerNode is new with index: " << newInnerNode.getIndex());
        return;
      }

      /** case 2: the outerEntry was already in the network, but not innerEntry:
       *  add new node(innerEntry) to network
       *  add innerNode to existing outerNode
       *  add innerNode to innerEnds, if outerNode was in innerEnds before, replace old one with new innerNode
       * */
      if (outerNodeIter != m_nodes.rend() and innerNodeIter == m_nodes.rend()) {
        Node& outerNode = **outerNodeIter;
        Node& newInnerNode = addNode(innerEntry);

        linkNodes(outerNode, newInnerNode);
        m_lastInnerNode = &newInnerNode;
        m_lastOuterNode = &outerNode;

//      B2INFO("DirectedNodeNetwork::linkTheseEntries(): outerNode existed, innerNode not. B4 updateNetworkEnd, innerEnds: " << miniPrinter(m_innerEnds) << "\n outerEnds: " << miniPrinter(m_outerEnds))
        bool outerNodeReplaced = updateNetworkEnd(outerNode, newInnerNode, m_innerEnds);

        B2DEBUG(250, "DirectedNodeNetwork::linkTheseEntries(): outerNode was already there with index " << outerNode.getIndex() <<
                ", innerNode is new with index: " << newInnerNode.getIndex() <<
                ", outerNode was innerNode before: " << (outerNodeReplaced ? "true" : "false"));
        return;
      }

      /** case 3: outerNode was not there yet, but innerEntry was:
       *  add new node(outerEntry) to network
       *  add outerNode to existing innerNode
       *  add outerNode to outerEnds, if innerNode was in outerEnds before, replace old one with new outerNode
       * */
      if (outerNodeIter == m_nodes.rend() and innerNodeIter != m_nodes.rend()) {
        B2DEBUG(10, "DNN:linkEntriesDEBUG CASE 3: outer: " << outerEntry << ", inner: " << innerEntry);
        Node& innerNode = **innerNodeIter; // has to come first to prevent-realloc-issues
        Node& newOuterNode = addNode(outerEntry);
        B2DEBUG(10, "DNN:linkEntriesDEBUG CASE 3: inner: " << innerNode.getEntry());
        B2DEBUG(10, "DNN:linkEntriesDEBUG CASE 3: NEW outer: " << newOuterNode.getEntry());

        linkNodes(newOuterNode, innerNode);
        m_lastInnerNode = &innerNode;
        m_lastOuterNode = &newOuterNode;

//      B2INFO("DirectedNodeNetwork::linkTheseEntries(): innerNode existed, outerNode not. B4 updateNetworkEnd, innerEnds: " << miniPrinter(m_innerEnds) << "\n outerEnds: " << miniPrinter(m_outerEnds))
        bool innerNodeReplaced = updateNetworkEnd(innerNode, newOuterNode, m_outerEnds);

        B2DEBUG(250, "DirectedNodeNetwork::linkTheseEntries(): outerNode is new with index " << newOuterNode.getIndex() <<
                ", innerNode was already there with index: " << innerNode.getIndex() <<
                ", innerNode was outerNode before: " << (innerNodeReplaced ? "true" : "false"));
        return;
      }

      /** case 4: both are already in the network ... */
      Node& outerNode = **outerNodeIter;
      bool wasInnermostNode = (outerNode.getInnerNodes().empty()) ? true : false;
      Node& innerNode = **innerNodeIter;
      bool wasOutermostNode = (innerNode.getOuterNodes().empty()) ? true : false;

      bool wasSuccessful = linkNodes(outerNode, innerNode);

      /** case 4A: ... but were not linked to each other yet:
         *  add innerNode to existing outerNode
         *  add outerNode to existing innerNode
         *  if innerNode was in outerEnds before, replace old one with new outerNode
         *  if outerNode was in innerEnds before, replace old one with new innerNode
         * */
      if (wasSuccessful) {
        m_lastInnerNode = &innerNode;
        m_lastOuterNode = &outerNode;

        if (wasInnermostNode) {
          auto innerPos = std::find(m_innerEnds.begin(), m_innerEnds.end(), outerNode.getIndex());
          if (innerPos != m_innerEnds.end()) { m_innerEnds.erase(innerPos); }
        }

        if (wasOutermostNode) {
          auto outerPos = std::find(m_outerEnds.begin(), m_outerEnds.end(), innerNode.getIndex());
          if (outerPos != m_outerEnds.end()) { m_outerEnds.erase(outerPos); }
        }

        B2DEBUG(250, "DirectedNodeNetwork::linkTheseEntries(): outerNode already there with index " << outerNode.getIndex() <<
                ", innerNode was already there with index: " << innerNode.getIndex());
        return;
      }

      /** case 4B: both are already there and already linked: */
      B2ERROR("DirectedNodeNetwork::linkTheseEntries(): outerEntry and innerEntry were already in the network and were already connected. This is a sign for unintended behavior!");
    }

/// getters:


    /** returns all nodes which have no outer nodes (but inner ones) and therefore are outer ends of the network */
    std::vector<Node*> getOuterEnds()
    {
      std::vector<Node*> outerEnds;
      for (unsigned int index : m_outerEnds) {
//         outerEnds.push_back(m_nodes[index].get());
        outerEnds.push_back(m_nodes[index]->getPtr());
      }
      return outerEnds;
    }


    /** returns all nodes which have no inner nodes (but outer ones) and therefore are inner ends of the network */
    std::vector<Node*> getInnerEnds()
    {
      std::vector<Node*> innerEnds;
      for (unsigned int index : m_innerEnds) {
//    innerEnds.push_back(m_nodes[index].get());
        innerEnds.push_back(m_nodes[index]->getPtr());
      }
      return innerEnds;
    }


    /** returns pointer to the node carrying the entry which is equal to given parameter. If no fitting entry was found, nullptr is returned. */
    Node* getNode(EntryType& toBeFound)
    {
      auto iter = isInNetwork(toBeFound);
      if (iter == m_nodes.rend()) return nullptr;
//    return iter->get();
      return *iter;
    }


    /** returns pointer to the node carrying the entry which has the given index. If no fitting entry was found, nullptr is returned. */
    Node* getNodeWithIndex(unsigned int index)
    {
//    if (index < size()) { return m_nodes[index].get(); }
      if (index < size()) { return m_nodes[index]->getPtr(); }
      return nullptr;
    }


    /** returns pointer to the last outer node added. */
    Node* getLastOuterNode() { return m_lastOuterNode; }


    /** returns pointer to the last inner node added. */
    Node* getLastInnerNode() { return m_lastInnerNode; }


    //  std::vector<std::unique_ptr<Node> >& getNodes() { return m_nodes; }
    /** returns all nodes of the network */
    std::vector<Node* >& getNodes() { return m_nodes; }


    /** returns iterator for container: begin */
    typename std::vector< DirectedNode< EntryType, MetaInfoType>* >::iterator begin() { return m_nodes.begin(); }


    /** returns iterator for container: end */
    typename std::vector< DirectedNode< EntryType, MetaInfoType>* >::iterator end() { return m_nodes.end(); }


    /** returns number of nodes to be found in the network */
    unsigned int size() const { return m_nodes.size(); }


    /** check if a given entry is already in the network */
    bool isEntryInNetwork(const EntryType& entry) //const
    {
      // isInNetwork() cannot be declared const at the moment
      // TODO: make isInNetwork() const and declare this as const too
      return (isInNetwork(entry) != m_nodes.rend());
    }
  };
}
