/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreAccessorBase.h>

#include <string>
#include <vector>

class TClass;
class TObject;

namespace Belle2 {
  /** a (simplified) python wrapper for StoreObjPtr.
  *
  * Compared to StoreObjPtr, PyStoreObj returns only TObjects (since
  * it doesn't use templates). Thanks to Python, you can still access all
  * public functions and data members of the actual type.
  *
  * You can check the runtime type information of the returned objects by
  * using Python's built-in type() function.
  *
  * <h1>Usage example</h1>
  * Inside a Python module's event() function, you can access DataStore
  * objects like this:
  * \code{.py}
    from ROOT import Belle2
    evtmetadata = Belle2.PyStoreObj('EventMetaData')
    # Alternatively: evtmetadata = Belle2.PyStoreObj(Belle2.EventMetaData.Class())
    if not evtmetadata:
      B2ERROR("No EventMetaData found");
    else:
      event = evtmetadata.getEvent()
      # alternatively: evtmetadata.obj().getEvent()
    \endcode

  * Most of the time you can just use the `PyStoreObj` instance like an
  * instance of the class it represents, i.e. call all the members. The only
  * exceptions are if the class has members which are also present in
  * `PyStoreObj` (for example isValid() or isRequired()). In this case you need
  * to use the obj() member to obtain a reference to the real object first as
  * shown in the example.
  *
  * <h1>Creating objects</h1>
  * You can also create new objects in your Python basf2 module, using
  * registerInDataStore() and create(). Since you
  * cannot specify the type directly, as with template arguments to StoreObjPtr,
  * the class name is assumed to be identical to the 'name' argument given to the
  * constructor, and to reside in the Belle2 namespace.
  * Consequently, you can only create objects with their default names.
  *
  * See display/examples/displaydata.py for a concrete example.
  *
  * \sa PyStoreArray and the Conditions Data interface classes PyDBObj and PyDBArray
  */
  class PyStoreObj {
  public:
    /** Return list of available objects for given durability. */
    static std::vector<std::string> list(DataStore::EDurability durability = DataStore::EDurability::c_Event);

    /** Print list of available objects for given durability. */
    static void printList(DataStore::EDurability durability = DataStore::EDurability::c_Event);

    /** constructor.
    * @param name Name of the entry to be accessed
    * @param durability 0: event, 1: persistent
    */
    explicit PyStoreObj(const std::string& name,
                        DataStore::EDurability durability =  DataStore::EDurability::c_Event);

    /** constructor.
    * @param objClass Class of the object to be accessed
    * @param durability 0: event, 1: persistent
    */
    explicit PyStoreObj(TClass* objClass,
                        DataStore::EDurability durability =  DataStore::EDurability::c_Event);

    /** constructor.
    * @param objClass Class of the object to be accessed
    * @param name Name of the entry to be accessed
    * @param durability 0: event, 1: persistent
    */
    explicit PyStoreObj(TClass* objClass,
                        const std::string& name,
                        DataStore::EDurability durability =  DataStore::EDurability::c_Event);

    /** Register the object in the DataStore.
     *  This must be called in the initialization phase.
     *
     *  @param storeFlags ORed combination of DataStore::EStoreFlags. (default: c_WriteOut)
     *  @return            True if the registration succeeded.
     */
    bool registerInDataStore(DataStore::EStoreFlags storeFlags);

    /** Register the object in the DataStore.
     *  This must be called in the initialization phase, and a TClass or name=class name must have been supplied during construction.
     *
     *  @param name        Name of the entry to be registered. Empty for default name.
     *  @param storeFlags  ORed combination of DataStore::EStoreFlags. (default: c_WriteOut)
     *  @return            True if the registration succeeded.
     */
    bool registerInDataStore(std::string name = "",
                             DataStore::EStoreFlags storeFlags = DataStore::EStoreFlags::c_WriteOut);

    /** Ensure this object has been registered previously.
     *  Will cause an ERROR if it does not exist.
     *  This must be called in the initialization phase.
     *
     *  @param name  If not empty, use non-default name for this object.
     *  @return      True if the object exists.
     */
    bool isRequired(const std::string& name = "");

    /** Tell the DataStore about an optional input.
     *
     *  Mainly useful for creating diagrams of module inputs and outputs.
     *  This must be called in the initialization phase.
     *
     *  @param name  If not empty, use non-default name for this object.
     *  @return      True if the object exists.
     */
    bool isOptional(const std::string& name = "");

    /** Check whether a TClass for the contained object could be determined. */
    bool hasValidClass() const;

    /** Check whether the object was registered and created. */
    bool isValid() const;

    /** Does this PyStoreObj contain a valid datastore object?
     *
     * Accessing the object's data is UNSAFE if this returns false.
     */
    operator bool() const { return isValid(); }

    /** Returns the attached DataStore object, or nullptr if no valid object exists. */
    TObject* obj() const { ensureAttached(); return isValid() ? m_storeEntry->ptr : nullptr; }

    /** Returns the attached DataStore object, or nullptr if no valid object exists. */
    TObject* operator->() const { return obj(); }

    /** Assign 'object' to the accessor. (takes ownership).
     *
     *  @param object    The object that should be put in the DataStore, should be of same type as the one used by this accessor.
     *  @param replace   Should an existing object be replaced? (if existing and supplied object are equal, this has no effect)
     *  @return          True if the assignment succeeded. If false, assign() will delete 'object', do not use it afterwards.
     **/
    bool assign(TObject* object, bool replace = false);

  private:
    /** Ensure that contained TObject has been created on the DataStore. */
    void ensureCreated();

  public:
    /** Create default constructed object in the DataStore.
     *
     *  @param replace   Should an existing object be replaced?
     *  @return          True if the creation succeeded.
     **/
    bool create(bool replace = false);

  private:
    /** Ensure that contained TObject has been attached to a memory location on the DataStore.*/
    void ensureAttached() const;

    /** Lookup the store entry and cache a pointer to it */
    void attach() const;

  private:
    /// Store accessor to retrieve the object.
    StoreAccessorBase m_storeAccessor;

    /// Pointer to the DataStore entry - serves as an internal cache omitting repeated look up from the DataStore.
    mutable StoreEntry* m_storeEntry = nullptr;
  };
}
