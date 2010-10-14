/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef STOREMAPITER_H
#define STOREMAPITER_H

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreIter.h>

#include <TObject.h>

namespace Belle2 {

  /*! Iterator for DataStore maps. */
  /*! This iterator has to be specified with the exact map, that is used.
      Usually you shouldn't use this class, but the parent class, and only the DataStore has
      to create this class internally.
      \author <a href="mailto:martin.heck@kit.edu?subject=StoreMapIter">Martin Heck</a>
  */
  template <class T>
  class StoreMapIter : public StoreIter {
  public:

    StoreMapIter(T* AMap)
        : m_map(AMap), m_iter(0) {}

    ~StoreMapIter() {}


    void first() { m_iter = m_map->begin(); }


    void next() { m_iter++; }


    bool isDone() const {
      if (m_iter == m_map->end()) {
        return true;
      }
      return false;
    }


    TObject* value() {
      return (m_iter->second);
    }


    std::string key() {
      return m_iter->first;
    }


    typename T::iterator find(const std::string& name) {
      m_iter = m_map.find(name);
      return m_iter;
    }


    typename T::iterator operator[](std::string str) {
      return m_map[str];
    }

    /*! Prefix increment operator */
    StoreMapIter <T>& operator++() {
      next();
      return *this;
    }

    /*! Postfix increment operator */
    StoreMapIter<T> operator++(int) {
      StoreMapIter<T> buffer(*this);
      next();
      return buffer;
    }

    //private:

    T*     m_map;
    typename T::iterator  m_iter;

  };
}



#endif // STOREMAPITER
