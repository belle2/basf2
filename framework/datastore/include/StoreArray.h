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

  /** Accessor to arrays stored in the data store.
   *
   *  StoreArrays (like StoreObjPtrs) are uniquely identified by their name
   *  and durability. In most cases, arrays are created with durability
   *  DataStore::c_Event and the default name corresponding to their type.
   *  (i.e. typename + 's')
   *  Thus, calling the constructor StoreArray<MyType>() will connect this
   *  StoreArray with the array called 'MyTypes' in the data store, with a
   *  lifetime of one event.
   *
   *
   *  Accessing elements of an existing array
   *  =======================================
   *
   *  Stored objects can be accessed directly using their array index and
   *  operator[]. For example, the following code snippet loops over all
   *  entries in an array of CDCSimHits:
   *
   *      StoreArray<CDCSimHit> cdcsimhits;
   *      //loop over all CDC simhits
   *      for(int iCDC = 0; iCDC < cdcsimhits.getEntries(); iCDC++) {
   *        const CDCSimHit* hit = cdcsimhits[iCDC]; //get iCDC'th entry in StoreArray
   *        // Use hit's data here...
   *      }
   *
   *  Objects linked together using relations can also be obtained through
   *  the RelationIndex class. (See class documentation for usage examples.)
   *
   *
   *  Adding elements
   *  ===============
   *
   *  Elements can be added to the array in a few ways. The easiest is to use
   *  something like:
   *
   *      StoreArray<CDCSimHit> cdcsimhits;
   *      //...
   *      CDCSimHit* newhit = cdcsimhits.appendNew();
   *      //fill newhit with data here...
   *
   *  alternatively, you can copy an existing object into the array using
   *  appendNew(const T& obj) instead.
   *  If performance is especially important, you can also create a new object
   *  using 'placement-new':
   *
   *      new (cdcsimhits[cdcsimhits.getEntries()]) CDCSimHit(some ctor arguments);
   *
   *  This creates a new CDCSimHit at the end of the array and allows
   *  you to fill its data members using a custom constructor.
   *
   *
   *  Note that if you want to create a new array in a module, you should
   *  create an object of type StoreArray<T> in your implementation of
   *  Module::initialize(). This registers the array in the data store and
   *  lets other modules know you intend to fill it.
   *
   *
   *  Internals
   *  =========
   *
   *  Internally, the arrays are stored as TClonesArrays, see the
   *  [ROOT documentation on TClonesArray](http://root.cern.ch/root/html/TClonesArray.html)
   *  for technical details.
   *
   *  @sa objects in different arrays can be linked using relations, see
   *      RelationArray or RelationIndex.
   *  @sa see StoreObjPtr for a way store single objects
   *
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
    explicit StoreArray(AccessorParams accessorParams) {
      assignArray(accessorParams.first, accessorParams.second);
    }

    /** Constructor for storage of TClonesArrays.
     *
     *  This is most likely needed for the multiprocessing implementation.
     *  @param array       TClonesArray for storage.
     *  @param name        Key under which TClonesArray is stored.
     *  @param durability  Lifetime of stored TClonesArray.
     */
    StoreArray(TClonesArray* const array, const std::string& name, DataStore::EDurability durability = DataStore::c_Event)
      : m_name(name), m_durability(durability), m_storeArray(array) {
      DataStore::Instance().handleArray<T>(m_name, durability, m_storeArray);
    }

    /** Check if two StoreArrays point to the same array. */
    bool operator==(const StoreArray<T> &b) const {
      return (b.m_name == m_name) && (b.m_durability == m_durability);
    }

    /** Check if two StoreArrays point to the different arrays. */
    bool operator!=(const StoreArray<T> &b) const {
      return !(*this == b);
    }

    /** Is this StoreArray's data safe to access? */
    operator bool() const {return m_storeArray;}

    /** Get the number of occupied slots in the array. */
    inline int getEntries() const {return m_storeArray->GetEntriesFast();}

    /** Access to the stored objects.
     *
     *  To add an element to the array, use:
     *
     *      new (myStoreArray[myStoreArray.getEntries()]) T(some ctor arguments);
     *
     *  which constructs a new T object at the end of myStoreArray.
     *
     *  If you only want to use T's default or copy constructor, use the safer
     *  appendNew() instead.
     *
     *
     *  \param i Array index, should be in 0..getEntries()-1 (no range check).
     *           Using i = getEntries() is O.K. when used with new (...) T, the
     *           array is expanded as necessary.
     *  \return pointer to the created object
     */
    inline T* operator [](int i) const {
      //type was checked by DataStore, so this is safe
      return static_cast<T*>(m_storeArray->AddrAt(i));
    }

    /** Construct a new T object at the end of the array.
     *
     *  Appends a new object to the array, and returns a pointer so
     *  it can be filled with data. The default constructor is used
     *  for the object's creation.
     *
     *  \return pointer to the created object
     */
    inline T* appendNew() { return new((*this)[getEntries()]) T(); }

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
     *  \return pointer to the created object
     */
    inline T* appendNew(const T& obj) { return new((*this)[getEntries()]) T(obj); }

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
