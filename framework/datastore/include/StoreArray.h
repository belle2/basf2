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
   *  The TClonesArrays are never deleted, but their content is deleted according to the EDurability type,
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
     *  @param generate    Shall array be created, if none with name exists so far?
     */
    StoreArray(const std::string& name = "", const DataStore::EDurability& durability = DataStore::c_Event) {
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
    StoreArray(TClonesArray* const array, const std::string& name , const DataStore::EDurability& durability = DataStore::c_Event)
        : m_name(name), m_durability(durability), m_storeArray(array) {
      DataStore::Instance().handleArray<T>(m_name, durability, m_storeArray);
    }

    /** Switch the array, the StoreArray points to.
     *
     *  @param name       Key with which the TClonesArray is saved. An empty string is treated as equal to class name.
     *  @param durability Specifies lifetime of array in question.
     *  @param generate   Shall array be created, if none with name exists so far.
     */
    bool assignArray(const std::string& name, const DataStore::EDurability& durability = DataStore::c_Event);

    //------------------------ Imitate array functionality -----------------------------------------------------
    TClonesArray& operator *() const {return *m_storeArray;} /**< Imitate array functionality. */
    TClonesArray* operator ->() const {return m_storeArray;} /**< Imitate array functionality. */

    bool operator==(const StoreArray<T> &b) {                /**< Check if two StoreArrays point to the same array. */
      return (b.m_name == m_name) && (b.m_durability == m_durability);
    }
    bool operator!=(const StoreArray<T> &b) {                /**< ...or to different ones. */
      return (b.m_name != m_name) || (b.m_durability != m_durability);
    }
    operator bool() const {return m_storeArray;}  /**< Imitate array functionality. */

    /** Access to the stored objects.
     *
     *  By default the TClonesArray would return TObjects, so a cast is necessary.
     *  The static cast is save here, because at a previous stage, it is already checked,
     *  that the TClonesArray contains type T.
     */
    T* operator [](int i) const {return static_cast<T*>(m_storeArray->At(i));}

    TClonesArray* getPtr() {return m_storeArray;} /**< Return stored object. */

    //------------------------ Getters for AccessorParams -----------------------------------------------------
    /** Returns name under which the object is saved in the DataStore. */
    AccessorParams getAccessorParams() const {return make_pair(m_name, m_durability);};
    /** Return  name under which the object is saved in the DataStore. */
    std::string getName() const { return m_name; }
    /** Return  durability with which the object is saved in the DataStore. */
    DataStore::EDurability getDurability() const { return m_durability; }

    //------------------------ Parsing for easier handling of TClonesArrays -----------------------------------
    /** Get the number of occupied slots in the array. */
    int GetEntries() const {
      B2WARNING("This method is depreciated. Please use getEntries() instead!");
      return m_storeArray->GetEntriesFast();
    }
    /** Get the number of occupied slots in the array. */
    int getEntries() const {return m_storeArray->GetEntriesFast();}

  protected:
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
bool Belle2::StoreArray<T>::assignArray(const std::string& name, const DataStore::EDurability& durability)
{
  if (name == "") {
    m_name = DataStore::defaultArrayName<T>();
  } else {
    m_name       = name;
  }
  m_durability = durability;
  m_storeArray = 0;

  B2DEBUG(250, "Calling DataStore from StoreArray " << name);
  return DataStore::Instance().handleArray<T>(m_name, durability, m_storeArray);
}

#endif
