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

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/RelationContainer.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /** Class to create/modify relations between StoreArrays.
   *
   *  Relations connect objects stored in two StoreArrays with each other,
   *  with the possibility of n:n connections and individual weights.
   *
   *  RelationArray is suitable for adding and modifying relations, see the
   *  documentation of RelationIndex for a more convenient way of finding and
   *  accessing relations.
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
   *  to store in your implementation of Module::initialize(), e.g. when creating
   *  a relation from MCParticles to SVDSimHits (with default name):
   *
   *  \code
      RelationArray::registerPersistent<MCParticle, SVDSimHit>();
      \endcode
   *
   * or, if you have arrays with non-default names:
   *  \code
      //create MCParticles (default name) -> MySVDSimHits relation
      RelationArray::registerPersistent<MCParticle, SVDSimHit>("", "MySVDSimHits");
      \endcode
   *
   *  \sa RelationIndex provides a convenient interface to finding objects
   *      related to a given FROM/TO side object.
   *  \sa The on-disk data structure is provided by RelationElement objects
   *      in a RelationContainer.
   */
  class RelationArray: public StoreAccessorBase {
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


    /** Register a relation array, that should be written to the output by default, in the data store.
     *  This must be called in the initialzation phase.
     *
     *  @param fromName    Name of from-array ("" for default name)
     *  @param toName      Name of to-array ("" for default name)
     *  @param durability  Specifies lifetime of array in question.
     *  @param errorIfExisting  Flag whether an error will be reported if the array was already registered.
     *  @return            True if the registration succeeded.
     */
    template<class FROM, class TO> static bool registerPersistent(const std::string& fromName, const std::string& toName, DataStore::EDurability durability = DataStore::c_Event,
        bool errorIfExisting = false) {
      const std::string& relName = DataStore::relationName(DataStore::arrayName<FROM>(fromName), DataStore::arrayName<TO>(toName));
      return DataStore::Instance().createEntry(relName, durability, RelationContainer::Class(), false, false, errorIfExisting);

    }
    /** Register a relation array, that should be written to the output by default, in the data store.
     *  This must be called in the initialzation phase.
     *
     *  @param durability  Specifies lifetime of array in question.
     *  @param errorIfExisting  Flag whether an error will be reported if the array was already registered.
     *  @return            True if the registration succeeded.
     */
    template<class FROM, class TO> static bool registerPersistent(DataStore::EDurability durability = DataStore::c_Event, bool errorIfExisting = false) {
      return registerPersistent<FROM, TO>("", "", durability, errorIfExisting);
    }
    /** Register a relation array, that should be written to the output by default, in the data store.
     *  This must be called in the initialzation phase.
     *
     *  @param name        Name under which the relation is stored.
     *  @param durability  Specifies lifetime of array in question.
     *  @param errorIfExisting  Flag whether an error will be reported if the array was already registered.
     *  @return            True if the registration succeeded.
     */
    static bool registerPersistent(const std::string& name, DataStore::EDurability durability = DataStore::c_Event,
                                   bool errorIfExisting = false) {
      return DataStore::Instance().createEntry(name, durability, RelationContainer::Class(), false, false, errorIfExisting);
    }

    /** Register a relation array, that should NOT be written to the output by default, in the data store.
     *  This must be called in the initialzation phase.
     *
     *  @param fromName    Name of from-array ("" for default name)
     *  @param toName      Name of to-array ("" for default name)
     *  @param durability  Specifies lifetime of array in question.
     *  @param errorIfExisting  Flag whether an error will be reported if the array was already registered.
     *  @return            True if the registration succeeded.
     */
    template<class FROM, class TO> static bool registerTransient(const std::string& fromName, const std::string& toName, DataStore::EDurability durability = DataStore::c_Event,
        bool errorIfExisting = false) {
      const std::string& relName = DataStore::relationName(DataStore::arrayName<FROM>(fromName), DataStore::arrayName<TO>(toName));
      return DataStore::Instance().createEntry(relName, durability, RelationContainer::Class(), false, true, errorIfExisting);
    }
    /** Register a relation array, that should NOT be written to the output by default, in the data store.
     *  This must be called in the initialzation phase.
     *
     *  @param durability  Specifies lifetime of array in question.
     *  @param errorIfExisting  Flag whether an error will be reported if the array was already registered.
     *  @return            True if the registration succeeded.
     */
    template<class FROM, class TO> static bool registerTransient(DataStore::EDurability durability = DataStore::c_Event,
        bool errorIfExisting = false) {
      return registerTransient<FROM, TO>("", "", durability, errorIfExisting);
    }
    /** Register a relation array, that should NOT be written to the output by default, in the data store.
     *  This must be called in the initialzation phase.
     *
     *  @param name        Name under which the relation is stored.
     *  @param durability  Specifies lifetime of array in question.
     *  @param errorIfExisting  Flag whether an error will be reported if the array was already registered.
     *  @return            True if the registration succeeded.
     */
    static bool registerTransient(const std::string& name, DataStore::EDurability durability = DataStore::c_Event,
                                  bool errorIfExisting = false) {
      return DataStore::Instance().createEntry(name, durability, RelationContainer::Class(), false, true, errorIfExisting);
    }

    /** Register the relation in the data store and include it in the output by default.
     *  This must be called in the initialization phase.
     *
     *  @param errorIfExisting  Flag whether an error will be reported if the array was already registered.
     *  @return            True if the registration succeeded.
     */
    bool registerAsPersistent(bool errorIfExisting = false) {
      return DataStore::Instance().createEntry(m_name, m_durability, RelationContainer::Class(), false, false, errorIfExisting);
    }

    /** Register the relation in the data store, but do not include it in the output by default.
     *  This must be called in the initialization phase.
     *
     *  @param errorIfExisting  Flag whether an error will be reported if the array was already registered.
     *  @return            True if the registration succeeded.
     */
    bool registerAsTransient(bool errorIfExisting = false) {
      return DataStore::Instance().createEntry(m_name, m_durability, RelationContainer::Class(), false, true, errorIfExisting);
    }

    /** Check whether a relation array was registered before.
     *
     *  It will cause an error if the object does not exist.
     *  You can use this in your module's initialize() function to require
     *  some relation to be registered. If it is absent, the execution will abort,
     *  allowing you to catch errors like missing modules early.
     *
     *  @param name        Name under which the relation array is stored.
     *  @param durability  Specifies lifetime of relation array in question.
     *  @return            True if the object exists.
     */
    static bool required(const std::string& name, DataStore::EDurability durability = DataStore::c_Event) {
      return DataStore::Instance().require(name, durability, RelationContainer::Class(), false);
    }

    /** Check whether the relation was registered before.
     *  It will cause an error if the relation does not exist.
     *  This must be called in the initialzation phase.
     *
     *  @return            True if the array exists.
     */
    bool isRequired() {
      return DataStore::Instance().require(m_name, m_durability, RelationContainer::Class(), false);
    }

    /** Create an empty relation array in the data store.
     *
     *  @param replace   Should an existing object be replaced?
     *  @return          True if the creation succeeded.
     **/
    bool create(bool replace = false) {
      bool result = DataStore::Instance().createObject(0, replace, m_name, m_durability, RelationContainer::Class(), false);
      m_relations = reinterpret_cast<RelationContainer**>(DataStore::Instance().getObject(m_name, m_durability, RelationContainer::Class(), false));
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
      StoreAccessorBase((name == "") ? DataStore::relationName(from.getName(), to.getName()) : name, durability),
      m_accessorFrom(from.getAccessorParams()),
      m_accessorTo(to.getAccessorParams()),
      m_relations(0) {
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
      StoreAccessorBase(name, durability),
      m_relations(0) {
      if (name == "") {
        B2FATAL("Cannot guess relation name, please supply correct name");
        return;
      }
    }

    /** Constructor which accepts the AccessorParams of the relation.
     *
     *  This constructor will only assign existing relations and will not create
     *  new ones since it lacks the information on the StoreArrays to relate
     *
     *  @param params     AccessorParams for the (existing) Relation
     */
    explicit RelationArray(const AccessorParams& params):
      StoreAccessorBase(params.first, params.second),
      m_relations(0) {
      if (params.first == "") {
        B2FATAL("Cannot guess relation name, please supply correct name");
        return;
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
    const RelationElement& operator[](int i) const { assertValid(); return (*m_relations)->elements(i);}

    /** Get the number of elements. */
    int getEntries() const { return isValid() ? ((*m_relations)->getEntries()) : 0; }

    /** Return the AccessorParams the attached relation points from. */
    const AccessorParams getFromAccessorParams() const { assertValid(); return AccessorParams((*m_relations)->getFromName(), (DataStore::EDurability)(*m_relations)->getFromDurability()); }

    /** Return the AccessorParams the attached relation points to. */
    const AccessorParams getToAccessorParams()   const { assertValid(); return AccessorParams((*m_relations)->getToName(), (DataStore::EDurability)(*m_relations)->getToDurability()); }

    /** Get modified flag of underlying container. */
    bool getModified() const { assertValid(); return (*m_relations)->getModified(); }

    /** Set modified flag of underlying container. */
    void setModified(bool modified) { assertCreated(); (*m_relations)->setModified(modified); }

    /** Clear all elements from the relation. */
    void clear() {
      setModified(true);
      (*m_relations)->elements().Delete();
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
    void add(index_type from, const std::vector<index_type>& to, weight_type weight = 1.0) {
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
    void add(index_type from, const std::vector<index_type>& to, const std::vector<weight_type>& weights) {
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

    /** Consolidate RelationElements
     *
     *  This function will loop over the relation and "compress" it by
     *  merging all elements with the same fromIndex.
     *
     *  The optional arguments can be used to replace indices in the
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

  private:

    /** Return address where the next RelationElement should be created. */
    RelationElement* next() {
      int index = (*m_relations)->elements().GetLast() + 1;
      return static_cast<RelationElement*>((*m_relations)->elements().AddrAt(index));
    }

    /** Check that the AccessorParams stored in the relation and the one given to the constructor are the same. */
    void checkRelation(const std::string& direction, const AccessorParams& array, const AccessorParams& rel) const {
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
    void ensureAttached() const {
      if (m_relations) return; //TODO: unsure about this...

      const_cast<RelationArray*>(this)->m_relations = reinterpret_cast<RelationContainer**>(DataStore::Instance().getObject(m_name, m_durability, RelationContainer::Class(), false));
      if (m_relations && *m_relations) {
        if (isValid() && (*m_relations)->isDefaultConstructed()) {
          //no relation found, mark as invalid
          const_cast<RelationArray*>(this)->m_relations = 0;
          return;
        }
        checkRelation("from", m_accessorFrom, getFromAccessorParams());
        checkRelation("to", m_accessorTo, getToAccessorParams());
      }
    }

    /** check that pointer exits, otherwise bail out. */
    void assertValid() const { if (!isValid()) B2FATAL("RelationArray does not point to valid StoreObject"); }

    /** Create relation, if necessary.
     *
     *  After this function returns, the relation is guaranteed to be writeable.
     */
    void assertCreated() {
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

  };

  template<class FunctionFrom, class FunctionTo>
  void RelationArray::consolidate(const FunctionFrom& replaceFrom, const FunctionTo& replaceTo)
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
  }

} // end namespace Belle2

#endif
