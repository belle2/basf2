/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef STOREITER_H
#define STOREITER_H

#include <TObject.h>

#include <string>

namespace Belle2 {

  /*! Iterator for DataStore maps. */
  /*! This is the base class for StoreMapIter objects, that is templatized.
      A common base class unifies the handling.
      \author <a href="mailto:martin.heck@kit.edu?subject=">Martin Heck</a>
  */
  class StoreIter {
  public:
    /*! Constructor.*/
    /*! Usually you don't need this, as this class is only a base class to unify the different StoreMapIter classes
        in a common handling.
    */
    StoreIter() {}

    /*! Destructor.*/
    /*! The destructor is virtual, because this is meant as base class.*/
    virtual ~StoreIter() {}

    /*!Set Iterator to first element.*/
    /*!*/
    virtual void first() = 0;

    /*!Set Iterator to next element.*/
    /*!*/
    virtual void next() = 0;

    /*!Checks, if you have moved to the last element.*/
    /*!*/
    virtual bool isDone() const = 0;

    /*!Returns the object, the Iterator currently points to.*/
    /*!*/
    virtual TObject* value() = 0;

    /*!*/
    /*!*/
    virtual std::string key() = 0;

  };
}

#endif
