/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Thomas Kuhr                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef STOREOBJPTR_H
#define STOREOBJPTR_H

#include <utility>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/Relation.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreAccessorAbs.h>

#include <framework/logging/Logger.h>

namespace Belle2 {

  /** Type save access pointer.
   *
   *  Use this, if you want to access or create a single object in the store.
   *
   *  @author <a href="mailto:martin.heck@kit.edu?subject=StoreObjPtr">Martin Heck</a>
   */
  template <class T>
  class StoreObjPtr : StoreAccessorAbs <TObject> {
  public:

    /** Constructor with assignment.
     *
     *  This constructor calls the assignObject function.
     *
     *  @param name       Name of the object to be hold by this pointer
     *  @param durability Decides durability map used for getting or creating the accessed object.
     *  @param generate   If true, object is created, if none exists with given name; points to existing object,
     *                    if one with given name AND type exists, and aborts otherwise. If false, it either points
     *                    to existing object with name AND type, or NULL.
     */
    StoreObjPtr(const std::string& name, const DataStore::EDurability& durability = DataStore::c_Event, bool generate = true) {
      if (assignObject(name, durability,  generate)) {
        B2DEBUG(100, name);
      };
    }

    /** Constructor, no assignment.
     *
     *  This contructor doesn't request a name. You can later assign an object to it, if you like.
     */
    StoreObjPtr()
        : m_storeObjPtr(0) {}

    /** Assigning an object to the pointer.
     *
     *  @return Was createObject called?
     *  @param name       Name of the object to be assigned.
     *  @param durability Decides durability map used for getting or creating the accessed object.
     *  @param generate   If true, object is created, if none exists with given name; points to existing object,
     *                    if one with given name AND type exists, and aborts otherwise. If false, it either points
     *                    to existing object with name AND type, or NULL.
     */
    bool assignObject(const std::string& name, const DataStore::EDurability& durability = DataStore::c_Event, bool generate = false);

    /** Store existing object.
     *
     *  Instead of creating a new obect, you can store an object, that already exists.
     *
     *  @return Could object be stored? False in case an object with given name already exists.
     *  @param name Name of the object to be stored.
     *  @param durability Decides durability map used to store the object.
     */
    bool storeObject(T* AObject, const std::string& name, const DataStore::EDurability& durability = DataStore::c_Event);

    /** Imitate pointer functionality. */
    T& operator *() const {return *m_storeObjPtr;};

    /** Imitate pointer functionality. */
    T* operator ->() const {return m_storeObjPtr;};

    /** Imitate pointer functionality. */
    operator bool() const {return m_storeObjPtr;};

    /** Returns the object as TObject.
     *
     * This function overwrites the inherited virtual function.
     */
    TObject* getPtr() {return m_storeObjPtr;}

    /** Convinient Relation creating.
     *
     *  Using this way to create Relations is safer than direct creation,
     *  because in this case you use definitively an object, that is already stored
     *  in the DataStore.
     */
    Relation* relateTo(const StoreAccessorAbs<TObject>& to, const float& weight = 1);

    /** Convenient RelationArray creating.
     *
     *  This way of creation can be used, if all weights are the same.
     */
    RelationArray* relateTo(const StoreAccessorAbs<TClonesArray>& to, const std::list<int>& indexList, const float& weight = 1);

    /** RelationArray creation in case of multiple weights. */
    RelationArray* relateTo(const StoreAccessorAbs<TClonesArray>& to, const std::list<std::pair<int, float> > indexWeightList);

  private:

    /** Store of actual pointer. */
    T* m_storeObjPtr;
  };

} // end namespace Belle2


// ------------ Implementation of template class ------------------------------------------------
template <class T>
bool StoreObjPtr<T>::assignObject(const std::string& name, const DataStore::EDurability& durability, bool generate)
{
  if (name == "") { B2FATAL("No Name was specified");}

  m_storeObjPtr =  DataStore::Instance().getObject<T>(name, durability);

  if (m_storeObjPtr) {
    return (false);
  }

  if (generate) {
    m_storeObjPtr = DataStore::Instance().createObject<T>(name, durability);
    return(true);
  }
  return (false);
}

template <class T>
bool StoreObjPtr<T>::storeObject(T* AObject, const std::string& name, const DataStore::EDurability& durability)
{
  if (name == "") { B2FATAL("No Name was specified");}

  m_storeObjPtr =  AObject;
  if (DataStore::Instance().storeObject(AObject, name, durability)) {
    return true;
  }
  return false;
}


template <class T>
Relation* StoreObjPtr<T>::relateTo(const StoreAccessorAbs<TObject>& to, const float& weight)
{
  return new Relation(m_storeObjPtr, to.getPtr(), weight);
}


template <class T>
RelationArray* StoreObjPtr<T>::relateTo(const StoreAccessorAbs<TClonesArray>& to, const std::list<int>& indexList, const float& weight)
{
  return RelationArray(m_storeObjPtr, to.getPtr(), indexList, weight);
}


template <class T>
RelationArray* StoreObjPtr<T>::relateTo(const StoreAccessorAbs<TClonesArray>& to, const std::list<std::pair<int, float> > indexWeightList)
{
  return RelationArray(m_storeObjPtr, to.getPtr(), indexWeightList);
}

#endif
