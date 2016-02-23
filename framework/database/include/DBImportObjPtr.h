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

namespace Belle2 {

  /**
   * Class for importing object to the database.
   * Note that the object is NOT parked at DBStore, but allocated internally.
   */
  template<class T> class DBImportObjPtr {
  public:

    /**
     * Constructor:
     * the object itself is not allocated here, but in construct(...) function.
     * @param module  Name under which the object will be stored in the database
     * @param package Package name
     */
    explicit DBImportObjPtr(const std::string& module = "",
                            const std::string& package = "dbstore"):
      m_module(DBStore::objectName<T>(module)),
      m_package(package)
    {}

    /**
     * Destructor
     */
    ~DBImportObjPtr()
    {
      if (m_object) delete m_object;
    }

    /**
     * Construct an object of type T in this DBImportObjPtr
     * using the provided constructor arguments.
     */
    template<class ...Args> void construct(Args&& ... params)
    {
      if (m_object) delete m_object;
      m_object = new T(std::forward<Args>(params)...);
    }

    /**
     * Imitate pointer functionality.
     */
    inline T* operator ->() const {return m_object;}

    /**
     * Import the object to database
     * @param iov interval of validity
     */
    inline bool import(IntervalOfValidity& iov)
    {
      if (!m_object) return false;
      return Database::Instance().storeData(m_package, m_module, m_object, iov);
    }


  private:

    /**
     * Hidden copy constructor.
     * To prevent making copies, since the class contains pointer to allocated memory.
     */
    DBImportObjPtr(const DBImportObjPtr&);

    std::string m_module;      /**< object name in database */
    std::string m_package;     /**< package name */
    T* m_object = 0;           /**< pointer to allocated object */

  };
}
