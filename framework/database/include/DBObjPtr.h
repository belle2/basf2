/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
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
    explicit DBObjPtr(const std::string& name = "", bool required = true, const EventMetaData* event = nullptr):
      DBAccessorBase(DBStore::objectName<T>(name), T::Class(), false, required, event) {}

#ifndef DISABLE_CONST_DBOBJECTS
    inline const T& operator *()  const {return *getObject<T>(); }  /**< Imitate pointer functionality. */
    inline const T* operator ->() const {return getObject<T>(); }   /**< Imitate pointer functionality. */
#else
    [[deprecated("Non-Const DBObjPtr access is deprecated. Please remove `env.Append(CPPDEFINES=['DISABLE_CONST_DBOBJECTS']) "
                 "and fix any compilation problems you might have")]]
    inline T& operator *()  const {return *const_cast<T*>(getObject<T>()); }  /**< Imitate pointer functionality. */
    [[deprecated("Non-Const DBObjPtr access is deprecated. Please remove `env.Append(CPPDEFINES=['DISABLE_CONST_DBOBJECTS']) "
                 "and fix any compilation problems you might have")]]
    inline T* operator ->() const {return const_cast<T*>(getObject<T>()); }   /**< Imitate pointer functionality. */
#endif
  };
}
