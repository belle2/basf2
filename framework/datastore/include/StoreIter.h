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
  /*! \author <a href="mailto:martin.heck@kit.edu?subject=">Martin Heck</a>
  */
  class StoreIter {
  public:
    StoreIter() {}

    virtual ~StoreIter() {}

    virtual void first() = 0;

    virtual void next() = 0;

    virtual bool isDone() const = 0;

    virtual TObject* value() = 0;

    virtual std::string key() = 0;

  };
}

#endif
