/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/DBArray.h>

namespace Belle2 {

  /** Use default name for array in DBPointer. */
  std::string DBPointer_defaultName = "";

  /**
   * Class for pointing to an element in an array stored in the database.
   */
  template<class T, typename KEY, KEY(T::*METHOD)() const, const std::string& NAME = DBPointer_defaultName> class DBPointer {
  public:

    /**
     * Constructor of pointer to an array element in the database.
     * @param key       The key value to identify the element.
     */
    explicit DBPointer(KEY key):
      m_key(key), m_object(0) {};

    /** Accessor for key value */
    inline KEY key() const {return m_key;}

    /** Accessor for key value */
    inline operator KEY() const {return m_key;}

    /** Setter for key value */
    inline KEY operator = (KEY key) {m_key = key; m_object = 0; return m_key;}

    /**
     * Check whether we point to a valid object.
     * @return          True if the object exists.
     **/
    inline bool isValid() const {return getPointer();}

    /** Is this pointer's data safe to access? */
    inline operator bool() const {return isValid();}

    inline const T& operator *()  const {return *getPointer();}  /**< Imitate pointer functionality. */
    inline const T* operator ->() const {return getPointer();}   /**< Imitate pointer functionality. */

  private:
    /** Find the referred object and set m_object to it */
    inline const T* getPointer() const
    {
      if (!m_object) {
        DBArray<T> array(NAME);
        m_object = array.getByKey(METHOD, m_key);
      }
      return m_object;
    }

    /** Key value of the referred array element. */
    KEY m_key;

    /** Pointer to actual object. */
    mutable const T* m_object;   //!
  };
}
