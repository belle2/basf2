/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreAccessorBase.h>

#include <stdexcept>

namespace Belle2 {

  /** Type-safe access to single objects in the data store.
   *
   *  This class provides access to single (i.e. non-array) objects
   *  in the data store, identified by their name and durability.
   *
   *  <h2>Accessing existing objects</h2>
   *  This example creates a new StoreObjPtr for the EventMetaData object,
   *  using the default name (EventMetaData) and default durability (event).
   *  If no object 'EventMetaData' is found in the data store, the store
   *  object pointer is invalid (accesses to it will cause an exception).
   *  \code
      StoreObjPtr<EventMetaData> eventmetadata;
      if(!eventmetadata) {
        B2INFO("an object called '" << eventmetadata.getName() << "' does not exist in the data store.");
      } else {
        //object exists, you can now access its data
        B2INFO("we're currently in event " << eventmetadata->getEvent() << "!");
      }
      \endcode
   *
   *  <h2>Storing objects</h2>
   *  First, objects have to be registered in the data store during the
   *  initialization phase, meaning in the initialize method of a module:
   *  \code
      void MyModule::initialize() {
        //register a single cdchit
        StoreObjPtr<CDCHit> cdchit;
        cdchit.registerInDataStore();
        //register a single cdchit under the name "AnotherHit" and do not write
        //it to the output file by default
        StoreObjPtr<CDCHit> cdchit2;
        cdchit2.registerInDataStore("AnotherHit", DataStore::c_DontWriteOut);
      }
      \endcode
   *  Before objects can be accessed they have to be created
   *  (in each event if the durability is c_Event):
   *  \code
      //store a single cdchit
      StoreObjPtr<CDCHit> cdchit;
      cdchit.create(); // or construct() if you want to specify constructor arguments
      cdchit->setCharge(5.0);
      \endcode
   *  To put an existing object in the data store, use the assign method:
   *  \code
      //store a single cdchit
      CDCHit* cdchit = new CDCHit;
      StoreObjPtr<CDCHit> cdchitPtr;
      cdchitPtr.assign(cdchit);
      cdchitPtr->setCharge(5.0);
      \endcode
   *  Note that the datastore takes ownership of the object!
   *
   *  <h2>Using StoreOjbPtr as a module member variable</h2>
   *  To avoid some overhead involved in re-creating the StoreObjPtr e.g. in
   *  each event() function call, you can also make StoreObjPtr a member variable
   *  of your class. If it is of non-event durability, you'll need to add the appropriate
   *  constructor call to the initializer list, e.g. (here with default name):
   *
      \code
      MyModule::MyModule():
        m_fooPtr("", DataStore::c_Persistent)
      {}
      \endcode
   *
   *  In initialize(), you should also use registerInDataStore() or isOptional()/isRequired()
   *  to specify wether it is an input or output.
   *  For <b>non-default names</b> (which you might not know in the constructor, e.g. in the
   *  case of module parameters), set the 'name' argument of any of these three functions to
   *  permanently bind the StoreObjPtr to the array with the given name.
   *
   *  @sa If you want to store more than a single object of one type, use the StoreArray class.
   *  @sa Data can also be created/accessed from Python modules using PyStoreObj
   */
  template <class T> class StoreObjPtr : public StoreAccessorBase {
  public:
    /** Constructor to access an object in the DataStore.
     *
     *  @param name       Name under which the object is stored in the DataStore.
     *                    If an empty string is supplied, the type name will be used.
     *  @param durability Decides durability map used for getting the accessed object.
     */
    explicit StoreObjPtr(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event):
      StoreAccessorBase(DataStore::objectName<T>(name), durability, T::Class(), false), m_storeObjPtr(0) {}

    /** Check whether the object was created.
     *
     *  @return          True if the object exists.
     **/
    inline bool isValid() const {ensureAttached(); return m_storeObjPtr && *m_storeObjPtr;}

    /** Construct an object of type T in this StoreObjPtr, using the provided constructor arguments.
     *
     * If this StoreObjPtr already contains an object, this function will fail.
     *
     *  @return          True if the creation succeeded.
     **/
    template<class ...Args> bool construct(Args&& ... params)
    {
      T* t = new T(std::forward<Args>(params)...);
      return assign(t, false);
    }

    /** Construct an object of type T in this StoreObjPtr, using the provided constructor arguments.
     *
     * If this StoreObjPtr already contains an object, it will be replaced.
     *
     *  @return          True if the creation succeeded.
     **/
    template<class ...Args> bool constructAndReplace(Args&& ... params)
    {
      T* t = new T(std::forward<Args>(params)...);
      return assign(t, true);
    }

    //------------------------ Imitate pointer functionality -----------------------------------------------
    inline T& operator *()  const {return *operator->();}  /**< Imitate pointer functionality. */
    inline T* operator ->() const {ensureValid(); return static_cast<T*>(*m_storeObjPtr);}   /**< Imitate pointer functionality. */
    inline operator bool()  const {return isValid();}   /**< Imitate pointer functionality. */


    /** Return list of object names with matching type.  */
    static std::vector<std::string> getObjectList(DataStore::EDurability durability = DataStore::c_Event)
    {
      return DataStore::Instance().getListOfObjects(T::Class(), durability);
    }

  private:
    /** Ensure that this object is attached. */
    inline void ensureAttached() const
    {
      if (!m_storeObjPtr) {
        const_cast<StoreObjPtr*>(this)->m_storeObjPtr = DataStore::Instance().getObject(*this);
      }
    }
    /** if accesses to this object would crash, throw an std::runtime_error */
    inline void ensureValid() const
    {
      ensureAttached();
      if (!m_storeObjPtr || !(*m_storeObjPtr))
        throw std::runtime_error("Trying to access StoreObjPtr " + readableName() +
                                 ", which was not created. Please check isValid() before accesses if the object is not guaranteed to be created in every event.");
    }
    /** Store of actual pointer. Don't make this a T** as this might cause problems with multiple inheritance objects */
    TObject** m_storeObjPtr;
  };
} // end namespace Belle2
