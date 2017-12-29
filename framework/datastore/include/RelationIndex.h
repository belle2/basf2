/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationIndexManager.h>

#include <boost/range/iterator_range.hpp>

namespace Belle2 {
  template <class T> class StoreArray;

  /** Provides access to fast ( O(log n) ) bi-directional lookups on a specified relation.
   *
   *  \note This class is used internally by RelationsObject/RelationsInterface. In most cases, you should avoid using RelationIndex directly and use these more friendly interfaces.
   *
   *  Relations connect objects stored in two StoreArrays with each other,
   *  with the possibility of n:n connections and individual weights.
   *
   *
   *  This class provides bidirectional access to a given relation to ease use
   *  of Relations for the normal user. There is no support for changing or adding
   *  Elements of the relation, this should be done directly using RelationArray.
   *
   *
   *  <h1>Finding related objects</h1>
   *  Given an object of type FROM or TO, RelationIndex can easily find objects
   *  on the other side connected to it. If there can be at most one relation
   *  from/to the given objects, this is especially simple:
   *  \code
      RelationIndex<MCParticle, CDCSimHit> mcparticlesToCdcsimhits;
      if(!mcparticlesToCdcsimhits)
        B2FATAL("No MCParticle -> CDCSimHit relation found!");

      typedef RelationIndex<MCParticle, CDCSimHit>::Element relElement_t;
      for(int iCDC = 0; iCDC < cdcsimhits.getEntries(); iCDC++) {
        const CDCSimHit* hit = cdcsimhits[iCDC];

        //assuming 'hit' was only created by a single particle
        const relElement_t* rel = mcparticlesToCdcsimhits.getFirstElementTo(hit);
        if(!rel) {
          B2WARNING("no MCParticle found for CDCSimHit " << iCDC);
          continue;
        }
        B2INFO("this CDCHit came from a particle with PDG code " << rel->from->getPDG());
      }
      \endcode
   *
   *  If more than one associated object exists, one can loop over them using
   *  range-based for. For example, when one instead wants to find the CDCSimHits
   *  belonging to a particle, one can use:
   *  \code
      const MCParticle* particle = ...;
      for(const relElement_t & rel: mcparticlesToCdcsimhits.getElementsFrom(particle)) {
        const CDCSimHit* hit = rel.to;
        //...
      }
      \endcode
   *
   *  The documentation of the relation element type used by getFirstElementTo()/
   *  getFirstElementFrom() or during the for loop can be found in
   *  RelationIndexContainer<FROM, TO>::Element.
   *
   *  \sa See RelationArray for examples on how to create new relations
   *      between objects.
   */
  template<class FROM, class TO> class RelationIndex {
  public:
    /** Struct representing a single element in the index. */
    typedef typename RelationIndexContainer<FROM, TO>::Element Element;

    /** Class representing a the index on the relation. */
    typedef typename RelationIndexContainer<FROM, TO>::ElementIndex ElementIndex;

    /** Typedef for easy access to the from side of the index. */
    typedef typename ElementIndex::template nth_index<0>::type index_from;

    /** Typedef for easy access to the to side of the index. */
    typedef typename ElementIndex::template nth_index<1>::type index_to;

    /** Element iterator of the from side index.
     *
     * @note Both iterator_from and iterator_to point to objects of type Element,
     *       but reflect the structure of the underlying index.
     * */
    typedef typename index_from::const_iterator iterator_from;

    /** Element iterator of the to side index. */
    typedef typename index_to::const_iterator iterator_to;

    /** Iterator range [first,second) of the from side. */
    typedef boost::iterator_range<iterator_from> range_from;

    /** Iterator range [first,second) of the to side. */
    typedef boost::iterator_range<iterator_to> range_to;

    /** Constructor.
     *
     *  @param name       Name of the relation. Empty string will be
     *                    replaced with the default relation name for
     *                    the given types
     *  @param durability Durabiliy of the relation
     */
    explicit RelationIndex(const std::string& name = (DataStore::defaultRelationName<FROM, TO>()),
                           DataStore::EDurability durability = DataStore::c_Event):
      m_index(RelationIndexManager::Instance().get<FROM, TO>(RelationArray(name, durability))),
      m_from(m_index->index().template get<0>()),
      m_to(m_index->index().template get<1>()) {}

    /** Constructor with checks.
     *
     *  This constructor takes the StoreArrays as arguments to perform
     *  additional sanity checks if the relation is correct.
     *
     *  @param from       StoreArray the relation is supposed to point from
     *  @param to         StoreArray the relation is supposed to point to
     *  @param name       Name of the relation. Empty string will be
     *                    replaced with the default relation name for
     *                    the given types
     *  @param durability Durabiliy of the relation
     */
    RelationIndex(const StoreArray<FROM>& from, const StoreArray<TO>& to, const std::string& name = "",
                  DataStore::EDurability durability = DataStore::c_Event):
      m_index(RelationIndexManager::Instance().get<FROM, TO>(RelationArray(from, to, name, durability))),
      m_from(m_index->index().template get<0>()),
      m_to(m_index->index().template get<1>()) {}

    /** check if index is based on valid relation. */
    operator bool() const { return *(m_index.get()); }

    /** Return a range of all elements pointing from the given object.
     *
     *  Can be used with range-based for, see RelationIndex class
     *  documentation for an example.
     *
     *  @param   from Pointer for which to get the relation.
     *  @returns Iterator range [first,second) of
     *           elements which point from this object.
     */
    range_from getElementsFrom(const FROM* from) const { return m_from.equal_range(from); }

    /** Return a range of all elements pointing from the given object.
     *
     *  Can be used with range-based for, see RelationIndex class
     *  documentation for an example.
     *
     *  @param from Reference for which to get the relation
     *  @returns Iterator range [first,second) of
     *           elements which point from this object
     */
    range_from getElementsFrom(const FROM& from) const { return m_from.equal_range(&from); }

    /** Return a range of all elements pointing to the given object.
     *
     *  Can be used with range-based for, see RelationIndex class
     *  documentation for an example.
     *
     *  @param to Pointer for which to get the relation
     *  @returns Iterator range [first,second) of
     *           elements which point to this object
     */
    range_to getElementsTo(const TO* to) const { return m_to.equal_range(to); }

    /** Return a range of all elements pointing to the given object.
     *
     *  Can be used with range-based for, see RelationIndex class
     *  documentation for an example.
     *
     *  @param to Reference for which to get the relation
     *  @returns Iterator range [first,second) of
     *           elements which point to this object
     */
    range_to getElementsTo(const TO& to) const { return m_to.equal_range(&to); }

    /** Return a pointer to the first relation Element of the given object.
     *
     *  Useful if there is at most one relation
     *  @param from Reference for which to get the Relation
     *  @returns Pointer to the RelationIndex<FROM,TO>::Element, can be
     *           NULL if no relation exists
     */
    const Element* getFirstElementFrom(const FROM& from) const { return getFirstElementFrom(&from); }

    /** Return a pointer to the first relation Element of the given object.
     *
     *  Useful if there is at most one relation
     *  @param from Pointer for which to get the Relation
     *  @returns Pointer to the RelationIndex<FROM,TO>::Element, can be
     *           NULL if no relation exists
     */
    const Element* getFirstElementFrom(const FROM* from) const
    {
      iterator_from it = m_from.find(from);
      if (it == m_from.end()) return 0;
      return &(*it);
    }

    /** Return a pointer to the first relation Element of the given object.
     *
     *  Useful if there is at most one relation
     *  @param to Reference for which to get the Relation
     *  @returns Pointer to the RelationIndex<FROM,TO>::Element, can be
     *           NULL if no relation exists
     */
    const Element* getFirstElementTo(const TO& to) const { return getFirstElementTo(&to); }

    /** Return a pointer to the first relation Element of the given object.
     *
     *  Useful if there is at most one relation
     *  @param to Pointer for which to get the Relation
     *  @returns Pointer to the RelationIndex<FROM,TO>::Element, can be
     *           NULL if no relation exists
     */
    const Element* getFirstElementTo(const TO* to) const
    {
      iterator_to it = m_to.find(to);
      if (it == m_to.end()) return 0;
      return &(*it);
    }

    /** Get the AccessorParams of the underlying relation. */
    const AccessorParams& getAccessorParams()     const { return m_index->getAccessorParams(); }

    /** Get the AccessorParams of the StoreArray the relation points from. */
    const AccessorParams& getFromAccessorParams() const { return m_index->getFromAccessorParams(); }

    /** Get the AccessorParams of the StoreArray the relation points to. */
    const AccessorParams& getToAccessorParams()   const { return m_index->getToAccessorParams(); }

    /** Get the size of the index. */
    size_t size() const { return m_index->index().size(); }
  protected:
    /** Reference to the IndexContainer. */
    const std::shared_ptr<RelationIndexContainer<FROM, TO>> m_index;

    /** Reference to the from index. */
    const index_from& m_from;

    /** Reference to the to index. */
    const index_to& m_to;
  };

} // end namespace Belle2
