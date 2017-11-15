/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Thomas Kuhr                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreAccessorBase.h>
#include <framework/datastore/DataStore.h>

#include <framework/utilities/ArrayIterator.h>

#include <TClonesArray.h>
#include <stdexcept>

namespace Belle2 {
  /** hide some implementation details. */
  namespace _StoreArrayImpl {
    /** clear all relations touching the given array. */
    void clearRelations(const StoreAccessorBase& array);
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
   *  <h2>Accessing elements of an existing array</h2>
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

      //or using range-based for:
      for(const CDCSimHit& hit : cdcsimhits) {
        // Use hit's data here...
      }
      \endcode
   *
   *  \note Remember to use references or pointers when iterating,
   *        otherwise accessing, e.g., relations will not work.
   *
   *  Objects linked together using relations can also be obtained by querying
   *  the objects themselves, since they should derive from RelationsObject
   *  (See class documentation for usage examples.)
   *
   *
   *  <h2>Adding elements</h2>
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
   *  <h2>Registration of arrays</h2>
   *  Note that you have to register an array in the initialize method of a
   *  module first (using registerInDataStore()) before you can use it.
   *  This procedure is the same  as for objects handled by StoreObjPtr.
   *
   *
   *  <h2>Using StoreArray as a module member variable</h2>
   *  To avoid some overhead involved in re-creating the StoreArray e.g. in
   *  each event() function call, you can also make StoreArray a member variable
   *  of your class. If it is of non-event durability, you'll need to add the appropriate
   *  constructor call to the initializer list, e.g. (here with default name):
   *
      \code
      MyModule::MyModule():
        m_fooHits("", DataStore::c_Persistent)
      {}
      \endcode
   *
   *  In initialize(), you should also use registerInDataStore() or isOptional()/isRequired()
   *  to specify wether it is an input or output.
   *  For <b>non-default names</b> (which you might not know in the constructor, e.g. in the
   *  case of module parameters), set the 'name' argument of any of these three functions to
   *  permanently bind the StoreArray to the array with the given name.
   *
   *
   *  @sa Objects in different arrays can be linked using relations, see RelationsInterface, RelationsObject
   *  @sa See StoreObjPtr for a way store single objects
   *  @sa Data can also be accessed from Python modules using PyStoreArray
   *  @sa While individual elements cannot be removed directly from StoreArray, SelectSubset can be used to filter it.
   */
  template <class T>
  class StoreArray : public StoreAccessorBase {
  public:
    /** STL-like iterator over the T objects (not T* ). */
    typedef ObjArrayIterator<TClonesArray, T> iterator;
    /** STL-like const_iterator over the T objects (not T* ). */
    typedef ObjArrayIterator<const TClonesArray, const T> const_iterator;

    /** Register an array, that should be written to the output by default, in the data store.
     *  This must be called in the initialization phase.
     *
     *  @warning Use discouraged, use registerInDataStore() instead.
     *  @param name        Name under which the TClonesArray is stored.
     *  @param durability  Specifies lifetime of array in question.
     *  @param errorIfExisting  Flag whether an error will be reported if the array was already registered.
     *  @return            True if the registration succeeded.
     */
    [[deprecated("Please use registerInDataStore() on an instance and consider making the StoreArray a member of your class")]]
    static bool registerPersistent(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event,
                                   bool errorIfExisting = false)
    {
      return DataStore::Instance().registerEntry(DataStore::arrayName<T>(name), durability, T::Class(), true,
                                                 errorIfExisting ? DataStore::c_ErrorIfAlreadyRegistered : DataStore::c_WriteOut);

    }

    /** Check whether an array was registered before.
     *  It will cause an error if the array does not exist.
     *  This must be called in the initialization phase.
     *
     *  @warning Use discouraged, use isRequired() instead.
     *  @param name        Name under which the TClonesArray is stored.
     *  @param durability  Specifies lifetime of array in question.
     *  @return            True if the array exists.
     */
    [[deprecated("Please use isRequired() on an instance and consider making the StoreArray a member of your class")]]
    static bool required(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event)
    {
      std::string arrayName = DataStore::arrayName<T>(name);
      return DataStore::Instance().requireInput(StoreAccessorBase(arrayName, durability, T::Class(), true));
    }

    /** Tell the data store about an optional input.
     *
     *  Mainly useful for creating diagrams of module inputs and outputs.
     *
     *
     *  @warning Use discouraged, use isOptional() instead.
     *  @param name        Name under which the TClonesArray is stored.
     *  @param durability  Specifies lifetime of array in question.
     *  @return            True if the array exists.
     */
    [[deprecated("Please use isOptional() on an instance and consider making the StoreArray a member of your class")]]
    static bool optional(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event)
    {
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
     * @param storeFlags ORed combination of DataStore::EStoreFlags
     */
    template <class TO> bool registerRelationTo(const StoreArray<TO>& toArray, DataStore::EDurability durability = DataStore::c_Event,
                                                DataStore::EStoreFlags storeFlags = DataStore::c_WriteOut) const
    {
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
    template <class TO> bool requireRelationTo(const StoreArray<TO>& toArray,
                                               DataStore::EDurability durability = DataStore::c_Event) const
    {
      return DataStore::Instance().requireRelation(*this, toArray, durability);
    }

    /** Tell the data store about a relation that we could make use of. (aka. optional input)
     *
     * Mainly useful for creating diagrams of module inputs and outputs.
     *
     *
     * @param toArray    Array the relation should point to (from this StoreArray)
     * @param durability Durability of the relation.
     * @return            True if the relations exists.
     */
    template <class TO> bool optionalRelationTo(const StoreArray<TO>& toArray,
                                                DataStore::EDurability durability = DataStore::c_Event) const
    {
      return DataStore::Instance().optionalRelation(*this, toArray, durability);
    }

    /** Delete all entries in this array.
     *
     * Any relations to objects in this array are also removed.
     */
    void clear() override
    {
      if (getEntries() != 0) {
        (*m_storeArray)->Delete();
        _StoreArrayImpl::clearRelations(*this);
      }
    }

    /** Get the number of objects in the array. */
    inline int getEntries() const { return isCreated() ? ((*m_storeArray)->GetEntriesFast()) : 0;}

    /** Access to the stored objects.
     *
     *  Out-of-bounds accesses throw an std::out_of_range exception
     *
     *  Note that using iterators (or range-based for) avoids the range-check internally,
     *  and thus might be slightly faster.
     *
     *  \param i Array index, should be in 0..getEntries()-1
     *  \return pointer to the object
     */
    inline T* operator [](int i) const
    {
      ensureCreated();
      //At() checks for out-of-range and returns NULL in that case
      TObject* obj = (*m_storeArray)->At(i);
      if (obj == nullptr)
        throw std::out_of_range("Out-of-range access in StoreArray::operator[], for " + readableName() + ", index " + std::to_string(i));
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
    template<class ...Args> T* appendNew(Args&& ... params)
    {
      return new(nextFreeAdress()) T(std::forward<Args>(params)...);
    }


    /** Return list of array names with matching type.  */
    static std::vector<std::string> getArrayList(DataStore::EDurability durability = DataStore::c_Event)
    {
      return DataStore::Instance().getListOfArrays(T::Class(), durability);
    }


    /** Check wether the array was registered.
     *
     *  @note Iterating over the array or calling getEntries() is safe even
     *        if this returns false. (getEntries() will return 0)
     *
     *  @return          True if the array was registered.
     **/
    inline bool isValid() const
    {
      ensureAttached();
      return m_storeArray;
    }

    /** Check wether the array was registered.
     *
     *  @note Iterating over the array or calling getEntries() is safe even
     *        if this returns false. (getEntries() will return 0)
     *
     *  @return          True if the array was registered.
     **/
    inline operator bool() const { return isValid(); }

    /** Raw access to the underlying TClonesArray.
     *
     *  \warning TClonesArray is dangerously easy to misuse. Whatever you do will probably
     *           be slow, leak memory, and murder your pets.
     *           In most cases, you'll want to use StoreArray functions like operator[],
     *           getEntries() or appendNew() instead.
     *  \returns pointer to TClonesArray, or NULL if this array was not registered in the data store
     */
    TClonesArray* getPtr() const
    {
      ensureCreated();
      return (m_storeArray ? *m_storeArray : nullptr);
    }

    /** Return iterator to first entry. */
    iterator begin() { ensureAttached(); return iterator(m_storeArray); }
    /** Return iterator to last entry +1. */
    iterator end() { ensureAttached(); return iterator(m_storeArray, true); }

    /** Return const_iterator to first entry. */
    const_iterator begin() const { ensureAttached(); return const_iterator(m_storeArray); }
    /** Return const_iterator to last entry +1. */
    const_iterator end() const { ensureAttached(); return const_iterator(m_storeArray, true); }

  private:
    /** Creating StoreArrays is unnecessary, only used internally. */
    bool create(bool replace = false) { return StoreAccessorBase::create(replace); }

    /** Returns address of the next free position of the array.
     *
     *  \return pointer to address just past the last array element
     */
    inline T* nextFreeAdress()
    {
      ensureCreated();
      return static_cast<T*>((*m_storeArray)->AddrAt(getEntries()));
    }

    /** Ensure that this object is attached. */
    inline void ensureAttached() const
    {
      if (!m_storeArray) {
        const_cast<StoreArray*>(this)->m_storeArray = reinterpret_cast<TClonesArray**>(DataStore::Instance().getObject(*this));
      }
    }
    /** Ensure that the array has been created.
     *
     * Called automatically by write operations.
     */
    inline void ensureCreated() const
    {
      if (!isCreated()) {
        if (!const_cast<StoreArray*>(this)->create())
          throw std::runtime_error("Write access to " + readableName() + " failed, did you remember to call registerInDataStore()?");
      }
    }

    /** Check wether the array object was created.  **/
    inline bool isCreated() const
    {
      ensureAttached();
      return m_storeArray && *m_storeArray;
    }

    /** Pointer that actually holds the TClonesArray. */
    TClonesArray** m_storeArray;

  };
} // end namespace Belle2
