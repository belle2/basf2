/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    /** STL-like const_iterator over the T objects (not T* ). */
    typedef ObjArrayIterator<const TClonesArray, const T> const_iterator;

    /**
     * Constructor to access an array of objects in the DBStore.
     * @param name Name under which the array is stored in the database (and DBStore).
     * @param required if false there will be no errors if the object cannot be
     *    found in the database. However this will require the user to check if
     *    the object is valid before each use.
     */
    explicit DBArray(const std::string& name = "", bool required = true):
      DBAccessorBase(DBStore::arrayName<T>(name), T::Class(), true, required) {}

    /** Get the number of objects in the array. */
    inline int getEntries() const { return isValid() ? getObject<TClonesArray>()->GetEntriesFast() : 0;}

    /** Access to the stored objects.
     *
     * Out-of-bounds accesses throw an std::out_of_range exception
     *
     * @param i  Array index, should be in 0..getEntries()-1
     * @return   pointer to the object
     */
    inline const T* operator [](int i) const
    {
      if (!isValid()) {
        throw std::out_of_range("Out-of-range access in DBArray::operator[], for " + getName() + ": Object not valid");
      }
      //At() checks for out-of-range and returns NULL in that case
      TObject* obj = getObject<TClonesArray>()->At(i);
      if (obj == nullptr)
        throw std::out_of_range("Out-of-range access in DBArray::operator[], for " + getName() + ", index " + std::to_string(i));
      return static_cast<T*>(obj); //type was checked by DataStore, so the cast is safe.
    }

    /** Access object by key instead of by index.
     * @param method pointer to a member function of the objects in the array
     * which will return the key for a given element
     * @param key key to look for. If none of the elements return this key then
     * NULL is returned
     */
    template<class KEY> const T* getByKey(KEY(T::*method)(void) const, KEY key) const
    {
      const TClonesArray& array = *getObject<TClonesArray>();
      for (int i = 0; i < getEntries(); i++) {
        T* obj = static_cast<T*>(array.At(i));
        if ((*obj.*method)() == key) {
          return obj;
        }
      }
      return nullptr;
    }

    /** Return const_iterator to first entry. */
    const_iterator begin() const { return const_iterator(getObject<TClonesArray>(), false); }
    /** Return const_iterator to last entry +1. */
    const_iterator end() const { return const_iterator(getObject<TClonesArray>(), true); }
  };

  /** Optional DBArray: This class behaves the same as the DBArray except that
   * it will not raise errors when an object could not be found in the
   * database.
   *
   * Users are responsible to check the validity of the array before using it.
   *
   * This is the same as using a DBArray with full constructor arguments to
   * specify whether it is optional but allows to use default constructor and
   * gives a clearer intention to readers.
   */
  template<class T> class OptionalDBArray: public DBArray<T> {
  public:
    /** Construct a new Array with a given name or with the default name which
     * is identical to the class name of T */
    explicit OptionalDBArray(const std::string& name = ""): DBArray<T>(name, false) {}
  };
}
