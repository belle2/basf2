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
   *  If no object 'EventMetaData' is found in the data store, the store
   *  object pointer is invalid.
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
   *  <h1>Storing objects</h1>
   *  First, objects have to be registered in the data store during the
   *  initialization phase, meaning in the initialize method of a module:
   *  \code
  void MyModule::initialize() {
    //register a single cdchit
    StoreObjPtr<CDCHit>::registerPersistent();
    //register a single cdchit under the name "AnotherHit" and do not write
    //it to the output file by default
    StoreObjPtr<CDCHit>::registerTransient("AnotherHit");
  }
      \endcode
   *  Before objects can be accessed they have to be created
   *  (in each event if the durability is c_Event):
   *  \code
  //store a single cdchit
  StoreObjPtr<CDCHit> cdchit;
  cdchit.create();
  cdchit->setCharge(5.0);
      \endcode
   *  To put an existing object in the data store, use it as first argument
   *  of the create method:
   *  \code
  //store a single cdchit
  CDCHit* cdchit = new CDCHit;
  StoreObjPtr<CDCHit> cdchit;
  cdchit.create(cdchit);
  cdchit->setCharge(5.0);
      \endcode
   *  Note that the datastore takes the ownership of the object!
   *
   *  @author <a href="mailto:belle2_software@bpost.kek.jp?subject=StoreObjPtr">The basf2 developers</a>
   *  @sa If you want to store more than a single object of one type, use the StoreArray class.
   */
  template <class T> class StoreObjPtr : public StoreAccessorBase {
  public:
    /** Register an object, that should be written to the output by default, in the data store.
     *  This must be called in the initialization phase.
     *
     *  @param name        Name under which the object is stored.
     *  @param durability  Specifies lifetime of object in question.
     *  @param errorIfExisting  Flag whether an error will be reported if the object was already registered.
     *  @return            True if the registration succeeded.
     */
    static bool registerPersistent(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event,
                                   bool errorIfExisting = true) {
      return DataStore::Instance().createEntry(DataStore::objectName<T>(name), durability, T::Class(), false, false, errorIfExisting);
    }

    /** Register an object, that should not be written to the output by default, in the data store.
     *
     *  This must be called in the initialization phase.
     *
     *  @param name        Name under which the object is stored.
     *  @param durability  Specifies lifetime of object in question.
     *  @param errorIfExisting  Flag whether an error will be reported if the object was already registered.
     *  @return            True if the registration succeeded.
     */
    static bool registerTransient(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event,
                                  bool errorIfExisting = true) {
      return DataStore::Instance().createEntry(DataStore::objectName<T>(name), durability, T::Class(), false, true, errorIfExisting);
    }

    /** Check whether an object was registered before.
     *
     *  It will cause an error if the object does not exist.
     *  This must be called in the initialzation phase.
     *
     *  @param name        Name under which the object is stored.
     *  @param durability  Specifies lifetime of object in question.
     *  @return            True if the object exists.
     */
    static bool required(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event) {
      std::string objName = DataStore::objectName<T>(name);
      if (!DataStore::Instance().hasEntry(objName, durability, T::Class(), false)) {
        B2ERROR("The required DataStore entry with name " << objName << " and durability " << durability << " does not exists.");
        return false;
      }
      return true;
    }

    /** Constructor to access an object in the DataStore.
     *
     *  @param name       Name under which the object is stored in the DataStore.
     *                    If an empty string is supplied, the type name will be used.
     *  @param durability Decides durability map used for getting the accessed object.
     */
    explicit StoreObjPtr(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event):
      StoreAccessorBase(DataStore::objectName<T>(name), durability), m_storeObjPtr(0) {
      if (DataStore::Instance().getInitializeActive()) {
        DataStore::Instance().backwardCompatibleRegistration(m_name, m_durability, T::Class(), false);
      }
    }

    /** Register the object in the data store and include it in the output by default.
     *  This must be called in the initialization phase.
     *
     *  @param errorIfExisting  Flag whether an error will be reported if the object was already registered.
     *  @return            True if the registration succeeded.
     */
    bool registerAsPersistent(bool errorIfExisting = false) {
      return DataStore::Instance().createEntry(m_name, m_durability, T::Class(), false, false, errorIfExisting);
    }

    /** Register the object in the data store and do not include it in the output by default.
     *  This must be called in the initialization phase.
     *
     *  @param errorIfExisting  Flag whether an error will be reported if the object was already registered.
     *  @return            True if the registration succeeded.
     */
    bool registerAsTransient(bool errorIfExisting = false) {
      return DataStore::Instance().createEntry(m_name, m_durability, T::Class(), false, true, errorIfExisting);
    }

    /** Check whether an object was registered before.
     *
     *  It will cause an error if the object does not exist.
     *  This must be called in the initialzation phase.
     *
     *  @param name        Name under which the object is stored.
     */
    bool isRequired() {
      if (!DataStore::Instance().hasEntry(m_name, m_durability, T::Class(), false)) {
        B2ERROR("The required DataStore entry with name " << m_name << " and durability " << m_durability << " does not exists.");
        return false;
      }
      return true;
    }

    /** Create a default object in the data store.
     *
     *  @param replace   Should an existing object be replaced?
     *  @return          True if the creation succeeded.
     **/
    bool create(bool replace = false) {
      return DataStore::Instance().createObject(0, replace, m_name, m_durability, T::Class(), false);
    };

    /** Add an existing object to the data store.
     *
     *  @param object    The object that should be put in the DataStore.
     *  @param replace   Should an existing object be replaced?
     *  @return          True if the creation succeeded.
     **/
    bool assign(TObject* object, bool replace = false) {
      return DataStore::Instance().createObject(object, replace, m_name, m_durability, T::Class(), false);
    };

    /** Check whether the object was created.
     *
     *  @return          True if the object exists.
     **/
    inline bool isValid() const {ensureAttached(); return m_storeObjPtr && *m_storeObjPtr;}


    /** Virtual destructor for inherited classes */
    virtual ~StoreObjPtr() {}

    //------------------------ Imitate pointer functionality -----------------------------------------------
    inline T& operator *()  const {ensureAttached(); return **m_storeObjPtr;}  /**< Imitate pointer functionality. */
    inline T* operator ->() const {ensureAttached(); return *m_storeObjPtr;}   /**< Imitate pointer functionality. */
    inline operator bool()  const {return isValid();}   /**< Imitate pointer functionality. */

  private:
    /** Ensure that this object is attached. */
    inline void ensureAttached() const {
      if (!m_storeObjPtr) {
        const_cast<StoreObjPtr*>(this)->m_storeObjPtr = reinterpret_cast<T**>(DataStore::Instance().getObject(m_name, m_durability, T::Class(), false));
      }
    }
    /** Store of actual pointer. */
    T** m_storeObjPtr;
  };
} // end namespace Belle2


#endif
