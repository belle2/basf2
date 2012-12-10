/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RELATIONSOBJECT_H
#define RELATIONSOBJECT_H

#include <framework/datastore/DataStore.h>

#include <TObject.h>

#include <vector>
#include <string>

namespace Belle2 {

  /** Relations interface class.
   *
   *  A class that wants to support the relations interface and is a subclass
   *  of BASE should be derived from RelationsInterface<BASE> instead.
   *  Then it provides methods like addRelationTo or getRelationsTo for
   *  an easy handling of relations to and from objects of this class.
   */
  template <class BASE> class RelationsInterface: public BASE {
  public:

    /** Default constructor.
     */
    RelationsInterface(): m_cacheDataStoreEntry(0), m_cacheArrayIndex(-1) {}

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
    RelationsInterface& operator = (const RelationsInterface& relationsInterface);


    /** Add a relation from this object to another object.
     *
     *  @param object  The object to which the relation should point.
     *  @param weight  The weight of the relation.
     *  @return        Flag whether the creation of the relation succeeded.
     */
    bool addRelationTo(const TObject* object, double weight = 1) const {
      return DataStore::Instance().addRelation(this, m_cacheDataStoreEntry, m_cacheArrayIndex, object, weight);
    }

    /** Get the relations that point from this object to another store array.
     *
     *  @tparam T      The class of objects to which the relations point.
     *  @param name    The name of the store array to which the relations point.
     *                 If empty the default store array name for class T will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return        A vector of relations.
     */
    template <class T> RelationVector<T> getRelationsTo(const std::string& name = "") const {
      return RelationVector<T>(DataStore::Instance().getRelationsFromTo(this, m_cacheDataStoreEntry, m_cacheArrayIndex, T::Class(), name));
    }

    /** Get the relations that point from another store array to this object.
     *
     *  @tparam T      The class of objects from which the relations point.
     *  @param name    The name of the store array from which the relations point.
     *                 If empty the default store array name for class T will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return        A vector of relations.
     */
    template <class T> RelationVector<T> getRelationsFrom(const std::string& name = "") const {
      return RelationVector<T>(DataStore::Instance().getRelationsToFrom(this, m_cacheDataStoreEntry, m_cacheArrayIndex, T::Class(), name));
    }

    /** Get the relations between this object and another store array.
     *
     *  Relations in both directions are returned. The direction is encoded in the sign
     *  of the weight. Relations that point to this object have a negative weight.
     *  @tparam T      The class of objects to or from which the relations point.
     *  @param name    The name of the store array to or from which the relations point.
     *                 If empty the default store array name for class T will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return        A vector of relations.
     */
    template <class T> RelationVector<T> getRelationsWith(const std::string& name = "") const {
      return RelationVector<T>(DataStore::Instance().getRelationsWith(this, m_cacheDataStoreEntry, m_cacheArrayIndex, T::Class(), name));
    }

    /** Get the object to which this object has a relation.
     *
     *  @tparam T      The class of objects to which the relation points.
     *  @param name    The name of the store array to which the relation points.
     *                 If empty the default store array name for class T will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return        The related object or a null pointer.
     */
    template <class T> const T* getRelatedTo(const std::string& name = "") const {
      return static_cast<const T*>(DataStore::Instance().getRelationFromTo(this, m_cacheDataStoreEntry, m_cacheArrayIndex, T::Class(), name).object);
    }

    /** Get the object from which this object has a relation.
     *
     *  @tparam T      The class of objects from which the relation points.
     *  @param name    The name of the store array from which the relation points.
     *                 If empty the default store array name for class T will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return        The related object or a null pointer.
     */
    template <class T> const T* getRelatedFrom(const std::string& name = "") const {
      return static_cast<const T*>(DataStore::Instance().getRelationToFrom(this, m_cacheDataStoreEntry, m_cacheArrayIndex, T::Class(), name).object);
    }

    /** Get the object to or from which this object has a relation.
     *
     *  @tparam T      The class of objects to or from which the relation points.
     *  @param name    The name of the store array to or from which the relation points.
     *                 If empty the default store array name for class T will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return        The related object or a null pointer.
     */
    template <class T> const T* getRelated(const std::string& name = "") const {
      return static_cast<const T*>(DataStore::Instance().getRelationWith(this, m_cacheDataStoreEntry, m_cacheArrayIndex, T::Class(), name).object);
    }

    /**
     * @{
     * define versions without template arguments, to be useful within python modules.
     */
    RelationVector<TObject> getRelationsTo(const std::string& name = "") const {
      return getRelationsTo<TObject>(name);
    }
    RelationVector<TObject> getRelationsFrom(const std::string& name = "") const {
      return getRelationsFrom<TObject>(name);
    }
    RelationVector<TObject> getRelationsWith(const std::string& name = "") const {
      return getRelationsWith<TObject>(name);
    }
    const TObject* getRelatedTo(const std::string& name = "") const {
      return getRelatedTo<TObject>(name);
    }
    const TObject* getRelatedFrom(const std::string& name = "") const {
      return getRelatedFrom<TObject>(name);
    }
    const TObject* getRelated(const std::string& name = "") const {
      return getRelated<TObject>(name);
    }
    /** @} */

  private:

    ClassDef(RelationsInterface, 0)

    /** Cache of the data store entry to which this object belongs. */
    mutable DataStore::StoreEntry* m_cacheDataStoreEntry;  //!

    /** Cache of the index in the TClonesArray to which this object belongs. */
    mutable int                    m_cacheArrayIndex;      //!
  };

  template <class BASE> RelationsInterface<BASE>& RelationsInterface<BASE>::operator = (const RelationsInterface<BASE>& relationsInterface)
  {
    if (this != &relationsInterface) {
      this->BASE::operator=(relationsInterface);
      m_cacheDataStoreEntry = 0;
      m_cacheArrayIndex = -1;
    }
    return *this;
  }

  /** Typedef for relations interface for classes derived from TObject. */
  typedef RelationsInterface<TObject> RelationsObject;
}

#endif /* RELATIONSOBJECT_H */
