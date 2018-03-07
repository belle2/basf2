/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreAccessorBase.h>
#include <framework/utilities/ArrayIterator.h>
#include <framework/dataobjects/RelationElement.h>
#include <framework/dataobjects/RelationContainer.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  template <class T> class StoreArray;

  /** Low-level class to create/modify relations between StoreArrays.
   *
   *  Relations connect objects stored in two StoreArrays with each other,
   *  with the possibility of n:n connections and individual weights.
   *
   *  \note In almost all cases, you'll want to create/find relations using RelationsObject instead.
   *
   *  <h1>Creating new relations</h1>
   *  Assuming you have two StoreArrays called 'particles' and 'cdcsimhits',
   *  you can use RelationArray to create relations between entries:
   *
   *  \code
      RelationArray particlesToCdchits(particles, cdcsimhits);
      for(int iPart = 0; iPart < particles.getEntries(); iPart++) {
        //... create new hit 'myhit'
        cdcsimhits.appendNew(myhit);
        int cdcsimhitIdx = cdcsimhits.getEntries()-1; //index of last object stored

        //connect objects at indices iPart and cdcsimhitIdx
        particlesToCdchits.add(iPart, cdcsimhitIdx);
      }
      \endcode
   *
   *  This example loops over the 'particles' array and might for example
   *  simulate the particles' interaction with the detector. New hits are
   *  added and also connected with the particles that created them.
   *
   *  As with other data store objects, you should register relations you want
   *  to store in your implementation of Module::initialize(), see StoreArray::registerRelationTo()
   *
   *
   *  \sa RelationsObject for the main user interface to relations.
   *  \sa RelationIndex provides an low-levvel interface to quickly find objects
   *      related to a given FROM/TO side object.
   *  \sa The on-disk data structure is provided by RelationElement objects
   *      in a RelationContainer.
   */
  class RelationArray: public StoreAccessorBase {
  public:
    /** STL-like const_iterator over the T objects (not T* ). */
    typedef ObjArrayIterator<const TClonesArray, const RelationElement> const_iterator;

    /** Typedef to simplify use of correct index_type */
    typedef RelationElement::index_type index_type;

    /** Typedef to simplify use of correct weight_type */
    typedef RelationElement::weight_type weight_type;

    /** Modification actions for the consolidate member.
     * Determines what is to be done with elements where the original
     * element was re-attributed and not just reordered
     */
    enum EConsolidationAction {
      c_doNothing, /**< Do nothing, just treat it as reordering */
      c_negativeWeight, /**< Flip the sign of the weight to become negative if the original element got re-attributed */
      c_zeroWeight, /**< Set the weight of the relation to 0 if the original element got re-attributed */
      c_deleteElement /**< Delete the whole relation element if the original element got re-attributed */
    };

    /** Typedef declaring the return value of any consolidation mapping.
     * It contains the new index of the element and a bool which is true if the
     * old element has been re-attributed and false if there was just an reordering.
     */
    typedef std::pair<index_type, bool> consolidation_type;

    /** Struct for identity transformation on indices.
     *
     * @see consolidate
     */
    struct Identity {
      /** Take old index and return the new index */
      consolidation_type operator()(index_type old) const { return std::make_pair(old, false); }
    };

    /** Struct to replace indices based on a map-like container.
     *
     *  Will keep old index if no replacement can be found
     *  @see consolidate
     */
    template < class MapType = std::map<index_type, consolidation_type> > class ReplaceMap {
    public:

      /** Set reference to used replacement map */
      explicit ReplaceMap(MapType& replace): m_replace(replace) {}

      /** Take old index and return the new index */
      consolidation_type operator()(index_type old) const
      {
        typename MapType::const_iterator iter = m_replace.find(old);
        if (iter != m_replace.end()) {
          return iter->second;
        }
        return std::make_pair(old, false);
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
    template < class VecType = std::vector<consolidation_type> > class ReplaceVec {
    public:

      /** Set reference to used replacement vector */
      explicit ReplaceVec(VecType& replace): m_replace(replace) {}

      /** Take old index and return the new index */
      consolidation_type operator()(index_type old) const { return m_replace[old];  }

    private:

      /** Reference of the used replacement vector */
      VecType& m_replace;
    };


    /** Create an empty relation array in the data store.
     *
     *  @param replace   Should an existing object be replaced?
     *  @return          True if the creation succeeded.
     **/
    bool create(bool replace = false)
    {
      bool result = DataStore::Instance().createObject(0, replace, *this);
      m_relations = reinterpret_cast<RelationContainer**>(DataStore::Instance().getObject(*this));
      if (result) {
        (*m_relations)->setFromName(m_accessorFrom.first);
        (*m_relations)->setFromDurability(m_accessorFrom.second);
        (*m_relations)->setToName(m_accessorTo.first);
        (*m_relations)->setToDurability(m_accessorTo.second);
      }
      return result;
    }

    /** Constructor which takes both store arrays and performs some sanity checks on the relation.
     *
     *  If the relation already exists it will be checked that the relation
     *  really relates between the given StoreArrays
     *
     *  @param from       StoreArray the relation points from
     *  @param to         StoreArray the relation points to
     *  @param name       Name of the relation. Default is to use the default
     *                   name based on the StoreArrays
     *  @param durability Durability of the relation. If the durability is larger than the
     *                   durability of the related StoreArrays, an error is raised.
     */
    template <class FROM, class TO> RelationArray(const StoreArray<FROM>& from, const StoreArray<TO>& to, const std::string& name = "",
                                                  DataStore::EDurability durability = DataStore::c_Event):
      StoreAccessorBase(name.empty() ? DataStore::relationName(from.getName(), to.getName()) : name, durability,
                        RelationContainer::Class(), false),
      m_accessorFrom(from.getAccessorParams()),
      m_accessorTo(to.getAccessorParams()),
      m_relations(0)
    {
      if (m_accessorFrom.second > m_durability || m_accessorTo.second > m_durability) {
        B2FATAL("Tried to create RelationArray '" << m_name << "' with a durability larger than the StoreArrays it relates");
      }
    }

    /** Constructor with AccessorParams for from- and to-side
     *
     *  @param fromAccessor (name, durability) of from side array
     *  @param toAccessor (name, durability) of to side array
     *  @param durability Durability of the relation. If the durability is larger than the
     *                   durability of the related StoreArrays, an error is raised.
     */
    RelationArray(const AccessorParams& fromAccessor, const AccessorParams& toAccessor,
                  DataStore::EDurability durability = DataStore::c_Event):
      StoreAccessorBase(DataStore::relationName(fromAccessor.first, toAccessor.first), durability, RelationContainer::Class(), false),
      m_accessorFrom(fromAccessor),
      m_accessorTo(toAccessor),
      m_relations(nullptr)
    {
      if (m_accessorFrom.second > m_durability || m_accessorTo.second > m_durability) {
        B2FATAL("Tried to create RelationArray '" << m_name << "' with a durability larger than the StoreArrays it relates");
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
    explicit RelationArray(const std::string& name, DataStore::EDurability durability = DataStore::c_Event):
      StoreAccessorBase(name, durability, RelationContainer::Class(), false),
      m_relations(0)
    {
      if (name.empty()) {
        B2FATAL("Cannot guess relation name, please supply correct name");
      }
    }

    /** Empty destructor */
    ~RelationArray() {}

    /** Check whether the object was created.
     *
     *  @return          True if the object exists.
     **/
    inline bool isValid() const { ensureAttached(); return m_relations && *m_relations;}
    /** Check whether the object was created. */
    inline operator bool()  const { return isValid(); }

    /** Imitate array functionality. */
    const RelationElement& operator[](int i) const { assertValid(); return (*m_relations)->getElement(i);}

    /** Get the number of elements. */
    int getEntries() const { return isValid() ? ((*m_relations)->getEntries()) : 0; }

    /** Return the AccessorParams the attached relation points from. */
    const AccessorParams& getFromAccessorParams() const
    {
      ensureAttached();
      if (m_accessorFrom.first.empty())
        B2FATAL("trying to get accessor params from non-existing relation (this is likely a framework bug)");
      return m_accessorFrom;
    }

    /** Return the AccessorParams the attached relation points to. */
    const AccessorParams& getToAccessorParams() const
    {
      ensureAttached();
      if (m_accessorTo.first.empty())
        B2FATAL("trying to get accessor params from non-existing relation (this is likely a framework bug)");
      return m_accessorTo;
    }

    /** Get modified flag of underlying container. */
    bool getModified() const { assertValid(); return (*m_relations)->getModified(); }

    /** Set modified flag of underlying container. */
    void setModified(bool modified) { assertCreated(); (*m_relations)->setModified(modified); }

    /** Clear all elements from the relation. */
    void clear() override
    {
      setModified(true);
      (*m_relations)->elements().Delete();
    }

    /** Add a new element to the relation
     *
     *  @param from    index to point from
     *  @param to      index to point to
     *  @param weight  weight of the relation
     */
    void add(index_type from, index_type to, weight_type weight = 1.0)
    {
      setModified(true);
      new(next()) RelationElement(from, to, weight);
    }

    /** Add a new element to the relation
     *
     *  @param from    index to point from
     *  @param to      indices to point to
     *  @param weight  weight for all relations
     */
    void add(index_type from, const std::vector<index_type>& to, weight_type weight = 1.0)
    {
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
    void add(index_type from, const std::vector<index_type>& to, const std::vector<weight_type>& weights)
    {
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
    template <class InputIterator> void add(index_type from, const InputIterator& begin, const InputIterator& end)
    {
      setModified(true);
      new(next()) RelationElement(from, begin, end);
    }

    /** Consolidate Relation Elements
     *
     *  This function will loop over the relation and "compress" it by
     *  merging all elements with the same fromIndex.
     */
    void consolidate() { consolidate<Identity, Identity>(); }

    /** Consolidate RelationElements
     *
     *  This function will loop over the relation and "compress" it by
     *  merging all elements with the same fromIndex.
     *
     *  The optional arguments can be used to replace indices in the
     *  relation, mainly useful in the simulation to replace the Geant4
     *  TrackID with the index of the MCParticle at the end of the event
     *
     *  The Function objects should return a new (index,bool) pair for every
     *  index in the old relation. The returned index will be used as the new
     *  index in the relation and the bool pair indicates whether the original
     *  element got re-attributed or if it is just an index reordering. If it
     *  the original element got re-attributed the behaviour depends on the
     *  value of action, see EConsolidationAction
     *
     *  Warning: If action is set to c_negativeWeight and there is more
     *  than one element having the same from->to index pair after
     *  transformation the behaviour is undefined as the two elements get
     *  merged. It could happen that a negative and a positive weight get
     *  summed up, leaving the weight rather meaningless
     *
     *  @see   Identity, ReplaceMap, ReplaceVec, EConsolidationAction
     *  @param replaceFrom  Function object containing replacements for the from
     *  indices
     *  @param replaceTo Function object containing replacements for
     *  the to indices
     *  @param action Action to be performed when the old
     *  element got removed
     */
    template<class FunctionFrom, class FunctionTo> void consolidate(const
        FunctionFrom& replaceFrom = FunctionFrom(), const FunctionTo&
        replaceTo = FunctionTo(), EConsolidationAction action =
          c_doNothing);

    /** Return const_iterator to first entry. */
    const_iterator begin() const { assertValid(); return const_iterator((*m_relations)->elements(), 0); }
    /** Return const_iterator to last entry +1. */
    const_iterator end() const { assertValid(); return const_iterator((*m_relations)->elements(), getEntries()); }

  private:
    /** Constructor which accepts the AccessorParams of the relation.
     *
     *  This constructor will only assign existing relations and will not create
     *  new ones since it lacks the information on the StoreArrays to relate
     *
     *  @param params     AccessorParams for the (existing) Relation
     */
    explicit RelationArray(const AccessorParams& params):
      StoreAccessorBase(params.first, params.second, RelationContainer::Class(), false),
      m_relations(0)
    {
      if (params.first.empty()) {
        B2FATAL("Cannot guess relation name, please supply correct name");
      }
    }


    /** Return address where the next RelationElement should be created. */
    RelationElement* next()
    {
      int index = (*m_relations)->elements().GetLast() + 1;
      return static_cast<RelationElement*>((*m_relations)->elements().AddrAt(index));
    }

    /** Check that the AccessorParams stored in the relation and the one given to the constructor are the same. */
    void checkRelation(const std::string& direction, const AccessorParams& array, const AccessorParams& rel) const
    {
      if (array.second == 0 && array.first.empty())
        return; //no information to check against...

      if (array != rel) {
        B2FATAL("Relation '" << m_name << "' exists but points " << direction << " wrong array:"
                << " requested " << array.first << "(" << array.second << ")"
                << ", got " << rel.first << "(" << rel.second << ")"
               );
      }
    }

    /** Attach to relation, if necessary. */
    void ensureAttached() const
    {
      if (m_relations)
        return;

      const_cast<RelationArray*>(this)->m_relations = reinterpret_cast<RelationContainer**>(DataStore::Instance().getObject(*this));
      if (m_relations && *m_relations && !(*m_relations)->isDefaultConstructed()) {
        AccessorParams fromAccessorRel((*m_relations)->getFromName(), (DataStore::EDurability)(*m_relations)->getFromDurability());
        AccessorParams toAccessorRel((*m_relations)->getToName(), (DataStore::EDurability)(*m_relations)->getToDurability());
        //set if unset
        if (m_accessorFrom.first.empty())
          const_cast<RelationArray*>(this)->m_accessorFrom = fromAccessorRel;
        if (m_accessorTo.first.empty())
          const_cast<RelationArray*>(this)->m_accessorTo = toAccessorRel;
        checkRelation("from", m_accessorFrom, fromAccessorRel);
        checkRelation("to", m_accessorTo, toAccessorRel);
      } else {
        //no relation found, mark as invalid
        const_cast<RelationArray*>(this)->m_relations = nullptr;
      }
    }

    /** check that pointer exits, otherwise bail out. */
    void assertValid() const { if (!isValid()) B2FATAL("RelationArray does not point to valid StoreObject"); }

    /** Create relation, if necessary.
     *
     *  After this function returns, the relation is guaranteed to be writeable.
     */
    void assertCreated()
    {
      if (!isValid()) {
        if (!create()) {
          B2FATAL("Couldn't create relation " << m_name << "!");
        }
      }
    }

    /** Accessor params for from array. */
    AccessorParams m_accessorFrom;

    /** Accessor params for to array. */
    AccessorParams m_accessorTo;

    /** Pointer that actually holds the relations. */
    RelationContainer** m_relations;

    template<class FROM, class TO> friend class RelationIndex;
    template<class FROM, class TO> friend class RelationIndexContainer;

  };

  template<class FunctionFrom, class FunctionTo>
  void RelationArray::consolidate(const FunctionFrom& replaceFrom, const FunctionTo& replaceTo, EConsolidationAction action)
  {
    if (!isValid()) {
      B2ERROR("Cannot consolidate an invalid relation (" << m_name << ")");
      return;
    }
    typedef std::map<index_type, weight_type> element_t;
    typedef std::map<index_type, element_t > buffer_t;
    buffer_t buffer;

    //Fill all existing elements in a nested map, adding the weights of
    //duplicate elements
    index_type lastFromIndex(0);
    buffer_t::iterator lastFromIter = buffer.end();
    unsigned int nElements = (*m_relations)->getEntries();
    TClonesArray& elements = (*m_relations)->elements();
    for (unsigned int i = 0; i < nElements; ++i) {
      RelationElement& element = *static_cast<RelationElement*>(elements[i]);
      //Replace from index
      consolidation_type from = replaceFrom(element.getFromIndex());

      //Ignore whole element if original element got deleted
      if (action == c_deleteElement && from.second) continue;

      //Check if the fromIndex is the same as the last one and reuse
      //iterator if possible
      if (from.first != lastFromIndex || lastFromIter == buffer.end()) {
        lastFromIter = buffer.insert(make_pair(from.first, element_t())).first;
        lastFromIndex = from.first;
      }
      //Loop over all elements of this relationelement and add them to the map
      size_t size = element.getSize();
      for (size_t j = 0; j < size; ++j) {
        //Replace to Index
        consolidation_type to = replaceTo(element.getToIndex(j));
        //Ignore whole element if original element got deleted
        if (action == c_deleteElement && to.second) continue;
        double weight = element.getWeight(j);
        //Original from or to element got deleted. Do whatever is specified by action
        //Warning: if there is more than one element pointing to the same
        //from->to element after transformation the negative weight option is
        //not safe as we sum a positive and a negative weight when
        //consolidating.
        if (from.second || to.second) {
          if (action == c_zeroWeight) {
            weight = 0;
          } else if (action == c_negativeWeight && weight > 0) {
            weight = -weight;
          }
        }
        //add the weight to the new from->to index pair
        lastFromIter->second[to.first] += weight;
      }
    }
    //Clear the existing relation
    elements.Delete();
    //Fill the map into the relation
    for (buffer_t::iterator iter = buffer.begin(); iter != buffer.end(); ++iter) {
      add(iter->first, iter->second.begin(), iter->second.end());
    }
  }

} // end namespace Belle2
