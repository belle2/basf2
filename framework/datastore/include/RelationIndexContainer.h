/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#ifndef RelationIndexContainer_H
#define RelationIndexContainer_H

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#include <string>

namespace Belle2 {

  /** Baseclass for all RelationIndexContainers.
   *
   *  This is an empty baseclass to allow storage of all template
   *  specializations to be stored in the same map
   *
   *  This class is only used internally, users should use RelationsObject/RelationsInterface to access/add relations.
   */
  class RelationIndexBase {
  public:

    /** Virtual destructor to create vtable. */
    virtual ~RelationIndexBase() {}
  };

  /** Class to store a bidirectional index between two StoreArrays.
   *
   *  This class provides a bidirectional access to a given Relation to ease
   *  use of Relations for the normal user. There is no support for changing
   *  or adding Relations. All instances of this class will be managed and
   *  created by the RelationIndexManager.
   *
   *  This class is only used internally, users should use RelationIndex/RelationArray to access/modify relations.
   */
  template<class FROM, class TO> class RelationIndexContainer: public RelationIndexBase {
  public:

    /** Element type for the index. */
    struct Element {

      /** Create a new element. */
      Element(RelationElement::index_type indexFrom,  RelationElement::index_type indexTo,
              const FROM* from, const TO* to,  RelationElement::weight_type weight):
        indexFrom(indexFrom), indexTo(indexTo), from(from), to(to), weight(weight) {}

      /** index of the element from which the relation points. */
      RelationElement::index_type indexFrom;

      /** index of the element to which the relation points. */
      RelationElement::index_type indexTo;

      /** pointer of the element from which the relation points. */
      const FROM* from;

      /** pointer of the element to which the relation points. */
      const TO* to;

      /** weight of the relation. */
      RelationElement::weight_type weight;
    };

    /** Boost MultiIndex container to keep the bidirectional index.
     *
     *  All the heavy lifting is done by this class
     */
    typedef boost::multi_index::multi_index_container <
    Element,
    boost::multi_index::indexed_by <
    boost::multi_index::ordered_non_unique <
    boost::multi_index::member<Element, const FROM*, &Element::from>
    > ,
    boost::multi_index::ordered_non_unique <
    boost::multi_index::member<Element, const TO*, &Element::to>
    >
    >
    > ElementIndex;

    /** Returns true if relation is valid */
    operator bool() const { return m_valid; }

    /** Get the index. */
    const ElementIndex&  index() const { return m_index; }
    /** Get the index. */
    ElementIndex&  index() { return m_index; }

    /** Get the AccessorParams of the underlying relation. */
    const AccessorParams& getAccessorParams()     const { return m_storeRel; }

    /** Get the AccessorParams of the StoreArray the relation points from. */
    const AccessorParams& getFromAccessorParams() const { return m_storeFrom; }

    /** Get the AccessorParams of the StoreArray the relation points to. */
    const AccessorParams& getToAccessorParams()   const { return m_storeTo; }

  protected:
    /** Constructor to create a new IndexContainer.
     *
     *  @param params pair of name and durability
     */
    RelationIndexContainer(const AccessorParams& params): m_storeRel(params), m_valid(false) {
      rebuild(true);
    }

    /** Restrict copies */
    RelationIndexContainer(const RelationIndexContainer&);

    /** Restrict copies */
    RelationIndexContainer& operator=(const RelationIndexContainer&);

    /** Rebuild the index.
     *
     *  @param force if force is true, the index will be rebuild even if the
     *               RelationArray says that it has not been modified
     */
    void rebuild(bool force = false);

    /** Instance of the index. */
    ElementIndex m_index;

    /** AccessorParams of the underlying relation. */
    AccessorParams m_storeRel;

    /** AccessorParams of the StoreArray the relation points from. */
    AccessorParams m_storeFrom;

    /** AccessorParams of the StoreArray the relation points to. */
    AccessorParams m_storeTo;

    /** Indicate wether the relation is valid. */
    bool m_valid;

    /** Allow the RelationIndexManager to create instances. */
    friend class RelationIndexManager;
  };

  /** Rebuild the index. */
  template<class FROM, class TO> void RelationIndexContainer<FROM, TO>::rebuild(bool force)
  {
    RelationArray storeRel(m_storeRel);
    m_valid = storeRel.isValid();
    if (!m_valid) {
      B2DEBUG(100, "Relation " << m_storeRel.first << " does not exist, cannot build index");
      m_index.clear();
      m_storeFrom = AccessorParams();
      m_storeTo = AccessorParams();
      return;
    }

    //Check if relation has been modified since we created the index
    //If not, keep old contents
    if (!force && !storeRel.getModified()) return;

    B2DEBUG(100, "Building index for " << m_storeRel.first);

    //Reset modification flag
    storeRel.setModified(false);

    //Clear index
    m_index.clear();

    //Get related StoreArrays
    m_storeFrom = storeRel.getFromAccessorParams();
    m_storeTo = storeRel.getToAccessorParams();
    const StoreArray<FROM> storeFrom(m_storeFrom.first, m_storeFrom.second);
    const StoreArray<TO>   storeTo(m_storeTo.first, m_storeTo.second);

    //Get number of entries in relation and stores
    unsigned int nRel = storeRel.getEntries();
    RelationElement::index_type nFrom = storeFrom.getEntries();
    RelationElement::index_type nTo = storeTo.getEntries();

    //Loop over all RelationElements and add them to index
    for (unsigned int i = 0; i < nRel; ++i) {
      const RelationElement& r = storeRel[i];
      RelationElement::index_type idxFrom = r.getFromIndex();
      if (idxFrom >= nFrom) B2FATAL("Relation " <<  m_storeRel.first << " is inconsistent: from-index (" << idxFrom << ") out of range");
      const FROM* from = storeFrom[idxFrom];

      //Loop over index and weight vector at once
      typedef std::vector< RelationElement::index_type> idx_t;
      typedef std::vector< RelationElement::weight_type> wgt_t;
      const idx_t& indices = r.getToIndices();
      const wgt_t& weights = r.getWeights();
      idx_t::const_iterator itIdx = indices.begin();
      wgt_t::const_iterator itWgt = weights.begin();
      for (; itIdx != indices.end() && itWgt != weights.end(); ++itIdx, ++itWgt) {
        RelationElement::index_type idxTo = *itIdx;
        if (idxTo >= nTo) B2FATAL("Relation " <<  m_storeRel.first << " is inconsistent: to-index (" << idxTo << ") out of range");
        const TO* to = storeTo[idxTo];
        m_index.insert(Element(idxFrom, idxTo, from, to, *itWgt));
      }
    }
  };

} // end namespace Belle2

#endif
