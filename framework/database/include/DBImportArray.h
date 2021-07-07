/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/database/DBImportBase.h>
#include <framework/utilities/ArrayIterator.h>
#include <TClonesArray.h>
#include <stdexcept>

namespace Belle2 {

  /**
   * Class for importing array of objects to the database.
   * Note that the array is NOT parked at DBStore, but allocated internally.
   */
  template<class T> class DBImportArray: public  DBImportBase {
  public:

    /** STL-like iterator over the T objects (not T* ). */
    typedef ObjArrayIterator<TClonesArray, T> iterator;
    /** STL-like const_iterator over the T objects (not T* ). */
    typedef ObjArrayIterator<const TClonesArray, const T> const_iterator;

    /**
     * Constructor
     * @param name  Name under which the array will be stored in the database
     */
    explicit DBImportArray(const std::string& name = ""):
      DBImportBase(DBStore::arrayName<T>(name))
    {
      m_object = new TClonesArray(T::Class());
    }

    /**
     * Destructor
     */
    ~DBImportArray()
    {
      delete m_object;
    }

    /**
     * Returns number of objects in the array.
     */
    inline int getEntries() const
    {
      return static_cast<TClonesArray*>(m_object)->GetEntriesFast();
    }

    /**
     * Construct a new T object at the end of the array.
     *
     * Appends a new object to the array, and returns a pointer so
     * it can be filled with data. The default constructor is used
     * for the object's creation.
     * @return pointer to the created object
     */
    inline T* appendNew() { return new(nextFreeAdress()) T(); }

    /**
     * Construct a new T object at the end of the array.
     *
     * This is done by forwarding all arguments to the constructor of the type T.
     * If there is a constructor which takes the given combination of arguments
     * then this call will succeed, otherwise it fails on compilation.
     *
     * This method imposes no overhead as no temporary has to be constructed
     * and should be the preferred solution for creating new objects.
     * @return pointer to the created object
     */
    template<class ...Args> T* appendNew(Args&& ... params)
    {
      return new(nextFreeAdress()) T(std::forward<Args>(params)...);
    }

    /**
     * Access to the stored objects.
     * Out-of-bounds accesses throw an std::out_of_range exception.
     * @param i  Array index, should be in 0..getEntries()-1
     * @return   pointer to the object
     */
    inline T* operator [](int i) const
    {
      TObject* obj = static_cast<TClonesArray*>(m_object)->At(i);
      if (obj == nullptr)
        throw std::out_of_range("Out-of-range access in DBImportArray::operator[], for "
                                + getName() + ", index "
                                + std::to_string(i));
      return static_cast<T*>(obj);
    }

    /**
     * Returns iterator to first entry.
     */
    iterator begin() { return iterator(*static_cast<TClonesArray*>(m_object), 0); }

    /**
     * Returns iterator to last entry +1.
     */
    iterator end() { return iterator(*static_cast<TClonesArray*>(m_object), getEntries()); }

    /**
     * Returns const_iterator to first entry.
     */
    const_iterator begin() const { return const_iterator(*static_cast<TClonesArray*>(m_object), 0); }

    /**
     * Returns const_iterator to last entry +1.
     */
    const_iterator end() const { return const_iterator(*static_cast<TClonesArray*>(m_object), getEntries()); }

    /**
     * add event dependency
     * @param eventNumber event number
     */
    void addEventDependency(unsigned int eventNumber) override
    {
      addIntraRunDependency(eventNumber, c_Event);
      m_object = new TClonesArray(T::Class());
    }

    /**
     * add time stamp dependency
     * @param timeStamp time stamp
     */
    void addTimeStampDependency(unsigned long long int timeStamp) override
    {
      addIntraRunDependency(timeStamp, c_TimeStamp);
      m_object = new TClonesArray(T::Class());
    }

    /**
     * add subrun dependency
     * @param subrun subrun number
     */
    void addSubrunDependency(int subrun) override
    {
      addIntraRunDependency(subrun, c_Subrun);
      m_object = new TClonesArray(T::Class());
    }

    /**
     * Clear the content, e.g. destroy allocated objects and prepare for the new DB import.
     */
    void clear() override
    {
      DBImportBase::clear();
      m_object = new TClonesArray(T::Class());
    }


  private:

    /**
     * Returns address of the next free position of the array.
     * @return pointer to address just past the last array element
     */
    inline T* nextFreeAdress()
    {
      return static_cast<T*>(static_cast<TClonesArray*>(m_object)->AddrAt(getEntries()));
    }

  };
}
