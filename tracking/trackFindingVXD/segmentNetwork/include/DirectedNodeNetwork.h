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

  /// TODO optimize: the way this pseudo code is written, there is a high probability that Segments are recreated (and detected by the network) several times.
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
  template<typename EntryType>
  class DirectedNodeNetwork {
  protected:

    /** ************************* DATA MEMBERS ************************* */

//  std::vector<std::unique_ptr<DirectedNode<EntryType>> > m_nodes;
    /** carries all nodes */
    std::vector< DirectedNode<EntryType>* > m_nodes;


    /** keeps track of current outerEnds (nodes which have no outerNodes) - entries are the index numbers of the nodes which currently form an outermost node */
    std::vector<unsigned int> m_outerEnds;


    /** keeps track of current innerEnds (nodes which have no innerNodes) - entries are the index numbers of the nodes which currently form an innermost node */
    std::vector<unsigned int> m_innerEnds;


    /** ************************* INTERNAL MEMBER FUNCTIONS ************************* */


    /** make_unique, same as make_shared, but for unique_ptr. This is only needed because some people forgot to put make_unique into C++11.
     * WARNING This part is commented out until ROOT 6 is there! */
//  template<typename T, typename ...Args>
//  std::unique_ptr<T> make_unique( Args&& ...args )
//  {
//    return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
//  }


    /** internal function for adding Nodes */
    DirectedNode<EntryType>& addNode(EntryType& entry)
    {
      unsigned int index = m_nodes.size();
//    m_nodes.push_back(make_unique<DirectedNode<EntryType> >(entry, index) );
      m_nodes.push_back(new DirectedNode<EntryType>(entry, index));
      return *m_nodes[index];
    }


    /** checks whether given 'toBeFound' is already in the vector.
    * returns iterator of toBeFound if found, returns givenVector.end() if not
     */
    template<class T, class AnEntry>
    static typename std::vector<AnEntry >::iterator isInVector(const T& toBeFound, std::vector<AnEntry >& givenVector)
    {
      return std::find(givenVector.begin(), givenVector.end(), toBeFound);
    }


    /** checks whether given 'toBeFound' is already in the vector of pointers to something.
    * returns iterator of toBeFound if found, returns givenVector.end() if not
    * WARNING This part is commented out until ROOT 6 is there! */
//  template<class AnyType>
//  typename std::vector<std::unique_ptr<DirectedNode<EntryType>> >::iterator isInVectorOfPtrs(const AnyType& toBeFound)
//  {
//    return std::find_if(m_nodes.begin(),
//              m_nodes.end(),
//              [&] (const std::unique_ptr<DirectedNode<EntryType>>& vecEntry) -> bool {
//              DirectedNode<EntryType>& entryReference = *vecEntry;
//              return entryReference == toBeFound;
//              }
//    );
//  }


    /** checks whether given 'toBeFound' is already in the vector of pointers to something.
    * returns iterator of toBeFound if found, returns givenVector.end() if not */
    template<class AnyType>
    typename std::vector< DirectedNode<EntryType>* >::iterator isInVectorOfPtrs(const AnyType& toBeFound)
    {
      return std::find_if(m_nodes.begin(),
                          m_nodes.end(),
                          [&](const DirectedNode<EntryType>* vecEntry) -> bool
      { return *vecEntry == toBeFound; }
                         );
    }


    /** links nodes with each other. returns true if everything went well, returns false, if not  */
    static bool linkNodes(DirectedNode<EntryType>& outerNode, DirectedNode<EntryType>& innerNode)
    {
      auto outerPos = std::find(outerNode.getInnerNodes().begin(), outerNode.getInnerNodes().end(), &innerNode);
      auto innerPos = std::find(innerNode.getOuterNodes().begin(), innerNode.getOuterNodes().end(), &outerNode);

      // not successful if one of them was already added to the other one:
      if (outerPos != outerNode.getInnerNodes().end() or innerPos != innerNode.getOuterNodes().end()) { return false; }

      outerNode.addInnerNode(innerNode);
      innerNode.addOuterNode(outerNode);
      return true;
    }

    /** adds newNode as a new end to the endVector and replaces old one if necessary */
    static bool updateNetworkEnd(DirectedNode<EntryType>& oldNode, DirectedNode<EntryType>& newNode,
                                 std::vector<unsigned int>& endVector)
    {
      auto iter = isInVector(oldNode.getIndex(), endVector);
      if (iter != endVector.end()) {
        unsigned int oldIndex = *iter;
        *iter = newNode.getIndex();
        B2WARNING("DirectedNodeNetwork::updateNetworkEnd(): oldNode has index " << oldNode.getIndex() <<
                  ", newNode has index: " << newNode.getIndex() << ", oldNode was part of endVector and will therefore replaced (oldIndex was " <<
                  oldIndex << " and newIndex is " << *iter << ")!")
        return true;
      }
      B2WARNING("DirectedNodeNetwork::updateNetworkEnd(): oldNode has index " << oldNode.getIndex() <<
                ", newNode has index: " << newNode.getIndex() <<
                ", oldNode is not part of endVector and will therefore not replaced (endVector[0]/size() is: " << endVector.at(
                  0) << "/" << endVector.size() << "), newNode will be added instead!")
      endVector.push_back(newNode.getIndex());
      return false;
    }
  public:


    /** ************************* DESTRUCTOR ************************* */


    /** standard constructor for ROOT IO */
    DirectedNodeNetwork() {}


    /** destructor taking care of cleaning up the pointer-mess - WARNING only needed when using classic pointers for the nodes! */
    ~DirectedNodeNetwork()
    {
      for (DirectedNode<EntryType>* nodePointer : m_nodes) {
        delete nodePointer;
      }
      m_nodes.clear();
    }


    /** ************************* PUBLIC MEMBER FUNCTIONS ************************* */

    /** takes two entries and weaves them into the network */
    void linkTheseEntries(EntryType& outerEntry, EntryType& innerEntry)
    {
      // check if entries are already in network.
      auto outerNodeIter = isInVectorOfPtrs(outerEntry);
      auto innerNodeIter = isInVectorOfPtrs(innerEntry);

      /** case: none of the entries are added yet:
       *  create nodes for both and link with each other, where outerEntry will be carried by outer node and inner entry by inner node
       *  outerNode will be added to outerEnds, and innerNode will be added to innerEnds.
       * */
      if (outerNodeIter == m_nodes.end() and innerNodeIter == m_nodes.end()) {
        DirectedNode<EntryType>& newOuterNode = addNode(outerEntry);
        DirectedNode<EntryType>& newInnerNode = addNode(innerEntry);

        linkNodes(newOuterNode, newInnerNode);

        m_outerEnds.push_back(newOuterNode.getIndex());
        m_innerEnds.push_back(newInnerNode.getIndex());

        B2WARNING("DirectedNodeNetwork::linkTheseEntries(): outerNode is new with index " << newOuterNode.getIndex() <<
                  ", innerNode is new with index: " << newInnerNode.getIndex())
        return;
      }

      /** case: the outerEntry was already in the network, but not innerEntry:
       *  add new node(innerEntry) to network
       *  add innerNode to existing outerNode
       *  add innerNode to innerEnds, if outerNode was in innerEnds before, replace old one with new innerNode
       * */
      if (outerNodeIter != m_nodes.end() and innerNodeIter == m_nodes.end()) {
        DirectedNode<EntryType>& outerNode = **outerNodeIter;
        DirectedNode<EntryType>& newInnerNode = addNode(innerEntry);

        linkNodes(outerNode, newInnerNode);

        bool outerNodeReplaced = updateNetworkEnd(outerNode, newInnerNode, m_innerEnds);

        B2WARNING("DirectedNodeNetwork::linkTheseEntries(): outerNode was already there with index " << outerNode.getIndex() <<
                  ", innerNode is new with index: " << newInnerNode.getIndex() <<
                  ", outerNode was innerNode before: " << (outerNodeReplaced ? "true" : "false"))
        return;
      }

      /** case: outerNode was not there yet, but innerEntry was:
       *  add new node(outerEntry) to network
       *  add outerNode to existing innerNode
       *  add outerNode to outerEnds, if innerNode was in outerEnds before, replace old one with new outerNode
       * */
      if (outerNodeIter == m_nodes.end() and innerNodeIter != m_nodes.end()) {
        DirectedNode<EntryType>& newOuterNode = addNode(outerEntry);
        DirectedNode<EntryType>& innerNode = **innerNodeIter;

        linkNodes(newOuterNode, innerNode);

        bool innerNodeReplaced = updateNetworkEnd(innerNode, newOuterNode, m_outerEnds);

        B2WARNING("DirectedNodeNetwork::linkTheseEntries(): outerNode is new with index " << newOuterNode.getIndex() <<
                  ", innerNode was already there with index: " << innerNode.getIndex() <<
                  ", innerNode was outerNode before: " << (innerNodeReplaced ? "true" : "false"))
        return;
      }

      /// case: both are already in the network ...
      DirectedNode<EntryType>& outerNode = **outerNodeIter;
      DirectedNode<EntryType>& innerNode = **innerNodeIter;

      bool wasSuccessful = linkNodes(outerNode, innerNode);

      /** ... but were not linked to each other yet:
       *  add innerNode to existing outerNode
       *  add outerNode to existing innerNode
       *  if innerNode was in outerEnds before, replace old one with new outerNode
       *  if outerNode was in innerEnds before, replace old one with new innerNode
       * */
      if (wasSuccessful) {
        bool outerNodeReplaced = updateNetworkEnd(outerNode, innerNode, m_innerEnds);
        bool innerNodeReplaced = updateNetworkEnd(innerNode, outerNode, m_outerEnds);

        B2WARNING("DirectedNodeNetwork::linkTheseEntries(): outerNode already there with index " << outerNode.getIndex() <<
                  ", innerNode was already there with index: " << innerNode.getIndex() <<
                  ", outerNode was innerNode before: " << (innerNodeReplaced ? ("true") : ("false"))  <<
                  ", innerNode was outerNode before: " << (outerNodeReplaced ? ("true") : ("false")))
        return;
      }

      /// case: both are already there and already linked:
      B2WARNING("DirectedNodeNetwork::linkTheseEntries(): outerEntry and innerEntry were already in the network and were already connected. This is a sign for unintended behavior!")
    }

/// getters:


    /** returns all nodes which have no outer nodes (but inner ones) and therefore are outer ends of the network */
    std::vector<DirectedNode<EntryType>*> getOuterEnds()
    {
      std::vector<DirectedNode<EntryType>*> outerEnds;
      for (unsigned int index : m_outerEnds) {
//         outerEnds.push_back(m_nodes[index].get());
        outerEnds.push_back(m_nodes[index]);
      }
      return outerEnds;
    }


    /** returns all nodes which have no inner nodes (but outer ones) and therefore are inner ends of the network */
    std::vector<DirectedNode<EntryType>*> getInnerEnds()
    {
      std::vector<DirectedNode<EntryType>*> innerEnds;
      for (unsigned int index : m_innerEnds) {
//    innerEnds.push_back(m_nodes[index].get());
        innerEnds.push_back(m_nodes[index]);
      }
      return innerEnds;
    }


    /** returns pointer to the node carrying the entry which is equal to given parameter. If no fitting entry was found, NULL is returned. */
    DirectedNode<EntryType>* getNode(EntryType& toBeFound)
    {
      auto iter = isInVectorOfPtrs(toBeFound);
      if (iter == m_nodes.end()) return NULL;
//    return iter->get();
      return *iter;
    }


//  std::vector<std::unique_ptr<DirectedNode<EntryType>> >& getNodes() { return m_nodes; }
    /** returns all nodes of the network */
    std::vector<DirectedNode<EntryType>* >& getNodes() { return m_nodes; }


    /** returns number of nodes to be found in the network */
    unsigned int size() const { return m_nodes.size(); }
  };
}
