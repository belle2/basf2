/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/DBAccessorBase.h>

namespace Belle2 {

  /**
   * Class for accessing objects in the database.
   */
  template<class T> class DBObjPtr: public DBAccessorBase {
  public:

    /**
     * Constructor to access an object in the DBStore.
     * @param name       Name under which the object is stored in the database (and DBStore).
     */
    explicit DBObjPtr(const std::string& name = ""):
      DBAccessorBase(DBStore::objectName<T>(name), T::Class(), false) {m_object = reinterpret_cast<T**>(&m_entry->object);};

    inline T& operator *()  const {return **m_object;}  /**< Imitate pointer functionality. */
    inline T* operator ->() const {return *m_object;}   /**< Imitate pointer functionality. */

  private:
    /** Pointer to pointer to the object in the DBStore. */
    T** m_object;

  };
}
