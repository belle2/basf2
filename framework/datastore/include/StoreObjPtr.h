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

  /** Type safe access to single objects in the data store.
   *
   *  This class provides access to single (i.e. non-array) objects
   *  in the data store, identified by their name and durability.
   *
   *  <h1>Accessing existing objects</h1>
   *  This example creates a new StoreObjPtr for the EventMetaData object,
   *  using the default name (EventMetaData) and default durability (event).
   *  \code
  StoreObjPtr<EventMetaData> eventmetadata;
  B2INFO("we're currently in event " << eventmetadata->getEvent() << "!");
      \endcode
   *  If no object 'EventMetaData' is found in the data store, a new one
   *  will be created. This can be avoided by setting the generate parameter
   *  of the constructor to false:
   *  \code
  StoreObjPtr<EventMetaData> checkObj("", DataStore::c_Event, false);
  if(!checkObj) {
    B2INFO("an object called '" << checkObj.getName() << "' does not exist in the data store.");
  } else {
    //object exists, you can now access its data
  }
      \endcode
   *
   *  <h1>Storing objects</h1>
   *  Storing objects works in the same way as the first example:
   *  \code
  //store a single cdchit
  StoreObjPtr<CDCHit> cdchit;
  cdchit->setCharge(5.0);
      \endcode
   *  As in the very first example above, StoreObjPtr will create a new object
   *  and add it to the data store (unless an object was found already).
   *
   *  Note that if you want to create a new object in a module, you should
   *  create an object of type StoreObjPtr<T> in your implementation of
   *  Module::initialize(). This registers the array in the data store and
   *  lets other modules know you intend to fill it.
   *
   *  @author <a href="mailto:belle2_software@bpost.kek.jp?subject=StoreObjPtr">The basf2 developers</a>
   *  @sa If you want to store more than a single object of one type, use the StoreArray class.
   */
  template <class T> class StoreObjPtr : StoreAccessorBase {
  public:
    /** Constructor with assignment.
     *
     *  Note that if generate is set to false, the created StoreObjPtr may be
     *  invalid if no object was found in the data store. Use operator bool()
     *  before trying to access the object's data.
     *
     *  In case the object in the data store is incompatible with the type T,
     *  the program will abort. Accessing a stored object through a StoreObjPtr
     *  of a base class T will work. (Don't try to store them like that, though.)
     *
     *  @param name       Name under which the object to be hold by this pointer is stored.
     *                    If an empty string is supplied, the type name will be used.
     *  @param durability Decides durability map used for getting or creating the accessed object.
     *  @param generate   Shall an object in the DataStore be created, if none exists with given name and durability?
     *  @sa assignObject()
     */
    explicit StoreObjPtr(const std::string& name = "", const DataStore::EDurability& durability = DataStore::c_Event, bool generate = true) {
      assignObject(name, durability, generate);
    }

    /** Constructor for storing an existing object.
     *
     *  @param AObject    Object that is to be saved in the data store.
     *  @param name       Name under which the object to be hold by this pointer is stored.
     *                    If an empty string is supplied, the type name will be used.
     *  @param durability Decides durability map used for getting or creating the accessed object.
     *  @sa assignObject()
     */
    explicit StoreObjPtr(T* const AObject, const std::string& name = "", const DataStore::EDurability& durability = DataStore::c_Event) {
      assignObject(name, durability, true, AObject);
    }

    /** Constructor for usage with Relations etc.
     *
     *  No new objects are created when using this constructor.
     *  @param accessorParams   A pair with name and durability.
     */
    explicit StoreObjPtr(AccessorParams accessorParams) {
      assignObject(accessorParams.first, accessorParams.second, false);
    }

    /** Virtual destructor for inherited classes */
    virtual ~StoreObjPtr() {}

    //------------------------ Imitate pointer functionality -----------------------------------------------
    inline T& operator *()  const {return *m_storeObjPtr;}  /**< Imitate pointer functionality. */
    inline T* operator ->() const {return m_storeObjPtr;}   /**< Imitate pointer functionality. */
    inline operator bool()  const {return m_storeObjPtr;}   /**< Imitate pointer functionality. */

    //------------------------ Getters for AccessorParams --------------------------------------------------
    AccessorParams getAccessorParams() const {     /**< Returns name and durability under which the object is saved in the DataStore. */
      return AccessorParams(m_name, m_durability);
    }
    const std::string& getName() const { return m_name; } /**< Return  name under which the object is saved in the DataStore. */
    DataStore::EDurability getDurability() const { /**< Return durability with which the object is saved in the DataStore. */
      return m_durability;
    }

  protected:
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


    /** Assigning an object to the pointer.
     *
     *  This function actually calls the DataStore. If the DataStore is called for the first time during an execution of basf2,
     *  a new slot in the DataStore is registered. Due to I/O handling, this is only allowed during the initialize function of modules.
     *  If it happens at other times, a B2ERROR is produced. <br>
     *  If there is already a slot registered in the DataStore under the given name and durability, this slot is used. If the slot is
     *  occupied by an object of different type than the template class of StoreObjPtr, a B2FATAL error message is produced.
     *  If the slot isn't occupied so far by an object, you can create a new one.
     *
     *  @param name       Name under which the object to be assigned is stored. An empty string is treated as name equal to the template class name.
     *  @param durability Decides durability map used for getting or creating the accessed object.
     *  @param generate   Shall an object in the DataStore be created, if none occupies the slot with given name and durability?
     *                    For this purpose the default constructor of the template class is used and the created object assigned
     *                    to the StoreObjPtr.
     *  @param AObject    Object to add to the data store, NULL to connect to object
     *                    existing in the data store or for creation of new object.
     *  @return           Was a new object generated?
     */
    bool assignObject(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event, bool generate = false, T* const AObject = 0);

    /** Store of actual pointer. */
    T* m_storeObjPtr;

    /** name under which object is saved. */
    std::string m_name;

    /**durability under which the object is saved. */
    DataStore::EDurability m_durability;
  };
} // end namespace Belle2


// ------------ Implementation of template class -----------------------------------------------------------
template <class T> bool Belle2::StoreObjPtr<T>::assignObject(const std::string& name, Belle2::DataStore::EDurability durability, bool generate, T* const AObject)
{
  if (name == "") {
    m_name = DataStore::defaultObjectName<T>();
  } else {
    m_name = name;
  }
  m_durability = durability;
  m_storeObjPtr = AObject;

  B2DEBUG(250, "Calling DataStore from StoreObjPtr." << name);
  return DataStore::Instance().handleObject<T>(m_name, durability, generate, m_storeObjPtr);
}

#endif
