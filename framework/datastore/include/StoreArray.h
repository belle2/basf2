/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Thomas Kuhr                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef STOREARRAY_H
#define STOREARRAY_H

#include <framework/datastore/StoreAccessorBase.h>
#include <framework/datastore/DataStore.h>

#include <TClonesArray.h>

#include <utility>

namespace Belle2 {

  /** Accessor to stored TClonesArrays.
   *
   *  This is an accessor class for the TClonesArrays saved in the DataStore.
   *  To add new objects, please use the TClonesArray function
   *  <a href="http://root.cern.ch/root/htmldoc/TClonesArray.html#TClonesArray:New">new</a>.
   *  The TClonesArrays are never deleted, but their content is deleted according to the EDurability type
   *  that is given to them.
   *  @author <a href="mailto:belle2_software@bpost.kek.jp?subject=StoreArray">The basf2 developers</a>
   */
  template <class T>
  class StoreArray : public StoreAccessorBase {
  public:
    /** Constructor with assignment.
     *
     *  This constructor calls the assignArray function.
     *  @param name        Name under which the TClonesArray is stored.
     *  @param durability  Specifies lifetime of array in question.
     */
    explicit StoreArray(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event) {
      assignArray(name, durability);
    }

    /** Constructor for usage with Relations etc.
     *
     *  @param accessorParams   A pair with name and durability.
     */
    StoreArray(AccessorParams accessorParams) {
      assignArray(accessorParams.first, accessorParams.second);
    }

    /** Constructor for storage of TClonesArrays.
     *
     *  This is most likely needed for the multiprocessing implementation.
     *  @param name        Key under which TClonesArray is stored.
     *  @param durability  Lifetime of stored TClonesArray.
     *  @param array       TClonesArray for storage.
     */
    StoreArray(TClonesArray* const array, const std::string& name, DataStore::EDurability durability = DataStore::c_Event)
      : m_name(name), m_durability(durability), m_storeArray(array) {
      DataStore::Instance().handleArray<T>(m_name, durability, m_storeArray);
    }


    bool operator==(const StoreArray<T> &b) const {                /**< Check if two StoreArrays point to the same array. */
      return (b.m_name == m_name) && (b.m_durability == m_durability);
    }
    bool operator!=(const StoreArray<T> &b) const {                /**< ...or to different ones. */
      return *this != b;
    }

    /** Get the number of occupied slots in the array. */
    int getEntries() const {return m_storeArray->GetEntriesFast();}

    /** Is this StoreArray's data safe to access? */
    operator bool() const {return m_storeArray;}

    /** Access to the stored objects.
     *
     *  To add an element to the array, use:
     *   new (myStoreArray[myStoreArray.getEntries()]) T(some ctor arguments);
     *  which constructs a new T object at the end of myStoreArray.
     *
     *  If you only want to use T's default or copy constructor, use the safer
     *  appendNew() instead.
     *
     *
     *  \param i Array index, should be in 0..getEntries()-1 (no range check).
     *           Using i = getEntries() is O.K. when used with new (...) T, the
     *           array is expanded as necessary.
     *  \return const pointer to T (no left-hand side assignments)
     */
    inline T* const operator [](int i) const {
      //type was checked by DataStore, so this is safe
      return static_cast<T*>(m_storeArray->AddrAt(i));
    }

    /** Construct a new T object at the end of the array.
     *
     *  Appends a new object to the array, and returns a pointer so
     *  it can be filled with data. The default constructor is used
     *  for the object's creation.
     *
     *  \return const pointer to the created object (no left-hand side assignments)
     */
    inline T* const appendNew() { return new((*this)[getEntries()]) T(); }

    /** Copy-construct a new T object at the end of the array.
     *
     *  Appends a new object to the array, and returns a pointer so
     *  it can be filled with data. The copy-constructor of T is
     *  used to create the object.
     *
     *  \note For code that needs to store large numbers of objects in an
     *        array, you may want to avoid creating a temporary object for
     *        each of them. In this case, the default-constructing variant
     *        of appendNew() or placement-new with a custom constructor (see
     *        documentation of operator[]) may be better.
     *
     *  \return const pointer to the created object (no left-hand side assignments)
     */
    inline T* const appendNew(const T& obj) { return new((*this)[getEntries()]) T(obj); }

    //@{
    /** Raw access to the underlying TClonesArray.
     *
     *  In most cases, you'll want to avoid direct interaction with
     *  TClonesArrays and use StoreArray functions like operator[],
     *  getEntries() or appendNew() instead.
     *  If you must access the TClonesArray, using the getPtr()
     *  function is recommended, as the difference between . and ->
     *  may be lost on casual readers of the source code.
     */
    TClonesArray& operator *() const {return *m_storeArray;}
    TClonesArray* operator ->() const {return m_storeArray;}
    TClonesArray* getPtr() const {return m_storeArray;}
    //@}

    //------------------------ Getters for AccessorParams -----------------------------------------------------
    /** Returns name/durability under which the object is saved in the DataStore. */
    AccessorParams getAccessorParams() const {return make_pair(m_name, m_durability);};
    /** Return name under which the object is saved in the DataStore. */
    const std::string& getName() const { return m_name; }
    /** Return durability with which the object is saved in the DataStore. */
    DataStore::EDurability getDurability() const { return m_durability; }

  protected:
    /** Switch the array the StoreArray points to.
     *
     *  @param name       Key with which the TClonesArray is saved. An empty string is treated as equal to class name.
     *  @param durability Specifies lifetime of array in question.
     */
    bool assignArray(const std::string& name, DataStore::EDurability durability = DataStore::c_Event);

    /** Pointer that actually holds the TClonesArray. */
    TClonesArray* m_storeArray;

    /** Store name under which TClonesArray is saved. */
    std::string m_name;

    /**Store durability under which the TClonesArray is saved. */
    DataStore::EDurability m_durability;

  };

} // end namespace Belle2

//-------------------Implementation of template part of the class ---------------------------------
template <class T>
bool Belle2::StoreArray<T>::assignArray(const std::string& name, DataStore::EDurability durability)
{
  if (name == "") {
    m_name = DataStore::defaultArrayName<T>();
  } else {
    m_name = name;
  }
  m_durability = durability;
  m_storeArray = 0;

  B2DEBUG(250, "Calling DataStore from StoreArray " << name);
  return DataStore::Instance().handleArray<T>(m_name, durability, m_storeArray);
}

#endif
