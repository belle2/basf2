/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Marko Staric                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/DBStore.h>

#include <string>
#include <utility>

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
     */
    DBAccessorBase(const std::string& name,
                   const TClass* objClass,
                   bool isArray) : m_ptr(0)
    {
      m_entry = DBStore::Instance().getEntry(name, objClass, isArray);
      m_iov = m_entry->iov;
    };

    /**
     * Destructor.
     * Virtual because this is a base class.
     */
    virtual ~DBAccessorBase() {};

    /** Return name under which the object is saved in the DBStore. */
    const std::string& getName() const { return m_entry->name; }

    /**
     * Check whether a valid object was obtained from the database.
     * @return          True if the object exists.
     **/
    inline bool isValid() const {return m_entry->object;}

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
      bool result = ((m_iov != m_entry->iov) || (m_ptr != m_entry->object));
      m_iov = m_entry->iov;
      m_ptr = m_entry->object;
      return result;
    }

    /**
     * Add a callback function.
     * The given function will be called whenever there is a new database entry for this DBAccessor.
     *
     * @param callback   The callback function.
     */
    void addCallback(void(*callback)())
    {
      DBStore::Instance().addCallback(m_entry->name, callback, std::make_pair(reinterpret_cast<void*>(callback),
                                      nullptr));
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
      union {void(T::*callback)(); void* pointer;} tmp;
      tmp.callback = callback;
      DBStore::Instance().addCallback(m_entry->name, std::bind(std::mem_fn(callback), object),
                                      std::make_pair(tmp.pointer, object));
    }

  protected:
    /** Pointer to the entry in the DBStore. */
    DBEntry* m_entry;

    /** IoV at last call to hasChanged. */
    IntervalOfValidity m_iov;

    /** object pointer at last call to hasChanged. */
    TObject* m_ptr;

  };
}
