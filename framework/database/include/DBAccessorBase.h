/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Marko Staric, Martin Ritter                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/DBStore.h>
#include <framework/logging/Logger.h>

#include <string>
#include <vector>
#include <functional>

class TClass;

namespace Belle2 {

  /** Base class for DBObjPtr and DBArray for easier common treatment. */
  class DBAccessorBase {
  public:

    /**
     * Constructor to access an object in the DBStore.
     * @param name       Name under which the object is stored in the database (and DBStore).
     * @param objClass   The type of the object.
     * @param isArray    Flag that indicates whether this is a single object or a TClonesArray.
     * @param isRequired If true emit errors if the object cannot be found in the Database
     */
    DBAccessorBase(const std::string& name, const TClass* objClass, bool isArray, bool isRequired) :
      m_type(DBStoreEntry::c_Object), m_name(name), m_objClass(objClass), m_isArray(isArray), m_isRequired(isRequired),
      m_entry{DBStore::Instance().getEntry(name, objClass, isArray, isRequired)}, m_changed{isValid()}
    {
      if (m_entry) m_entry->registerAccessor(this);
    }

    /**
     * Constructor to access an object in the DBStore which is not a ROOT Object.
     * @param type       should be one of c_RootFile or c_RawFile.
     * @param name       Name under which the object is stored in the database (and DBStore).
     * @param isArray    Flag that indicates whether this is a single object or a TClonesArray.
     * @param isRequired If true emit errors if the object cannot be found in the Database
     */
    DBAccessorBase(DBStoreEntry::EPayloadType type, const std::string& name, bool isRequired):
      m_type(type), m_name(name), m_objClass(nullptr), m_isArray(false), m_isRequired(isRequired),
      m_entry{DBStore::Instance().getEntry(type, name, nullptr, false, isRequired)}, m_changed{isValid()}
    {
      if (m_entry) m_entry->registerAccessor(this);
    }

    /**
     * Destructor.
     * Virtual because this is a base class.
     */
    virtual ~DBAccessorBase()
    {
      if (m_entry) m_entry->removeAccessor(this);
    }

    /** Return name under which the object is saved in the DBStore. */
    const std::string& getName() const { return m_name; }

    /**
     * Check whether a valid object was obtained from the database.
     * @return          True if the object exists.
     **/
    inline bool isValid() const {if (!ensureAttached()) return false; return (m_entry->getObject() != nullptr);}

    inline operator bool()  const {return isValid();}   /**< Imitate pointer functionality. */

    /** Check if two store accessors point to the same object/array. */
    virtual bool operator==(const DBAccessorBase& other) const
    {
      return getName() == other.getName();
    }

    /** Check if two store accessors point to a different object/array. */
    virtual bool operator!=(const DBAccessorBase& other) const
    {
      return !(*this == other);
    }

    /** Check whether the object has changed since the last call to hasChanged  of the accessor). */
    bool hasChanged()
    {
      const bool ret = m_changed;
      m_changed = false;
      return ret;
    }

    /** Add a callback method.
     * The given method will be called whenever there is a new database entry for this DBAccessor. The one parameter is the
     * name of the DB entry which changed (in case the same callback method is to be used for multiple payloads.
     *
     * @param callback function pointer to a callback function
     * @param onDestruction if true the callback will not be called for each
     *    change but only when the Database entry is deleted and can be used to
     *    remove dangling pointers to the entry.
     */
    void addCallback(std::function<void(const std::string&)> callback, bool onDestruction = false)
    {
      m_callbacks.emplace_back(callback, onDestruction);
    }

    /** Add a callback method.
     * The given method will be called whenever there is a new database entry for this DBAccessor.
     *
     * @param callback function pointer to a callback function
     * @param onDestruction if true the callback will not be called for each
     *    change but only when the Database entry is deleted and can be used to
     *    remove dangling pointers to the entry.
     */
    void addCallback(std::function<void()> callback, bool onDestruction = false)
    {
      addCallback([callback](const std::string&) -> void { callback(); }, onDestruction);
    }

    /**
     * Add a callback method of an object.
     * The given method will be called whenever there is a new database entry for this DBAccessor.
     *
     * @param object     The object with the callback method.
     * @param callback   The callback method.
     */
    template<class T> void addCallback(T* object, void(T::*callback)())
    {
      addCallback([ = ](const std::string&) {(*object.*callback)();});
    }

  protected:
    /** Return a pointer to the Object already cast to the correct type */
    template<class T = TObject> const T * getObject() const
    {
      if (!ensureAttached()) return nullptr;
      return reinterpret_cast<const T*>(m_entry->getObject());
    }

    /** Make sure we are attached to the the DBStore. If not try to reconnect */
    bool ensureAttached() const
    {
      if (!m_entry) {
        B2DEBUG(32, "DBAccessor " << m_name << " lost connection, reattaching");
        m_entry = DBStore::Instance().getEntry(m_type, m_name, m_objClass, m_isArray, m_isRequired);
        m_changed = true;
        if (!m_entry) return false;
        m_entry->registerAccessor(const_cast<DBAccessorBase*>(this));
      }
      return true;
    }

    /** Callback function which gets called by the DBStoreEntry object if it changes */
    void storeEntryChanged(bool destructed)
    {
      // we obviously changed
      m_changed = true;
      // StoreEntry is destructed, remove reference
      if (destructed) m_entry = nullptr;
      // Now run all registered callbacks
      // TODO: We could guard m_callbacks against insertions during callback
      // execution to prevent exponential growth of callbacks
      for (const auto& cb : m_callbacks) {
        if (destructed == cb.second) cb.first(m_name);
      }
    }

    /** Type of the payload */
    const DBStoreEntry::EPayloadType m_type;
    /** Name of the payload in the database */
    const std::string m_name;
    /** Class of the payload if type is c_Object */
    const TClass* m_objClass;
    /** True if the payload is an array of objects */
    const bool m_isArray;
    /** True if the payload is required, otherwise no errors will be raised if it cannot be found */
    const bool m_isRequired;
    /** Pointer to the entry in the DBStore. */
    mutable DBStoreEntry* m_entry;
    /** Internal flag whether the object has changed since we last checked */
    mutable bool m_changed{false};
    /** List of all registered callback functions */
    std::vector<std::pair<std::function<void(const std::string&)>, bool>> m_callbacks;
    /** Allow the DBStoreEntry to call the callback notifier */
    friend class DBStoreEntry;
  };
}
