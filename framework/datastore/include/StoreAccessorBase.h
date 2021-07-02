/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/datastore/DataStore.h>

#include <string>
#include <utility>

class TClass;

namespace Belle2 {

  typedef std::pair<std::string, DataStore::EDurability>
  AccessorParams; /**< Pair of parameters needed to find an object in the DataStore. */

  /** Base class for StoreObjPtr and StoreArray for easier common treatment.  */
  class StoreAccessorBase {
  public:

    /** Constructor to access an object or array in the DataStore.
     *
     *  @param name       Name under which the object is stored in the DataStore.
     *  @param durability Decides durability map used for getting the accessed object.
     *  @param objClass   Type of the object
     *  @param isArray    true if the entry in the DataStore is an array
     */
    StoreAccessorBase(const std::string& name, DataStore::EDurability durability, TClass* objClass, bool isArray):
      m_name(name), m_durability(durability), m_class(objClass), m_isArray(isArray) {}

    /** Destructor.
     *
     *  Virtual because this is a base class.
     */
    virtual ~StoreAccessorBase() {}

    /** Register the object/array in the DataStore.
     *  This must be called in the initialization phase.
     *
     *  @param storeFlags ORed combination of DataStore::EStoreFlags.
     *  @return            True if the registration succeeded.
     */
    bool registerInDataStore(DataStore::EStoreFlags storeFlags = DataStore::c_WriteOut)
    {
      return DataStore::Instance().registerEntry(m_name, m_durability, getClass(), isArray(), storeFlags);
    }

    /** Register the object/array in the DataStore.
     *  This must be called in the initialization phase.
     *
     *  @param name  If not empty, set non-default name for this object/array. This is permanent, so that e.g. after using registerInDataStore("myName") in initialize(), this object will continue refer to 'myName' in event().
     *  @param storeFlags ORed combination of DataStore::EStoreFlags.
     *  @return            True if the registration succeeded.
     */
    bool registerInDataStore(const std::string& name, DataStore::EStoreFlags storeFlags = DataStore::c_WriteOut)
    {
      if (!name.empty())
        m_name = name;
      return DataStore::Instance().registerEntry(m_name, m_durability, getClass(), isArray(), storeFlags);
    }

    /** Ensure this array/object has been registered previously.
     *  Will cause an ERROR if it does not exist.
     *  This must be called in the initialization phase.
     *
     *  @param name  If not empty, set non-default name for this object/array. This is permanent, so that e.g. after using registerInDataStore("myName") in initialize(), this object will continue refer to 'myName' in event().
     *  @return            True if the object/array exists.
     */
    bool isRequired(const std::string& name = "")
    {
      if (!name.empty())
        m_name = name;
      return DataStore::Instance().requireInput(*this);
    }

    /** Tell the DataStore about an optional input.
     *
     *  Mainly useful for creating diagrams of module inputs and outputs.
     *
     *
     *  @param name  If not empty, set non-default name for this object/array. This is permanent, so that e.g. after using registerInDataStore("myName") in initialize(), this object will continue refer to 'myName' in event().
     *  @return            True if the object/array exists.
     */
    bool isOptional(const std::string& name = "")
    {
      if (!name.empty())
        m_name = name;
      return DataStore::Instance().optionalInput(*this);
    }

    /** Create a default object in the data store.
     *
     *  This only works after registerInDataStore() has been called by this or another module.
     *
     *  @param replace   Should an existing object be replaced?
     *  @return          True if the creation succeeded.
     **/
    bool create(bool replace = false)
    {
      return DataStore::Instance().createObject(0, replace, *this);
    }

    /** Assign 'object' to this accessor. (takes ownership).
     *
     *  @param object    The object that should be put in the DataStore, should be of same type as the one used by this accessor.
     *  @param replace   Should an existing object be replaced? (if existing and supplied object are equal, this has no effect)
     *  @return          True if the assignment succeeded. If false, assign() will delete 'object', do not use it afterwards.
     **/
    bool assign(TObject* object, bool replace = false);

    /** Clear contents of this object. */
    virtual void clear()
    {
      create(true);
    }


    /** Return name under which the object is saved in the DataStore. */
    const std::string& getName() const { return m_name; }

    /** Return durability with which the object is saved in the DataStore. */
    DataStore::EDurability getDurability() const { return m_durability; }

    /** Return pair of name and durability under which stored object is saved.  */
    AccessorParams getAccessorParams() const { return make_pair(m_name, m_durability);}

    /** Check if two store accessors point to the same object/array. */
    virtual bool operator==(const StoreAccessorBase& other) const
    {
      return getAccessorParams() == other.getAccessorParams();
    }

    /** Check if two store accessors point to a different object/array. */
    virtual bool operator!=(const StoreAccessorBase& other) const
    {
      return !(*this == other);
    }

    /** The underlying object's type. */
    TClass* getClass() const { return m_class; }

    /** Is this an accessor for an array? */
    bool isArray() const { return m_isArray; }

    /** Returns true if this object/array should not be saved by output modules. See DataStore::c_DontWriteOut. Can be changed by re-registering it with/without the flag. */
    bool notWrittenOut() const;


    /** Convert this acessor into a readable string (for messages).
     *
     * e.g. "object EventMetaData (durability: event)"
     */
    std::string readableName() const;

  protected:
    /** Store name under which this object/array is saved. */
    std::string m_name;

    /**Store durability under which the object/array is saved. */
    DataStore::EDurability m_durability;

    /** The underlying object's type. */
    TClass* m_class;

    /** Is this an accessor for an array? */
    bool m_isArray;

  };
}
