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

#include <vector>

#include <framework/logging/Logger.h>


namespace Belle2 {

  /** The Node-Class.
   *
   * Carries an instance of a class which shall be woven into a network.
   * additionally a Cell can be attached for Cellular Automaton functionality.
   * If you don't need the Cell-features, just insert an empty dummy-class.
   * or use ../tracking/trackFindingVXD/segmentNetwork/VoidMetaInfo.h, where such a dummy class is defined.
   *
   *  Prerequisites for template EntryType:
   * - has to have an == operator defined
   * prerequisites for template MetaInfoType:
   * - has to have a public constructor with no arguments passed.
   */
  template<typename EntryType, typename MetaInfoType>
  class DirectedNode {
    /** Only the DirectedNodeNetwork can create DirectedNodes and link them */
    template<typename AnyType, typename AnyOtherType> friend class DirectedNodeNetwork;

  protected:
    /** ************************* CONSTRUCTORS ************************* */
    /** Protected constructor. accepts an entry which can not be changed any more */
    DirectedNode(EntryType& entry) : m_entry(entry), m_metaInfo(MetaInfoType()), m_family(-1)
    {
      // Reserve some space for the vectors, TODO: can still be fine-tuned
      m_innerNodes.reserve(10);
      m_outerNodes.reserve(10);
    }


    /** copy constructor */
    DirectedNode(const DirectedNode& node) :
      m_entry(node.m_entry),
      m_innerNodes(node.m_innerNodes),
      m_outerNodes(node.m_outerNodes),
      m_metaInfo(node.m_metaInfo),
      m_family(-1)
    {
      B2ERROR("DirectedNode-copy-constructor has been called!");
    }


    /** ************************* INTERNAL MEMBER FUNCTIONS ************************* */
    /** Adds new links to the inward direction */
    void addInnerNode(DirectedNode<EntryType, MetaInfoType>& newNode)
    {
      m_innerNodes.push_back(&newNode);
    }


    /** Adds new links to the outward direction */
    void addOuterNode(DirectedNode<EntryType, MetaInfoType>& newNode)
    {
      m_outerNodes.push_back(&newNode);
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
    /// Getters
    /** Returns links to all inner nodes attached to this one */
    std::vector<DirectedNode<EntryType, MetaInfoType>*>& getInnerNodes() { return m_innerNodes; }

    /** Returns links to all outer nodes attached to this one */
    std::vector<DirectedNode<EntryType, MetaInfoType>*>& getOuterNodes() { return m_outerNodes; }

    /** Allows access to stored entry */
    EntryType& getEntry() { return m_entry; }

    /** Allows const access to stored entry (needed for external operator overload */
    const EntryType& getConstEntry() const { return m_entry; }

    /** Returns Pointer to this node */
    DirectedNode<EntryType, MetaInfoType>* getPtr() { return this; }

    /** Returns reference to MetaInfoType attached to this node */
    MetaInfoType& getMetaInfo() { return m_metaInfo; }

    /** Returns identifier of this cell */
    short getFamily() const { return m_family; }

    /** Assign a family identifier to this cell */
    void setFamily(short family) { m_family = family; }


    /** ************************* DATA MEMBERS ************************* */
    /** Entry can be of any type, DirectedNode is just the carrier */
    EntryType& m_entry;

    /** Carries all links to inner nodes */
    std::vector<DirectedNode<EntryType, MetaInfoType>*> m_innerNodes;

    /** Carries all links to outer nodes */
    std::vector<DirectedNode<EntryType, MetaInfoType>*> m_outerNodes;

    /** Contains a MetaInfo for doing extra-stuff (whatever you need) */
    MetaInfoType m_metaInfo;

    /** Identifier for all connected nodes */
    short m_family;
  };


  /** ************************* NON-MEMBER FUNCTIONS ************************* */
  /** Non-memberfunction Comparison for equality with EntryType <-> DirectedNode< EntryType > */
  template <class EntryType, class MetaInfoType>
  inline bool operator == (const EntryType& a, const DirectedNode<EntryType, MetaInfoType>& b)
  {
    return (a == b.getConstEntry());
  }
}
