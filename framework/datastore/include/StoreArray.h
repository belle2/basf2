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

#include <framework/utilities/ArrayIterator.h>

#include <TClonesArray.h>
#include <stdexcept>

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
   *  Objects linked together using relations can also be obtained by querying
   *  the objects themselves, since they should derive from RelationsObject
   *  (See class documentation for usage examples.)
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
   *  appendNew() can also use non-default constructors, e.g. if there is a constructor
   *  that takes the arguments (int, float), you can use appendNew(int, float) instead.
   *
   *
   *  <h1>Registration of arrays</h1>
   *  Note that you have to register an array in the initialize method of a
   *  module first (using registerInDataStore()) before you can use it.
   *  This procedure is the same  as for objects handled by StoreObjPtr.
   *
   *
   *  <h1>Internals</h1>
   *  Internally, the arrays are stored as TClonesArrays, see
   *  http://root.cern.ch/root/html/TClonesArray.html for technical details.
   *
   *  @sa Objects in different arrays can be linked using relations, see RelationsInterface, RelationsObject
   *  @sa See StoreObjPtr for a way store single objects
   *  @sa Data can also be accessed from Python modules using PyStoreArray
   *
   *  @author <a href="mailto:belle2_software@bpost.kek.jp?subject=StoreArray">The basf2 developers</a>
   */
  template <class T>
  class StoreArray : public StoreAccessorBase {
  public:
    /** STL-like iterator over the T objects (not T* ). */
    typedef ArrayIterator<StoreArray<T>, T> iterator;
    /** STL-like const_iterator over the T objects (not T* ). */
    typedef ArrayIterator<StoreArray<T>, const T> const_iterator;

    /** Register an array, that should be written to the output by default, in the data store.
     *  This must be called in the initialization phase.
     *
     *  @param name        Name under which the TClonesArray is stored.
     *  @param durability  Specifies lifetime of array in question.
     *  @param errorIfExisting  Flag whether an error will be reported if the array was already registered.
     *  @return            True if the registration succeeded.
     */
    static bool registerPersistent(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event,
                                   bool errorIfExisting = false) {
      return DataStore::Instance().registerEntry(DataStore::arrayName<T>(name), durability, T::Class(), true, errorIfExisting ? DataStore::c_ErrorIfAlreadyRegistered : 0);

    }

    /** Register an array, that should not be written to the output by default, in the data store.
     *  This must be called in the initialization phase.
     *
     *  @param name        Name under which the TClonesArray is stored.
     *  @param durability  Specifies lifetime of array in question.
     *  @param errorIfExisting  Flag whether an error will be reported if the array was already registered.
     *  @return            True if the registration succeeded.
     */
    static bool registerTransient(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event,
                                  bool errorIfExisting = false) {
      return DataStore::Instance().registerEntry(DataStore::arrayName<T>(name), durability, T::Class(), true, DataStore::c_DontWriteOut | (errorIfExisting ? DataStore::c_ErrorIfAlreadyRegistered : 0));
    }

    /** Check whether an array was registered before.
     *  It will cause an error if the array does not exist.
     *  This must be called in the initialization phase.
     *
     *  @param name        Name under which the TClonesArray is stored.
     *  @param durability  Specifies lifetime of array in question.
     *  @return            True if the array exists.
     */
    static bool required(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event) {
      std::string arrayName = DataStore::arrayName<T>(name);
      return DataStore::Instance().requireInput(StoreAccessorBase(arrayName, durability, T::Class(), true));
    }

    /** Tell the data store about an optional input.
     *
     *  Mainly useful for creating diagrams of module inputs and outputs.
     *  This must be called in the initialization phase.
     *
     *  @param name        Name under which the TClonesArray is stored.
     *  @param durability  Specifies lifetime of array in question.
     *  @return            True if the array exists.
     */
    static bool optional(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event) {
      std::string arrayName = DataStore::arrayName<T>(name);
      return DataStore::Instance().optionalInput(StoreAccessorBase(arrayName, durability, T::Class(), true));
    }

    /** Constructor to access an array in the DataStore.
     *
     *  @param name       Name under which the array is stored in the DataStore.
     *                    If an empty string is supplied, the type name will be used.
     *  @param durability Decides durability map used for getting the accessed array.
     */
    explicit StoreArray(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event):
      StoreAccessorBase(DataStore::arrayName<T>(name), durability, T::Class(), true), m_storeArray(0) {}


    /** Register a relation to the given StoreArray.
     *
     *  Use this if you want to create relate objects in this array to objects in 'toArray'.
     *  Must be called in the initialization phase.
     *
     * @param toArray    Array the relation should point to (from this StoreArray)
     * @param durability Durability of the relation.
     * @param storeFlags ORed combination of DataStore::EStoreFlag flags
     */
    template <class TO> bool registerRelationTo(const StoreArray<TO>& toArray, DataStore::EDurability durability = DataStore::c_Event, DataStore::EStoreFlags storeFlags = DataStore::c_WriteOut) const {
      return DataStore::Instance().registerRelation(*this, toArray, durability, storeFlags);
    }

    /** Produce error if no relation from this array to 'toArray' has been registered.
     *
     * Must be called in initialization phase, aborts job if it fails. (allowing you to catch problems early)
     *
     * @param toArray    Array the relation should point to (from this StoreArray)
     * @param durability Durability of the relation.
     * @return            True if the relations exists.
     */
    template <class TO> bool requireRelationTo(const StoreArray<TO>& toArray, DataStore::EDurability durability = DataStore::c_Event) const {
      return DataStore::Instance().requireRelation(*this, toArray, durability);
    }

    /** Tell the data store about a relation that we could make use of. (aka. optional input)
     *
     * Mainly useful for creating diagrams of module inputs and outputs.
     * This must be called in the initialization phase.
     *
     * @param toArray    Array the relation should point to (from this StoreArray)
     * @param durability Durability of the relation.
     * @return            True if the relations exists.
     */
    template <class TO> bool optionalRelationTo(const StoreArray<TO>& toArray, DataStore::EDurability durability = DataStore::c_Event) const {
      return DataStore::Instance().optionalRelation(*this, toArray, durability);
    }

    /** Delete all entries in this array.
     *
     * TODO: currently produces dangling relations if any were created
     */
    void clear() override {
      if (isValid())
        (*m_storeArray)->Delete();
    }

    /** Check whether the array was created.
     *
     *  @return          True if the array exists.
     **/
    inline bool isValid() const {
      ensureAttached();
      return m_storeArray && *m_storeArray;
    }

    /** Is this StoreArray's data safe to access? */
    inline operator bool() const { return isValid(); }

    /** Get the number of objects in the array. */
    inline int getEntries() const { return isValid() ? ((*m_storeArray)->GetEntriesFast()) : 0;}

    /** Access to the stored objects.
     *
     *  Out-of-bounds accesses throw an std::out_of_range exception
     *
     *  \param i Array index, should be in 0..getEntries()-1
     *  \return pointer to the object
     */
    inline T* operator [](int i) const {
      ensureCreated();
      //At() checks for out-of-range and returns NULL in that case
      TObject* obj = (*m_storeArray)->At(i);
      if (obj == nullptr)
        throw std::out_of_range("Out-of-range access in StoreArray::operator[], for " + readableName());
      return static_cast<T*>(obj); //type was checked by DataStore, so the cast is safe.
    }

    /** Construct a new T object at the end of the array.
     *
     *  Appends a new object to the array, and returns a pointer so
     *  it can be filled with data. The default constructor is used
     *  for the object's creation.
     *
     *  \return pointer to the created object
     */
    inline T* appendNew() { return new(nextFreeAdress()) T(); }

    /** Construct a new T object directly at the end of the array.
     *
     * This is done by forwarding all arguments to the constructor of the type T.
     * If there is a constructor which takes the given combination of arguments
     * then this call will succeed, otherwise it fails on compilation.
     *
     * This method imposes no overhead as no temporary has to be constructed
     * and should be the preferred solution for creating new objects.
     *
     * It handles just like the (C++11) standard library's emplace..() functions:
     * \code
       myStoreArray.appendNew(some ctor arguments);
       \endcode

     * \note since all arguments are passed to the constructor of class T,
     *       doing something like appendNew(T(ctor arguments)) is unnecessary. This would construct a temporary object of class T,
     *       then copy-construct another T object in the array and destroy the temporary object again.
     *
     *  \return pointer to the created object
     */
    template<class ...Args> T* appendNew(Args&& ... params) {
      return new(nextFreeAdress()) T(std::forward<Args>(params)...);
    }


    /** Return list of array names with matching type.  */
    static std::vector<std::string> getArrayList(DataStore::EDurability durability = DataStore::c_Event) {
      return DataStore::Instance().getListOfArrays(T::Class(), durability);
    }


    /** Raw access to the underlying TClonesArray.
     *
     *  \warning TClonesArray is dangerously easy to misuse. Whatever you do will probably
     *           be slow, leak memory, and murder your pets.
     *           In most cases, you'll want to use StoreArray functions like operator[],
     *           getEntries() or appendNew() instead.
     */
    TClonesArray* getPtr() const { ensureCreated(); return *m_storeArray;}

    /** Return iterator to first entry. */
    iterator begin() { ensureAttached(); return iterator(this, 0); }
    /** Return iterator to last entry +1. */
    iterator end() { ensureAttached(); return iterator(this, getEntries()); }

    /** Return const_iterator to first entry. */
    const_iterator begin() const { ensureAttached(); return const_iterator(this, 0); }
    /** Return const_iterator to last entry +1. */
    const_iterator end() const { ensureAttached(); return const_iterator(this, getEntries()); }

  private:
    /** Returns address of the next free position of the array.
     *
     *  \return pointer to address just past the last array element
     */
    inline T* nextFreeAdress() {
      ensureCreated();
      return static_cast<T*>((*m_storeArray)->AddrAt(getEntries()));
    }

    /** Ensure that this object is attached. */
    inline void ensureAttached() const {
      if (!m_storeArray) {
        const_cast<StoreArray*>(this)->m_storeArray = reinterpret_cast<TClonesArray**>(DataStore::Instance().getObject(*this));
      }
    }
    /** Ensure that the array has been created.
     *
     * Called automatically by write operations.
     */
    inline void ensureCreated() const {
      if (!isValid()) {
        const_cast<StoreArray*>(this)->create();
      }
    }

    /** Pointer that actually holds the TClonesArray. */
    TClonesArray** m_storeArray;

  };
} // end namespace Belle2
#endif
