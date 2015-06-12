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
   *
   * prerequesites for entryClass:
   * - has to have an == operator defined
   */
  template<typename EntryType>
  class DirectedNode {

    /** only the DirectedNodeNetwork can create DirectedNodes and link them */
    template<typename AnyType> friend class DirectedNodeNetwork;

  protected:
    /** ************************* DATA MEMBERS ************************* */

    /** entry can be of any type, DirectedNode is just the carrier */
    EntryType& m_entry;

    /** carries all links to inner nodes */
    std::vector<DirectedNode<EntryType>*> m_innerNodes;

    /** carries all links to outer nodes */
    std::vector<DirectedNode<EntryType>*> m_outerNodes;

    /** is the index position of this node in the network */
    unsigned int m_index;


    /** ************************* CONSTRUCTORS ************************* */

    /** protected constructor. accepts an entry which can not be changed any more */
    DirectedNode(EntryType& entry, unsigned int index) : m_entry(entry), m_index(index) {}


    /** copy constructor */
    DirectedNode(const DirectedNode& node) : m_entry(node.m_entry), m_innerNodes(node.m_innerNodes), m_outerNodes(node.m_outerNodes),
      m_index(node.m_index)
    {
      B2ERROR("DirectedNode-copy-constructor has been called!")
    }

    /** ************************* INTERNAL MEMBER FUNCTIONS ************************* */

    /** adds new links to the inward direction */
//  void addInnerNode(std::shared_ptr<DirectedNode<EntryType> > newNode)
    void addInnerNode(DirectedNode<EntryType>& newNode)
    {
      B2DEBUG(250, "DirectedNode::addInnerNode(): was called! OwnIndex/newInnerNodeIndex: " << m_index << "/" << newNode.getIndex() <<
              " and innerNodesSize: " << m_innerNodes.size() << ")!")
      m_innerNodes.push_back(&newNode);
//    auto* newPtr(&newNode);
//    B2WARNING("newPtr = " << newPtr << " and newPtr->getIndex: " << newPtr->getIndex())
//    m_innerNodes.push_back(newPtr); // WARNING ERROR does not work...

//    m_innerNodes.push_back(newNode.getIndex());
      B2DEBUG(250, "push_back of nodeIndex: " << m_index << " is over and has now innerNodesSize: " << m_innerNodes.size())
    }


    /** adds new links to the outward direction */
    void addOuterNode(DirectedNode<EntryType>& newNode)
    {
      B2DEBUG(250, "DirectedNode::addOuterNode(): was called! OwnIndex/outerNodeIndex: " << m_index << "/" << newNode.getIndex() <<
              " and innerNodesSize: " << m_innerNodes.size() << ")!")
      m_outerNodes.push_back(&newNode);
      B2DEBUG(250, "push_back of nodeIndex: " << m_index << " is over and has now innerNodesSize: " << m_innerNodes.size())
    }


    /** returns the index position of this node in the network */
    unsigned int getIndex() const { return m_index; }


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

    /** returns links to all inner nodes attached to this one */
    std::vector<DirectedNode<EntryType>*>& getInnerNodes() { return m_innerNodes; }
//     std::vector<std::shared_ptr<DirectedNode<EntryType> > >& getInnerNodes() { return m_innerNodes; }


    /** returns links to all outer nodes attached to this one */
    std::vector<DirectedNode<EntryType>*>& getOuterNodes() { return m_outerNodes; }


    /** allows access to stored entry */
    EntryType& getEntry() { return m_entry; }


    /** allows const access to stored entry (needed for external operator overload */
    const EntryType& getConstEntry() const { return m_entry; }


    /** returns Pointer to this node */
    DirectedNode<EntryType>* getPtr() { return this; }
  };


  /** ************************* NON-MEMBER FUNCTIONS ************************* */


  /** non-memberfunction Comparison for equality with EntryType <-> DirectedNode< EntryType > */
  template < typename EntryType>
  inline bool operator == (const EntryType& a, const DirectedNode<EntryType>& b)
  {
    return (a == b.getConstEntry());
  }
}
