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
   *  <h1>Storing an array</h1>
   *  Note that you have to register an array in the initialize method of a
   *  module first (using registerPersistent()), and then create() it
   *  (in the event() method of a module for durability c_Event) before
   *  you can use it. This procedure is the same  as for objects handled by
   *  StoreObjPtr.
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
    /** Register an array, that should be written to the output by default, in the data store.
     *  This must be called in the initialzation phase.
     *
     *  @param name        Name under which the TClonesArray is stored.
     *  @param durability  Specifies lifetime of array in question.
     *  @param errorIfExisting  Flag whether an error will be reported if the array was already registered.
     *  @return            True if the registration succeeded.
     */
    static bool registerPersistent(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event,
                                   bool errorIfExisting = false) {
      return DataStore::Instance().createEntry(DataStore::arrayName<T>(name), durability, T::Class(), true, false, errorIfExisting);
    }

    /** Register an array, that should not be written to the output by default, in the data store.
     *  This must be called in the initialzation phase.
     *
     *  @param name        Name under which the TClonesArray is stored.
     *  @param durability  Specifies lifetime of array in question.
     *  @param errorIfExisting  Flag whether an error will be reported if the array was already registered.
     *  @return            True if the registration succeeded.
     */
    static bool registerTransient(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event,
                                  bool errorIfExisting = false) {
      return DataStore::Instance().createEntry(DataStore::arrayName<T>(name), durability, T::Class(), true, true, errorIfExisting);
    }

    /** Check whether an array was registered before.
     *  It will cause an error if the array does not exist.
     *  This must be called in the initialzation phase.
     *
     *  @param name        Name under which the TClonesArray is stored.
     *  @param durability  Specifies lifetime of array in question.
     *  @return            True if the array exists.
     */
    static bool required(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event) {
      std::string arrayName = DataStore::arrayName<T>(name);
      if (!DataStore::Instance().hasEntry(arrayName, durability, T::Class(), true)) {
        B2ERROR("The required DataStore entry with name " << arrayName << " and durability " << durability << " does not exists.");
        return false;
      }
      return true;
    }

    /** Constructor to access an array in the DataStore.
     *
     *  @param name       Name under which the array is stored in the DataStore.
     *                    If an empty string is supplied, the type name will be used.
     *  @param durability Decides durability map used for getting the accessed array.
     */
    explicit StoreArray(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event):
      StoreAccessorBase(DataStore::arrayName<T>(name), durability) {
      if (DataStore::Instance().getInitializeActive()) {
        //if we're called during initialization, register the array and print a warning
        DataStore::Instance().backwardCompatibleRegistration(m_name, m_durability, T::Class(), true);
      }
      m_storeArray = reinterpret_cast<TClonesArray**>(DataStore::Instance().getObject(m_name, durability, T::Class(), true));
    }

    /** Create an empty array in the data store.
     *
     *  @param replace   Should an existing object be replaced?
     *  @return          True if the creation succeeded.
     **/
    bool create(bool replace = false) {
      return DataStore::Instance().createObject(0, replace, m_name, m_durability, T::Class(), true);
    };

    /** Add an existing array to the data store.
     *
     *  @param object    The object that should be put in the DataStore.
     *  @param replace   Should an existing object be replaced?
     *  @return          True if the creation succeeded.
     **/
    bool assign(TObject* object, bool replace = false) {
      return DataStore::Instance().createObject(object, replace, m_name, m_durability, T::Class(), true);
    };

    /** Check whether the array was created.
     *
     *  @return          True if the array exists.
     **/
    inline bool isValid() const {return m_storeArray && *m_storeArray;}

    /** Is this StoreArray's data safe to access? */
    inline operator bool() const {return isValid();}

    /** Get the number of occupied slots in the array. */
    inline int getEntries() const { return isValid() ? ((*m_storeArray)->GetEntriesFast()) : (0);}

    /** Access to the stored objects.
     *
     *  \param i Array index, should be in 0..getEntries()-1
     *  \return pointer to the created object, or NULL if out of bounds
     */
    inline T* operator [](int i) const {
      if (i >= getEntries() or i < 0)
        return 0;
      //type was checked by DataStore, so this is safe
      return static_cast<T*>((*m_storeArray)->AddrAt(i));
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
      ensureAttached();
      return static_cast<T*>((*m_storeArray)->AddrAt(getEntries()));
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
    TClonesArray& operator *() const { ensureAttached(); return **m_storeArray;}
    ClonesArrayWrapper* operator ->() const { ensureAttached(); return static_cast<ClonesArrayWrapper*>(*m_storeArray);}
    TClonesArray* getPtr() const { ensureAttached(); return *m_storeArray;}
    //@}

  private:
    /** Ensure that this object is registered and attached. */
    inline void ensureAttached() const {
      DataStore::Instance().backwardCompatibleRegistration(m_name, m_durability, T::Class(), true);
      DataStore::Instance().backwardCompatibleCreation(m_name, m_durability, T::Class(), true);
      if (!m_storeArray) {
        const_cast<StoreArray*>(this)->m_storeArray = reinterpret_cast<TClonesArray**>(DataStore::Instance().getObject(m_name, m_durability, T::Class(), true));
      }
    }
    /** Pointer that actually holds the TClonesArray. */
    TClonesArray** m_storeArray;

  };
} // end namespace Belle2
#endif
