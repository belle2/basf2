/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/datastore/DataStore.h>

#include <string>
#include <utility>

class TClass;

namespace Belle2 {

  typedef std::pair<std::string, DataStore::EDurability> AccessorParams; /**< Pair of parameters needed to find an object in the DataStore. */

  /** Base class for StoreObjPtr and StoreArray for easier common treatment.
   *
   *  @author <a href="mailto:belle2_software@bpost.kek.jp?subject=StoreAccessorBase">The basf2 developers</a>
   */
  class StoreAccessorBase {
  public:

    /** Constructor to access an array in the DataStore.
     *
     *  @param name       Name under which the object is stored in the DataStore.
     *  @param durability Decides durability map used for getting the accessed object.
     */
    StoreAccessorBase(const std::string& name, DataStore::EDurability durability, const TClass* objClass, bool isArray):
      m_name(name), m_durability(durability), m_class(objClass), m_isArray(isArray) {};

    /** Destructor.
     *
     *  Virtual because this is a base class.
     */
    virtual ~StoreAccessorBase() {};

    /** Register the object/array in the DataStore and include it in the output by default.
     *  This must be called in the initialization phase.
     *
     *  @param errorIfExisting  Flag whether an error will be reported if the object/array was already registered.
     *  @return            True if the registration succeeded.
     */
    bool registerAsPersistent(bool errorIfExisting = false) {
      return DataStore::Instance().registerEntry(m_name, m_durability, getClass(), isArray(),
                                                 false, errorIfExisting);
    }

    /** Register the object/array in the data store, but do not include it in the output by default.
     *  This must be called in the initialization phase.
     *
     *  @param errorIfExisting  Flag whether an error will be reported if the object/array was already registered.
     *  @return            True if the registration succeeded.
     */
    bool registerAsTransient(bool errorIfExisting = false) {
      return DataStore::Instance().registerEntry(m_name, m_durability, getClass(), isArray(),
                                                 true, errorIfExisting);
    }

    /** Ensure this array/object has been registered previously.
     *  Will cause an ERROR if it does not exist.
     *  This must be called in the initialization phase.
     *
     *  @return            True if the object/array exists.
     */
    bool isRequired() {
      return DataStore::Instance().require(*this);
    }

    /** Tell the DataStore about an optional input.
     *
     *  Mainly useful for creating diagrams of module inputs and outputs.
     *  This must be called in the initialization phase.
     *
     *  @return            True if the object/array exists.
     */
    bool isOptional() {
      return DataStore::Instance().optionalInput(*this);
    }

    /** Create a default object in the data store.
     *
     *  @param replace   Should an existing object be replaced?
     *  @return          True if the creation succeeded.
     **/
    bool create(bool replace = false) {
      return DataStore::Instance().createObject(0, replace, *this);
    };

    /** Add an existing object to the data store (takes ownership).
     *
     *  @param object    The object that should be put in the DataStore.
     *  @param replace   Should an existing object be replaced?
     *  @return          True if the creation succeeded.
     **/
    bool assign(TObject* object, bool replace = false) {
      return DataStore::Instance().createObject(object, replace, *this);
    };


    /** Return name under which the object is saved in the DataStore. */
    const std::string& getName() const { return m_name; }

    /** Return durability with which the object is saved in the DataStore. */
    DataStore::EDurability getDurability() const { return m_durability; }

    /** Return pair of name and durability under which stored object is saved.  */
    AccessorParams getAccessorParams() const {return make_pair(m_name, m_durability);};

    /** Check if two store accessors point to the same object/array. */
    virtual bool operator==(const StoreAccessorBase& other) {
      return getAccessorParams() == other.getAccessorParams();
    }

    /** Check if two store accessors point to a different object/array. */
    virtual bool operator!=(const StoreAccessorBase& other) {
      return !(*this == other);
    }

    /** The underlying object's type. */
    const TClass* getClass() const { return m_class; }

    /** Is this an accessor for an array? */
    bool isArray() const { return m_isArray; }

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
    const TClass* m_class;

    /** Is this an accessor for an array? */
    bool m_isArray;

  };
}
