/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef STOREOBJPTR_H
#define STOREOBJPTR_H

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreAccessorBase.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /** Type save access pointer.
   *
   *  Use this, if you want to access or create a single object in the store.
   *  @author <a href="mailto:belle2_software@bpost.kek.jp?subject=StoreObjPtr">The basf2 developers</a>
   */
  template <class T> class StoreObjPtr : StoreAccessorBase {
  public:
    /** Constructor with assignment.
     *
     *  This constructor calls the assignObject function.
     *  @param name       Name under which the object to be hold by this pointer is stored.
     *  @param durability Decides durability map used for getting or creating the accessed object.
     *  @param generate   Shall an object in the DataStore be created, if none exists with given name and durability?
     *  @sa assignObject
     */
    explicit StoreObjPtr(const std::string& name = "", const DataStore::EDurability& durability = DataStore::c_Event, bool generate = true) {
      assignObject(name, durability,  generate);
    }

    /** Constructor for usage with Relations etc.
     *
     *  No new objects are created when using this constructor.
     *  @param accessorParams   A pair with name and durability.
     */
    StoreObjPtr(AccessorParams accessorParams) {
      assignObject(accessorParams.first, accessorParams.second, false);
    }

    /** Constructor, no assignment.
     *
     *  The default constructor already assigns the TObject pointer. Since the
     *  RelationArray needs to determine the name first and do some logic, we
     *  need a constructor which does not do anything. Therefore this
     *  constructor takes an int as argument to distuingish it from the default
     *  constructor. DO NOT DELETE AGAIN.
     *
     *  This contructor doesn't request a name. You can later assign an object to it, if you like.
     *
     *  The argument is ignored but required to distuingish
     *  between the default constructor and this one
     */
    explicit StoreObjPtr(int /*dummy*/)
      : m_storeObjPtr(0), m_name(""), m_durability(DataStore::c_Event) {}

    /** Virtual destructor for inherited classes */
    virtual ~StoreObjPtr() {}

    /** Assigning an object to the pointer.
     *
     *  This function actually calls the DataStore. If the DataStore is called for the first time during an execution of basf2,
     *  a new slot in the DataStore is registered. Due to I/O handling, this is only allowed during the initialize function of modules.
     *  If it happens at other times, a B2ERROR is produced. <br>
     *  If there is already a slot registered in the DataStore under the given name and durability, this slot is used. If the slot is
     *  occupied by an object of different type than the template class of StoreObjPtr, a B2FATAL error message is produced.
     *  If the slot isn't occupied so far by an object, you can create a new one.
     *  @return           Was a new object generated?
     *  @param name       Name under which the object to be assigned is stored. An empty string is treated as name equal to the template class name.
     *  @param durability Decides durability map used for getting or creating the accessed object.
     *  @param generate   Shall an object in the DataStore be created, if none occupies the slot with given name and durability?
     *                    For this purpose the default constructor of the template class is used and the created object assigned
     *                    to the StoreObjPtr.
     */
    bool assignObject(const std::string& name = "", const DataStore::EDurability& durability = DataStore::c_Event, bool generate = false);

    /** Store existing object.
     *
     *  Instead of creating a new object, you can store an object, that already exists.
     *  @return           True, if object was stored successfully. This might not be the case, if the requested slot is already occupied.
     *  @param name       Name under which the object shall be stored.
     *  @param durability Decides durability map used to store the object.
     */
    bool storeObject(T* const AObject, const std::string& name = "", const DataStore::EDurability& durability = DataStore::c_Event);

    //------------------------ Imitate pointer functionality -----------------------------------------------
    T& operator *()  const {return *m_storeObjPtr;}  /**< Imitate pointer functionality. */
    T* operator ->() const {return m_storeObjPtr;}   /**< Imitate pointer functionality. */
    operator bool()  const {return m_storeObjPtr;}   /**< Imitate pointer functionality. */

    //------------------------ Getters for AccessorParams --------------------------------------------------
    AccessorParams getAccessorParams() const {     /**< Returns name and durability under which the object is saved in the DataStore. */
      return AccessorParams(m_name, m_durability);
    }
    const std::string& getName() const { return m_name; } /**< Return  name under which the object is saved in the DataStore. */
    DataStore::EDurability getDurability() const { /**< Return  durability with which the object is saved in the DataStore. */
      return m_durability;
    }

  protected:
    /** Store of actual pointer. */
    T* m_storeObjPtr;

    /** Store name under which object is saved. */
    std::string m_name;

    /**Store durability under which the TClonesArray is saved. */
    DataStore::EDurability m_durability;
  };
} // end namespace Belle2


// ------------ Implementation of template class -----------------------------------------------------------
template <class T> bool Belle2::StoreObjPtr<T>::assignObject(const std::string& name, const Belle2::DataStore::EDurability& durability, bool generate)
{
  if (name == "") {
    m_name = DataStore::defaultObjectName<T>();
  } else {
    m_name = name;
  }
  m_durability = durability;
  m_storeObjPtr = 0;

  B2DEBUG(250, "Calling DataStore from StoreObjPtr." << name);
  return DataStore::Instance().handleObject<T>(m_name, durability, generate, m_storeObjPtr);
}

template <class T> bool Belle2::StoreObjPtr<T>::storeObject(T* const AObject, const std::string& name, const DataStore::EDurability& durability)
{
  if (name == "") {
    m_name = DataStore::defaultObjectName<T>();
  } else {
    m_name = name;
  }
  m_durability = durability;
  m_storeObjPtr = AObject;

  B2DEBUG(250, "Calling DataStore from StoreObjPtr.");
  return DataStore::Instance().handleObject<T>(m_name, durability, true, m_storeObjPtr);
}

#endif
