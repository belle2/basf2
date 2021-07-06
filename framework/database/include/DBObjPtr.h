/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
     * @param name Name under which the object is stored in the database (and DBStore).
     * @param required If false don't emit an error if the payload cannot be found.
     *     If the same payload is requested in different places then all occurences need to specify false
     *     here to suppress any payload error messages.
     */
    explicit DBObjPtr(const std::string& name = "", bool required = true):
      DBAccessorBase(DBStore::objectName<T>(name), T::Class(), false, required) {}

    inline const T& operator *()  const {return *getObject<T>(); }  /**< Imitate pointer functionality. */
    inline const T* operator ->() const {return getObject<T>(); }   /**< Imitate pointer functionality. */
  };

  /** Optional DBObjPtr: This class behaves the same as the DBObjPtr except
   * that it will not raise errors when an object could not be found in the
   * database.
   *
   * Users are responsible to check the validity of the object before using it.
   *
   * This is the same as using a DBObjPtr with full constructor arguments to
   * specify whether it is optional but allows to use default constructor and
   * gives a clearer intention to readers.
   */
  template<class T> class OptionalDBObjPtr: public DBObjPtr<T> {
  public:
    /** Construct a new Array with a given name or with the default name which
     * is identical to the class name of T */
    explicit OptionalDBObjPtr(const std::string& name = ""): DBObjPtr<T>(name, false) {}
  };
}
