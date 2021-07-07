/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/database/DBImportBase.h>
#include <stdexcept>

namespace Belle2 {

  /**
   * Class for importing a single object to the database.
   * Note that the object is NOT parked at DBStore, but allocated internally.
   */
  template<class T> class DBImportObjPtr: public DBImportBase {
  public:

    /**
     * Constructor:
     * the object itself is not allocated here, but in construct(...) function.
     * @param name  Name under which the object will be stored in the database
     */
    explicit DBImportObjPtr(const std::string& name = ""):
      DBImportBase(DBStore::objectName<T>(name))
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
    inline T* operator ->() const
    {
      if (!m_object)
        throw std::out_of_range("DBImportObjPtr::operator ->, for "
                                + getName() + ", "
                                "object does not exist or is invisible");
      return static_cast<T*>(m_object);
    }

  };
}
