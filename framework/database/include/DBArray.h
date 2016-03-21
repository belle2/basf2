/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/DBAccessorBase.h>

#include <framework/utilities/ArrayIterator.h>

#include <TClonesArray.h>
#include <stdexcept>

namespace Belle2 {

  /**
   * Class for accessing arrays of objects in the database.
   */
  template<class T> class DBArray: public DBAccessorBase {
  public:
    /** STL-like iterator over the T objects (not T* ). */
    typedef ArrayIterator<DBArray<T>, T> iterator;
    /** STL-like const_iterator over the T objects (not T* ). */
    typedef ArrayIterator<DBArray<T>, const T> const_iterator;

    /**
     * Constructor to access an array of objects in the DBStore.
     * @param name       Name under which the array is stored in the database (and DBStore).
     */
    explicit DBArray(const std::string& module = "", const std::string& package = "dbstore"):
      DBAccessorBase(package, DBStore::arrayName<T>(module), T::Class(), true) {m_array = reinterpret_cast<TClonesArray**>(&m_entry->object);};

    /** Get the number of objects in the array. */
    inline int getEntries() const { return isValid() ? ((*m_array)->GetEntriesFast()) : 0;}

    /** Access to the stored objects.
     *
     * Out-of-bounds accesses throw an std::out_of_range exception
     *
     * @param i  Array index, should be in 0..getEntries()-1
     * @return   pointer to the object
     */
    inline T* operator [](int i) const
    {
      //At() checks for out-of-range and returns NULL in that case
      TObject* obj = (*m_array)->At(i);
      if (obj == nullptr)
        throw std::out_of_range("Out-of-range access in DBArray::operator[], for " + getName() + ", index " + std::to_string(i));
      return static_cast<T*>(obj); //type was checked by DataStore, so the cast is safe.
    }

    template<class KEY> T* getByKey(KEY(T::*method)(void) const, KEY key) const
    {
      for (int i = 0; i < getEntries(); i++) {
        T* obj = static_cast<T*>((*m_array)->At(i));
        if ((*obj.*method)() == key) {
          return obj;
        }
      }
      return nullptr;
    }

    /** Return iterator to first entry. */
    iterator begin() { return iterator(this, 0); }
    /** Return iterator to last entry +1. */
    iterator end() { return iterator(this, getEntries()); }

    /** Return const_iterator to first entry. */
    const_iterator begin() const { return const_iterator(this, 0); }
    /** Return const_iterator to last entry +1. */
    const_iterator end() const { return const_iterator(this, getEntries()); }

  private:
    /** Pointer to pointer to the TClonesArray in the DBStore. */
    TClonesArray** m_array;

  };
}
