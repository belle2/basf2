/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef STOREACCESSORBASE_H
#define STOREACCESSORBASE_H

#include <framework/datastore/DataStore.h>

#include <string>
#include <utility>

namespace Belle2 {

  typedef std::pair<std::string, DataStore::EDurability> AccessorParams; /**< Pair of parameters needed to find an object in the DataStore. */

  /** Abstract base class for the StoreObjPtr and the StoreArray for easier common treatment.
   *
   *  @author <a href="mailto:belle2_software@bpost.kek.jp?subject=StoreAccessorBase">The basf2 developers</a>
   */
  class StoreAccessorBase {
  public:

    /** Destructor.
     *
     *  Virtual because this is a base class.
     */
    virtual ~StoreAccessorBase() {};


    /** Returns name under which stored object is saved.  */
    virtual AccessorParams getAccessorParams() const = 0;

    /** Check if two store accessors point to the same object/array. */
    virtual bool operator==(const StoreAccessorBase& other) {
      return getAccessorParams() == other.getAccessorParams();
    }

    /** Check if two store accessors point to a different object/array. */
    virtual bool operator!=(const StoreAccessorBase& other) {
      return !(*this == other);
    }

  };
}

#endif // STOREACCESSORBASE_H
