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
  /** A wrapper class to hide unwanted TClonesArray members when using StoreArray::operator->. */
  class ClonesArrayWrapper : public TClonesArray {
  private:
    /** hide TClonesArray::GetEntries() */
    Int_t GetEntries() const { return 0; }
  };

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
   *  <h1>Accessing elements of an existing array</h1>
   *  Stored objects can be accessed directly using their array index and
   *  operator[]. For example, the following code snippet loops over all
   *  entries in an array of CDCSimHits:
   *
      \code
  StoreArray<CDCSimHit> cdcsimhits;
  //loop over all CDC simhits
  for(int iCDC = 0; iCDC < cdcsimhits.getEntries(); iCDC++) {
    const CDCSimHit* hit = cdcsimhits[iCDC]; //get iCDC'th entry in StoreArray
    // Use hit's data here...
  }
      \endcode
   *
   *  Objects linked together using relations can also be obtained through
   *  the RelationIndex class. (See class documentation for usage examples.)
   *
   *
   *  <h1>Adding elements</h1>
   *  Elements can be added to the array in a few ways. The easiest is to use
   *  something like:
   *
      \code
  StoreArray<CDCSimHit> cdcsimhits;
  //...
  CDCSimHit* newhit = cdcsimhits.appendNew();
  //fill newhit with data here...
      \endcode
   *
   *  alternatively, you can copy an existing object into the array using
   *  appendNew(const T& obj) instead.
   *  If performance is especially important, you can also create a new object
   *  using 'placement-new':
   *  \code
  new (cdcsimhits.nextFreeAddress()) CDCSimHit(some ctor arguments);
      \endcode

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
   *  <h1>Internals</h1>
   *  Internally, the arrays are stored as TClonesArrays, see
   *  http://root.cern.ch/root/html/TClonesArray.html for technical details.
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

    /** Is this StoreArray's data safe to access? */
    operator bool() const {return m_storeArray;}

    /** Get the number of occupied slots in the array. */
    inline int getEntries() const {return m_storeArray->GetEntriesFast();}

    /** Access to the stored objects.
     *
     *  \param i Array index, should be in 0..getEntries()-1
     *  \return pointer to the created object, or NULL if out of bounds
     */
    inline T* operator [](int i) const {
      if (i >= getEntries() or i < 0)
        return 0;
      //type was checked by DataStore, so this is safe
      return static_cast<T*>(m_storeArray->AddrAt(i));
    }

    /** Returns address of the next free position of the array.
     *
     *  To add an element to the array, use:
     *  \code
    new (myStoreArray.nextFreeAddress()) T(some ctor arguments);
        \endcode
     *  which constructs a new T object at the end of myStoreArray.
     *
     *  If you only want to use T's default or copy constructor, use the safer
     *  appendNew() instead.
     *
     *  \return pointer to address just past the last array element
     */
    inline T* nextFreeAddress() {
      return static_cast<T*>(m_storeArray->AddrAt(getEntries()));
    }

    /** Construct a new T object at the end of the array.
     *
     *  Appends a new object to the array, and returns a pointer so
     *  it can be filled with data. The default constructor is used
     *  for the object's creation.
     *
     *  \return pointer to the created object
     */
    inline T* appendNew() { return new(nextFreeAddress()) T(); }

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
     *        documentation of nextFreeAddress() ) may be better.
     *
     *  \return pointer to the created object
     */
    inline T* appendNew(const T& obj) { return new(nextFreeAddress()) T(obj); }

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
    ClonesArrayWrapper* operator ->() const {return static_cast<ClonesArrayWrapper*>(m_storeArray);}
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
