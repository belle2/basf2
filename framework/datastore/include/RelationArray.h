/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RELATIONARRAY_H
#define RELATIONARRAY_H

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/RelationContainer.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /** Class to access Relation from Datastore. */
  class RelationArray: public StoreObjPtr<RelationContainer> {
  public:

    /** Typedef to simplify use of correct index_type */
    typedef RelationElement::index_type index_type;

    /** Typedef to simplify use of correct weight_type */
    typedef RelationElement::weight_type weight_type;

    /** Struct for identity transformation on indices.
     *
     * @see consolidate
     */
    struct Identity {
      /** Take old index and return the new index */
      index_type operator()(index_type old) const { return old; }
    };


    /** Struct to replace indices based on a map-like container.
     *
     *  Will keep old index if no replacement can be found
     *  @see consolidate
     */
    template < class MapType = std::map<index_type, index_type> > class ReplaceMap {
    public:

      /** Set reference to used replacement map */
      ReplaceMap(MapType& replace): m_replace(replace) {}

      /** Take old index and return the new index */
      index_type operator()(index_type old) const {
        typename MapType::const_iterator iter = m_replace.find(old);
        if (iter != m_replace.end()) {
          return iter->second;
        }
        return old;
      }

    private:

      /** Reference of the used replacement map */
      MapType& m_replace;
    };


    /** Struct to replace indices based on a sequential container.
     *
     *  No range check is performed so make sure all indices are mapped
     *  @see consolidate
     */
    template < class VecType = std::vector<index_type> > class ReplaceVec {
    public:

      /** Set reference to used replacement vector */
      ReplaceVec(VecType& replace): m_replace(replace) {}

      /** Take old index and return the new index */
      index_type operator()(index_type old) const { return m_replace[old];  }

    private:

      /** Reference of the used replacement vector */
      VecType& m_replace;
    };

    /** Constructor which takes both store arrays and performs some sanity checks on the relation.
     *
     *  If the relation already exists it will be checked that the relation
     *  really relates between the given StoreArrays
     *
     *  @param from       StoreArray the relation points from
     *  @param to         StoreArray the relation points to
     *  @param name       Name of the relation. Default is to use the default
     *                   name based on the StoreArrays
     *  @param durability Durability of the relation. If the durability then the
     *                   ones from the related StoreArrays, an
     *                   error is raised
     */
    template <class FROM, class TO> RelationArray(const StoreArray<FROM>& from, const StoreArray<TO>& to, std::string name = "",
                                                  const DataStore::EDurability& durability = DataStore::c_Event, bool generate = true):
      StoreObjPtr<RelationContainer>(0) {

      const AccessorParams accessorFrom = from.getAccessorParams();
      const AccessorParams accessorTo   = to.getAccessorParams();
      if (accessorFrom.second > durability || accessorTo.second > durability) {
        B2FATAL("Tried to create Relation '" << name
                << "' with a durability larger than the Arrays it relates");
      }

      if (name == "") name = DataStore::defaultRelationName<FROM, TO>();
      if (assignObject(name, durability, generate)) {
        m_storeObjPtr->setFromName(accessorFrom.first);
        m_storeObjPtr->setFromDurability(accessorFrom.second);
        m_storeObjPtr->setToName(accessorTo.first);
        m_storeObjPtr->setToDurability(accessorTo.second);
      } else {
        if (!m_storeObjPtr) {
          B2ERROR("Could not find relation with name " << name);
          return;
        }
        checkRelation("from", accessorFrom, getFromAccessorParams());
        checkRelation("to", accessorTo, getToAccessorParams());
      }
    }

    /** Constructor which only accepts name and durability of the relation.
     *
     *  This constructor will only assign existing relations and will not create
     *  new ones since it lacks the information on the StoreArrays to relate
     *
     *  @param name       Name of the (existing) Relation
     *  @param durability Durability of the (existing) Relation
     */
    RelationArray(std::string name, const DataStore::EDurability& durability = DataStore::c_Event): StoreObjPtr<RelationContainer>(0) {
      if (name == "") {
        B2FATAL("Cannot guess relation name, please supply correct name");
        return;
      }
      assignObject(name, durability, false);
    }

    /** Constructor which accepts the AccessorParams of the relation.
     *
     *  This constructor will only assign existing relations and will not create
     *  new ones since it lacks the information on the StoreArrays to relate
     *
     *  @param params     AccessorParams for the (existing) Relation
     */
    RelationArray(const AccessorParams& params):
      StoreObjPtr<RelationContainer>(0) {
      if (params.first == "") {
        B2FATAL("Cannot guess relation name, please supply correct name");
        return;
      }
      assignObject(params.first, params.second, false);
    }

    /** Empty destructor */
    ~RelationArray() {}

    /** Imitate array functionality. */
    const RelationElement& operator[](int i) const { check(); return m_storeObjPtr->elements(i);}

    /** Get the number of elements. */
    int getEntries() const { check(); return m_storeObjPtr->getEntries(); }

    /** Return the AccessorParams the relation points from. */
    const AccessorParams getFromAccessorParams() const { check(); return AccessorParams(m_storeObjPtr->getFromName(), (DataStore::EDurability) m_storeObjPtr->getFromDurability()); }

    /** Return the AccessorParams the relation points to. */
    const AccessorParams getToAccessorParams()   const { check(); return AccessorParams(m_storeObjPtr->getToName(), (DataStore::EDurability) m_storeObjPtr->getToDurability()); }

    /** Get modified flag of underlying container. */
    bool getModified() { check(); return m_storeObjPtr->getModified(); }

    /** Set modified flag of underlying container. */
    void setModified(bool modified) { check(); m_storeObjPtr->setModified(modified); }

    /** Clear all elements from the relation. */
    void clear() {
      setModified(true);
      m_storeObjPtr->elements().Delete();
    }

    /** Add a new element to the relation
     *
     *  @param from    index to point from
     *  @param to      index to point to
     *  @param weight  weight of the relation
     */
    void add(index_type from, index_type to, weight_type weight = 1.0) {
      setModified(true);
      new(next()) RelationElement(from, to, weight);
    }

    /** Add a new element to the relation
     *
     *  @param from    index to point from
     *  @param to      indices to point to
     *  @param weight  weight for all relations
     */
    void add(index_type from, std::vector<index_type> to, weight_type weight = 1.0) {
      setModified(true);
      std::vector<weight_type> weights(to.size(), weight);
      new(next()) RelationElement(from, to, weights);
    }

    /** Add a new element to the relation
     *
     *  @param from    index to point from
     *  @param to      indices to point to
     *  @param weights weights of the relations
     */
    void add(index_type from, std::vector<index_type> to, std::vector<weight_type> weights) {
      setModified(true);
      new(next()) RelationElement(from, to, weights);
    }

    /** Add a new element to the relation
     *
     *  @param from    index to point from
     *  @param begin   iterator pointing to the begin of a sequence of
     *                 std::pair<index_type,weight_type> or compatible
     *  @param end     iterator pointing to the end of a sequence of
     *                 std::pair<index_type,weight_type> or compatible
     */
    template <class InputIterator> void add(index_type from, InputIterator begin, InputIterator end) {
      setModified(true);
      new(next()) RelationElement(from, begin, end);
    }

    /** Consolidate Relation Elements
     *
     *  This function will loop over the relation and "compress" it by
     *  merging all elements with the same fromIndex.
     */
    void consolidate() { consolidate<Identity, Identity>(); }

    /** Consolidate Relation Elements
     *
     *  This function will loop over the relation and "compress" it by
     *  merging all elements with the same fromIndex.
     *
     *  The optional arguments can be used to replace indicies in the
     *  relation, mainly useful in the simulation to replace the Geant4
     *  TrackID with the index of the MCParticle at the end of the event
     *
     *  @see   Identity, ReplaceMap, ReplaceVec
     *  @param replaceFrom Function object containing replacements for
     *                     the from indices
     *  @param replaceTo   Function object containing replacements for
     *                     the to indices
     */
    template<class FunctionFrom, class FunctionTo> void consolidate(
      const FunctionFrom& replaceFrom = FunctionFrom(),
      const FunctionTo& replaceTo = FunctionTo());

  protected:

    /** Imitate pointer functionality. */
    const TClonesArray& operator *() const {return m_storeObjPtr->elements();};

    /** Imitate pointer functionality. */
    const TClonesArray* operator ->() const {return &m_storeObjPtr->elements();};

    /** Return address where the next RelationElement should be created. */
    RelationElement* next() {
      int index = m_storeObjPtr->elements().GetLast() + 1;
      return static_cast<RelationElement*>(m_storeObjPtr->elements().AddrAt(index));
    }

    /** check that pointer exits, otherwise bail out. */
    void check() const { if (!m_storeObjPtr) B2FATAL("RelationArray does not point to valid StoreObject"); }

    /** Check that the AccessorParams stored in the relation and the one given to the constructor are the same. */
    void checkRelation(const std::string& direction, const AccessorParams& array, const AccessorParams& rel) {
      if (array != rel) {
        B2FATAL("Relation '" << m_name << "' exists but points " << direction << " wrong array:"
                << " requested " << array.first << "(" << array.second << ")"
                << ", got " << rel.first << "(" << rel.second << ")"
               );
      }
    }

  };

  template<class FunctionFrom, class FunctionTo>
  void RelationArray::consolidate(const FunctionFrom& replaceFrom, const FunctionTo& replaceTo)
  {
    if (!m_storeObjPtr) {
      B2ERROR("Cannot consilidate relation if pointer to container is NULL");
      return;
    }
    typedef std::map<index_type, weight_type> element_t;
    typedef std::map<index_type, element_t > buffer_t;
    buffer_t buffer;

    //Fill all existing elements in a nested map, adding the weights of
    //duplicate elements
    index_type lastFromIndex(-1);
    buffer_t::iterator lastFromIter = buffer.end();
    TClonesArray& elements = m_storeObjPtr->elements();
    unsigned int nElements = elements.GetEntries();
    for (unsigned int i = 0; i < nElements; ++i) {
      RelationElement& element = *static_cast<RelationElement*>(elements[i]);
      //Replace from index
      index_type from = replaceFrom(element.getFromIndex());

      //Check if the fromIndex is the same as the last one and reuse
      //iterator if possible
      if (from != lastFromIndex || lastFromIter == buffer.end()) {
        lastFromIter = buffer.insert(make_pair(from, element_t())).first;
        lastFromIndex = from;
      }
      //Loop over all elements of this relationelement and add them to the map
      size_t size = element.getSize();
      for (size_t j = 0; j < size; ++j) {
        //Replace to Index
        index_type to = replaceTo(element.getToIndex(j));
        lastFromIter->second[to] += element.getWeight(j);
      }
    }
    //Clear the existing relation
    elements.Delete();
    //Fill the map into the relation
    for (buffer_t::iterator iter = buffer.begin(); iter != buffer.end(); ++iter) {
      add(iter->first, iter->second.begin(), iter->second.end());
    }
  };

} // end namespace Belle2

#endif
