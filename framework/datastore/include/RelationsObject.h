/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012-2014 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Christian Pulvermacher                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RELATIONSOBJECT_H
#define RELATIONSOBJECT_H

#include <framework/datastore/DataStore.h>
#include <framework/datastore/RelationVector.h>
#include <framework/datastore/RelationEntry.h>

#include <TObject.h>

#include <string>

namespace Belle2 {

  /** Defines interface for accessing relations of objects in StoreArray.
   *
   *  A class that wants to support the relations interface and is a subclass
   *  of BASE should be derived from RelationsInterface<BASE> instead.
   *  In most cases, BASE will be TObject and you can simply derive from RelationsObject.
   *
   *  Your class then provides methods like addRelationTo or getRelationsTo for
   *  an easy handling of relations to and from objects of this class.
   *  Retrieving relations this way is handled using auto-generated indices, and so should
   *  not introduce a large overhead.
   *
   *  <h1>Retrieving relations</h1>
   *  You can either retrieve a vector of relations using getRelations...(),
      \code
      //retrieve all CDCSimHits for given particle
      const MCParticle* particle = particles[i];
      RelationVector<CDCSimHit> cdcRelations = particle->getRelationsTo<CDCSimHit>();
      for (unsigned int iHit = 0; iHit < cdcRelations.size(); iHit++) {
        const CDCSimHit *simhit = cdcRelations[iHit];
        //...
      }
      \endcode
   *
   *  or, for 1:1 relations, the first related object (or NULL) using getRelated...():
   *
      \code
      //retrieve MCParticle (should be only one) for given simhit
      const CDCSimHit* simhit = cdcsimhits[i];
      const MCParticle* mcpart = simhit->getRelatedFrom<MCParticle>()
      if (!mcpart) {
        //nothing found, do some error handling here
      }
      \endcode
   *
   *  <h1>Adding relations</h1>
   *  Creating new relations is also fairly straightforward:
   *
      \code
      //given an MCParticle* particle:
      particle->addRelationTo(someOtherObject)
      \endcode

   *  Note that you'll have to register the relation in your module's initialize() function:
   *
      \code
      RelationArray::registerPersistent<MCParticle, SomeOtherClass>();
      \endcode
   */
  template <class BASE> class RelationsInterface: public BASE {
  public:

    /** Default constructor.
     */
    RelationsInterface(): m_cacheDataStoreEntry(NULL), m_cacheArrayIndex(-1) {}
#if defined(__CINT__) || defined(__ROOTCLING__) || defined(R__DICTIONARY_FILENAME)
#else
    /** Constructor, forwards all arguments to BASE constructor. */
    template<class ...Args> RelationsInterface(Args&& ... params) : BASE(std::forward<Args>(params)...), m_cacheDataStoreEntry(NULL), m_cacheArrayIndex(-1) { }
#endif

    /** Copy constructor.
     *
     *  Cached values are cleared.
     *  @param relationsInterface  The object that should be copied.
     */
    RelationsInterface(const RelationsInterface& relationsInterface): BASE(relationsInterface), m_cacheDataStoreEntry(0), m_cacheArrayIndex(-1) {}

    /** Assignment operator.
     *
     *  Cached values are cleared.
     *  @param relationsInterface  The object that should be assigned.
     */
    RelationsInterface& operator=(const RelationsInterface& relationsInterface);


    /** Add a relation from this object to another object (with caching).
     *
     *  @param object  The object to which the relation should point.
     *  @param weight  The weight of the relation.
     */
    void addRelationTo(const RelationsInterface<BASE>* object, double weight = 1.0) const {
      if (!object)
        return;
      DataStore::Instance().addRelation(this, m_cacheDataStoreEntry, m_cacheArrayIndex, object, object->m_cacheDataStoreEntry, object->m_cacheArrayIndex, weight);
    }

    /** Add a relation from this object to another object.
     *
     *  @param object  The object to which the relation should point.
     *  @param weight  The weight of the relation.
     */
    void addRelationTo(const TObject* object, double weight = 1.0) const {
      DataStore::Instance().addRelation(this, m_cacheDataStoreEntry, m_cacheArrayIndex, object, weight);
    }

    //====================================================================================================
    //These return an a vector of relations:

    /** Get the relations that point from this object to another store array.
     *
     *  @tparam TO     The class of objects to which the relations point.
     *  @param name    The name of the store array to which the relations point.
     *                 If empty the default store array name for class TO will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type TO are considered.
     *  @return        A vector of relations.
     */
    template <class TO> RelationVector<TO> getRelationsTo(const std::string& name = "") const {
      return RelationVector<TO>(DataStore::Instance().getRelationsWith(DataStore::c_ToSide, this, m_cacheDataStoreEntry, m_cacheArrayIndex, TO::Class(), name));
    }

    /** Get the relations that point from another store array to this object.
     *
     *  @tparam FROM   The class of objects from which the relations point.
     *  @param name    The name of the store array from which the relations point.
     *                 If empty the default store array name for class FROM will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type FROM are considered.
     *  @return        A vector of relations.
     */
    template <class FROM> RelationVector<FROM> getRelationsFrom(const std::string& name = "") const {
      return RelationVector<FROM>(DataStore::Instance().getRelationsWith(DataStore::c_FromSide, this, m_cacheDataStoreEntry, m_cacheArrayIndex, FROM::Class(), name));
    }

    /** Get the relations between this object and another store array.
     *
     *  Relations in both directions are returned.
     *
     *  @tparam T      The class of objects to or from which the relations point.
     *  @param name    The name of the store array to or from which the relations point.
     *                 If empty the default store array name for class T will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return        A vector of relations.
     */
    template <class T> RelationVector<T> getRelationsWith(const std::string& name = "") const {
      return RelationVector<T>(DataStore::Instance().getRelationsWith(DataStore::c_BothSides, this, m_cacheDataStoreEntry, m_cacheArrayIndex, T::Class(), name));
    }

    //====================================================================================================
    //These return only the first related object

    /** Get the object to which this object has a relation.
     *
     *  @tparam TO     The class of objects to which the relation points.
     *  @param name    The name of the store array to which the relation points.
     *                 If empty the default store array name for class TO will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type TO are considered.
     *  @return        The first related object or a null pointer.
     */
    template <class TO> TO* getRelatedTo(const std::string& name = "") const {
      return static_cast<TO*>(DataStore::Instance().getRelationWith(DataStore::c_ToSide, this, m_cacheDataStoreEntry, m_cacheArrayIndex, TO::Class(), name).object);
    }

    /** Get the object from which this object has a relation.
     *
     *  @tparam FROM   The class of objects from which the relation points.
     *  @param name    The name of the store array from which the relation points.
     *                 If empty the default store array name for class FROM will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type FROM are considered.
     *  @return        The first related object or a null pointer.
     */
    template <class FROM> FROM* getRelatedFrom(const std::string& name = "") const {
      return static_cast<FROM*>(DataStore::Instance().getRelationWith(DataStore::c_FromSide, this, m_cacheDataStoreEntry, m_cacheArrayIndex, FROM::Class(), name).object);
    }

    /** Get the object to or from which this object has a relation.
     *
     *  @tparam T      The class of objects to or from which the relation points.
     *  @param name    The name of the store array to or from which the relation points.
     *                 If empty the default store array name for class T will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return        The first related object or a null pointer.
     */
    template <class T> T* getRelated(const std::string& name = "") const {
      return static_cast<T*>(DataStore::Instance().getRelationWith(DataStore::c_BothSides, this, m_cacheDataStoreEntry, m_cacheArrayIndex, T::Class(), name).object);
    }


    //====================================================================================================
    //These return an object/weight pair

    /** Get first related object & weight of relation pointing to an array.
     *
     *  @tparam TO     The class of objects to which the relation points.
     *  @param name    The name of the store array to which the relation points.
     *                 If empty the default store array name for class TO will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type TO are considered.
     *  @return        Pair of first related object and the relation weight, or (NULL, 1.0) if none found.
     */
    template <class TO> std::pair<TO*, double> getRelatedToWithWeight(const std::string& name = "") const {
      RelationEntry entry = DataStore::Instance().getRelationWith(DataStore::c_ToSide, this, m_cacheDataStoreEntry, m_cacheArrayIndex, TO::Class(), name);
      return std::make_pair(static_cast<TO*>(entry.object), entry.weight);
    }

    /** Get first related object & weight of relation pointing from an array.
     *
     *  @tparam FROM   The class of objects from which the relation points.
     *  @param name    The name of the store array from which the relation points.
     *                 If empty the default store array name for class FROM will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type FROM are considered.
     *  @return        Pair of first related object and the relation weight, or (NULL, 1.0) if none found.
     */
    template <class FROM> std::pair<FROM*, double> getRelatedFromWithWeight(const std::string& name = "") const {
      RelationEntry entry = DataStore::Instance().getRelationWith(DataStore::c_FromSide, this, m_cacheDataStoreEntry, m_cacheArrayIndex, FROM::Class(), name);
      return std::make_pair(static_cast<FROM*>(entry.object), entry.weight);
    }

    /** Get first related object & weight of relation pointing from/to an array.
     *
     *  @tparam T      The class of objects to or from which the relation points.
     *  @param name    The name of the store array to or from which the relation points.
     *                 If empty the default store array name for class T will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return        Pair of first related object and the relation weight, or (NULL, 1.0) if none found.
     */
    template <class T> std::pair<T*, double> getRelatedWithWeight(const std::string& name = "") const {
      RelationEntry entry = DataStore::Instance().getRelationWith(DataStore::c_BothSides, this, m_cacheDataStoreEntry, m_cacheArrayIndex, T::Class(), name);
      return std::make_pair(static_cast<T*>(entry.object), entry.weight);
    }

    /** Get name of array this object is stored in, or "" if not found. */
    std::string getArrayName() const {
      DataStore::Instance().findStoreEntry(this, m_cacheDataStoreEntry, m_cacheArrayIndex);
      return m_cacheDataStoreEntry ? m_cacheDataStoreEntry->name : "";
    }


    /** Returns this object's array index (in StoreArray), or -1 if not found. */
    int getArrayIndex() const {
      DataStore::Instance().findStoreEntry(this, m_cacheDataStoreEntry, m_cacheArrayIndex);
      return m_cacheArrayIndex;
    }


#if defined(__CINT__) || defined(__ROOTCLING__) || defined(R__DICTIONARY_FILENAME)

    /**
     * @{
     * Define versions without template arguments, only available from python modules.
     */
    RelationVector<TObject> getRelationsTo(const std::string& name) const {
      return getRelationsTo<TObject>(name);
    }
    RelationVector<TObject> getRelationsFrom(const std::string& name) const {
      return getRelationsFrom<TObject>(name);
    }
    RelationVector<TObject> getRelationsWith(const std::string& name) const {
      return getRelationsWith<TObject>(name);
    }
    TObject* getRelatedTo(const std::string& name) const { return getRelatedTo<TObject>(name); }
    TObject* getRelatedFrom(const std::string& name) const { return getRelatedFrom<TObject>(name); }
    TObject* getRelated(const std::string& name) const { return getRelated<TObject>(name); }
    /** @} */
#endif

  private:

    /** Cache of the data store entry to which this object belongs. */
    mutable DataStore::StoreEntry* m_cacheDataStoreEntry;  //!

    /** Cache of the index in the TClonesArray to which this object belongs. */
    mutable int                    m_cacheArrayIndex;      //!

    friend class DataStore;

    //version 0 to disable streaming
    ClassDef(RelationsInterface, 0); /**< defines interface for accessing relations of objects in StoreArray. */
  };

  template <class BASE> RelationsInterface<BASE>& RelationsInterface<BASE>::operator = (const RelationsInterface<BASE>& relationsInterface)
  {
    if (this != &relationsInterface) {
      this->BASE::operator=(relationsInterface);
      m_cacheDataStoreEntry = NULL;
      m_cacheArrayIndex = -1;
    }
    return *this;
  }

  /** Typedef for relations interface for classes derived from TObject. */
  typedef RelationsInterface<TObject> RelationsObject;
}

#endif /* RELATIONSOBJECT_H */
