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

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  //! Type save access pointer.
  /*! Use this, if you want to access or create a single object in the store.
      \author <a href="mailto:martin.heck@kit.edu?subject=StoreObjPtr">Martin Heck</a>
  */
  template <class T>
  class StoreObjPtr {
  public:

    //! Constructor with assignment.
    /*! \param name Name of the object to be hold by this pointer
        \param durability Decides durability map used for getting or creating the accessed object.
        \param generate If true, object is created, if it doesn't exist with name AND type of template, points to NULL if one of
        different type with same name already exists, and points to existing object otherwise.
    */
    StoreObjPtr(const std::string& name, const EDurability& durability = c_Event, bool generate = true) {
      if (assignObject(name, durability,  generate)) {
        DEBUG(100, name);
      };
    }

    //! Constructor, no assignment
    /*! This contructor doesn't request a name. You can later assign an object to it, if you like.
    */
    StoreObjPtr()
        : m_storeObjPtr(0) {}

    //! Assigning an object to the pointer.
    /*! \param name Name of the object to be assigned.
        \param durability Decides durability map used for getting or creating the accessed object.
        \param generate If true, object is created, if it doesn't exist with name AND type of template, points to NULL if one of
        \return Was createObject called?
    */
    bool assignObject(const std::string& name, const EDurability& durability = c_Event, bool generate = false);

    //! Store existing object.
    /*! Instead of creating a new obect, you can store an object, that already exists.
        \param name Name of the object to be stored.
        \param durability Decides durability map used to store the object.
        \return Could object be stored? False in case an object with given name already exists.
    */
    bool storeObject(T* AObject, const std::string& name, const EDurability& durability = c_Event);

    //! imitate pointer functionality
    T& operator *() const {return *m_storeObjPtr;};

    //! imitate pointer functionality
    T* operator ->() const {return m_storeObjPtr;};

    //! imitate pointer functionality
    operator bool() const {return m_storeObjPtr;};

  private:

    //! Store of actual pointer
    T* m_storeObjPtr;
  };

} // end namespace Belle2


// ------------ Implementation of template class ------------------------------------------------
template <class T>
bool StoreObjPtr<T>::assignObject(const std::string& name, const EDurability& durability, bool generate)
{
  if (name == "") { FATAL("No Name was specified");}

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
bool StoreObjPtr<T>::storeObject(T* AObject, const std::string& name, const EDurability& durability)
{
  if (name == "") { FATAL("No Name was specified");}

  m_storeObjPtr =  AObject;
  if (DataStore::Instance().storeObject(AObject, name, durability)) {
    return true;
  }
  return false;
}
#endif
