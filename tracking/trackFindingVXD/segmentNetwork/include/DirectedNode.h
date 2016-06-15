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

// stl:
#include <vector>
// #include <memory>    // std::shared_ptr



namespace Belle2 {

  /** the node-class.
   *
   * carries an instance of a class which shall be woven into a network.
   * additionally a Cell can be attached for Cellular Automaton functionality.
   * If you don't need the Cell-features, just insert an empty dummy-class.
   * or use ../tracking/trackFindingVXD/segmentNetwork/VoidMetaInfo.h, where such a dummy class is defined.
   *
   * prerequesites for template EntryType:
   * - has to have an == operator defined
   * prerequisites for template MetaInfoType:
   * - has to have a public constructor with no arguments passed.
   */
  template<typename EntryType, typename MetaInfoType>
  class DirectedNode {

    /** only the DirectedNodeNetwork can create DirectedNodes and link them */
    template<typename AnyType, typename AnyOtherType> friend class DirectedNodeNetwork;

  protected:
    /** ************************* DATA MEMBERS ************************* */

    /** entry can be of any type, DirectedNode is just the carrier */
    EntryType& m_entry;

    /** carries all links to inner nodes */
    std::vector<DirectedNode<EntryType, MetaInfoType>*> m_innerNodes;

    /** carries all links to outer nodes */
    std::vector<DirectedNode<EntryType, MetaInfoType>*> m_outerNodes;

    /** is the index position of this node in the network */
    unsigned int m_index;

    /** contains a MetaInfo for doing extra-stuff (whatever you need) */
    MetaInfoType m_metaInfo;


    /** ************************* CONSTRUCTORS ************************* */

    /** protected constructor. accepts an entry which can not be changed any more */
    DirectedNode(EntryType& entry, unsigned int index) : m_entry(entry), m_index(index), m_metaInfo(MetaInfoType()) {}


    /** copy constructor */
    DirectedNode(const DirectedNode& node) : m_entry(node.m_entry), m_innerNodes(node.m_innerNodes), m_outerNodes(node.m_outerNodes),
      m_index(node.m_index), m_metaInfo(node.m_metaInfo)
    { B2ERROR("DirectedNode-copy-constructor has been called!"); }

    /** ************************* INTERNAL MEMBER FUNCTIONS ************************* */

    /** adds new links to the inward direction */
    //  void addInnerNode(std::shared_ptr<DirectedNode<EntryType, MetaInfoType> > newNode)
    void addInnerNode(DirectedNode<EntryType, MetaInfoType>& newNode)
    {
      B2DEBUG(10, "DirectedNode::addInnerNode(): was called! OwnIndex/newInnerNodeIndex: " << m_index << "/" << newNode.getIndex() <<
              " and innerNodesSize: " << m_innerNodes.size() << ")!");
      m_innerNodes.push_back(&newNode);
//    auto* newPtr(&newNode);
//    B2WARNING("newPtr = " << newPtr << " and newPtr->getIndex: " << newPtr->getIndex())
//    m_innerNodes.push_back(newPtr); // WARNING ERROR does not work...

//    m_innerNodes.push_back(newNode.getIndex());
      B2DEBUG(250, "push_back of nodeIndex: " << m_index << " is over and has now innerNodesSize: " << m_innerNodes.size());
    }


    /** adds new links to the outward direction */
    void addOuterNode(DirectedNode<EntryType, MetaInfoType>& newNode)
    {
      B2DEBUG(10, "DirectedNode::addOuterNode(): was called! OwnIndex/outerNodeIndex: " << m_index << "/" << newNode.getIndex() <<
              " and outerNodesSize: " << m_outerNodes.size() << ")!");
      B2DEBUG(10, "m_index is: " << m_index);
      m_outerNodes.push_back(&newNode);
      B2DEBUG(250, "push_back of nodeIndex: " << m_index << " is over and has now innerNodesSize: " << m_outerNodes.size());
    }




  public:

    /** ************************* OPERATORS ************************* */

    /** == -operator - compares if two nodes are identical */
    inline bool operator == (const DirectedNode& b) const { return (m_entry == b.getConstEntry()); }


    /** != -operator - compares if two nodes are not identical */
    inline bool operator != (const DirectedNode& b) const { return !(m_entry == b.getConstEntry()); }


    /** == -operator - compares if the entry passed is identical with the one linked in this node */
    inline bool operator == (const EntryType& b) const { return (m_entry == b); }


    /** == -operator - compares if the entry passed is not identical with the one linked in this node */
    inline bool operator != (const EntryType& b) const { return !(m_entry == b); }


    /** ************************* PUBLIC MEMBER FUNCTIONS ************************* */
/// getters
    /** returns the index position of this node in the network */
    unsigned int getIndex() const { return m_index; }

    /** returns links to all inner nodes attached to this one */
    std::vector<DirectedNode<EntryType, MetaInfoType>*>& getInnerNodes() { return m_innerNodes; }
//     std::vector<std::shared_ptr<DirectedNode<EntryType> > >& getInnerNodes() { return m_innerNodes; }


    /** returns links to all outer nodes attached to this one */
    std::vector<DirectedNode<EntryType, MetaInfoType>*>& getOuterNodes() { return m_outerNodes; }


    /** allows access to stored entry */
    EntryType& getEntry() { return m_entry; }


    /** allows const access to stored entry (needed for external operator overload */
    const EntryType& getConstEntry() const { return m_entry; }


    /** returns Pointer to this node */
    DirectedNode<EntryType, MetaInfoType>* getPtr() { return this; }


    /** returns reference to MetaInfoType attached to this node */
    MetaInfoType& getMetaInfo() { return m_metaInfo; }
  };

  /** ************************* NON-MEMBER FUNCTIONS ************************* */


  /** non-memberfunction Comparison for equality with EntryType <-> DirectedNode< EntryType > */
  template <class EntryType, class MetaInfoType>
  inline bool operator == (const EntryType& a, const DirectedNode<EntryType, MetaInfoType>& b)
  {
    return (a == b.getConstEntry());
  }
}
