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

    /*! Constructor.
        \par Map to which this is an iterator.
    */
    StoreMapIter(T* AMap)
        : m_map(AMap), m_iter(0) {}

    /* Destructor.

    */
    ~StoreMapIter() {}


    /*! Point Iterator to first element.

    */
    void first() { m_iter = m_map->begin(); }


    /*! Point Iterator to next element.

    */
    void next() { m_iter++; }


    /*! Check if iterator is through.

    */
    bool isDone() const {
      if (m_iter == m_map->end()) {
        return true;
      }
      return false;
    }


    /*! Getter for the object to which iterator points.
        \return Object pointed to.
    */
    TObject* value() {
      return (m_iter->second);
    }

    /*! Getter for the name under which the object is saved in the map.
        \return Name of object, the iterator points to.
    */
    std::string key() {
      return m_iter->first;
    }


    /*!Find key in map.
       \par name of the object to be pointed to.
       \return Iterator to object with key name.
    */
    typename T::iterator find(const std::string& name) {
      m_iter = m_map.find(name);
      return m_iter;
    }


    /*! Another form of find.
        This is a short form of find?!
        To be checked later again.
    */
    typename T::iterator operator[](std::string str) {
      return m_map[str];
    }

    /*! Prefix increment operator

     */
    StoreMapIter <T>& operator++() {
      next();
      return *this;
    }

    /*! Postfix increment operator

     */
    StoreMapIter<T> operator++(int) {
      StoreMapIter<T> buffer(*this);
      next();
      return buffer;
    }


  private:

    /*! Pointer to map for which the iterator is.

    */
    T* m_map;

    /*! Iterator for the map.

    */
    typename T::iterator  m_iter;

  };
}



#endif // STOREMAPITER
