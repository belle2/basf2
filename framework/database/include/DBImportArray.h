/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>

#include <TClonesArray.h>
#include <stdexcept>

namespace Belle2 {

  /**
   * Class for importing array of objects to the database.
   * Note that the array is NOT parked at DBStore, but allocated internally.
   */
  template<class T> class DBImportArray {
  public:

    /**
     * Constructor
     * @param module  Name under which the array will be stored in the database
     * @param package Package name
     */
    explicit DBImportArray(const std::string& module = "",
                           const std::string& package = "dbstore"):
      m_package(package)
    {
      m_module = DBStore::arrayName<T>(module);
      m_array = new TClonesArray(T::Class());
    }

    /**
     * Destructor
     */
    ~DBImportArray()
    {
      delete m_array; // does this delete also allocated elements?
    }

    /**
     * Return number of objects in the array.
     */
    inline int getEntries() const { return m_array->GetEntriesFast();}

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
     * Import the array to database
     * @param iov interval of validity
     */
    inline bool import(IntervalOfValidity& iov)
    {
      return Database::Instance().storeData(m_package, m_module, m_array, iov);
    }


  private:

    /**
     * Returns address of the next free position of the array.
     * @return pointer to address just past the last array element
     */
    inline T* nextFreeAdress()
    {
      return static_cast<T*>(m_array->AddrAt(getEntries()));
    }

    std::string m_module;      /**< array name in database */
    std::string m_package;     /**< package name */
    TClonesArray* m_array = 0; /**< pointer to allocated array */

  };
}
