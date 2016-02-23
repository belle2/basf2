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
   * Base class for importing objects to the database.
   * Note that the object is NOT parked at DBStore, but allocated internally.
   */
  class DBImportBase {
  public:

    /**
     * Constructor
     * @param module  Name under which the object will be stored in the database
     * @param package Package name
     */
    DBImportBase(const std::string& module, const std::string& package):
      m_module(module),
      m_package(package)
    {}

    /**
     * Destructor
     */
    ~DBImportBase()
    {}


    /**
     * Import the object to database
     * @param iov interval of validity
     */
    virtual bool import(IntervalOfValidity& iov)
    {
      if (!m_object) return false;
      return Database::Instance().storeData(m_package, m_module, m_object, iov);
    }


  protected:

    std::string m_module;      /**< object or array name in database */
    std::string m_package;     /**< package name */
    TObject* m_object = 0;     /**< pointer to allocated object or array */

  private:

    /**
     * Hidden copy constructor.
     * To prevent making copies, since the class contains pointer to allocated memory.
     */
    DBImportBase(const DBImportBase&);

  };
}
