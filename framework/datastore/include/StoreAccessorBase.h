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

  /** Base class for the StoreObjPtr and the StoreArray for easier common treatment.
   *
   *  @author <a href="mailto:belle2_software@bpost.kek.jp?subject=StoreAccessorBase">The basf2 developers</a>
   */
  class StoreAccessorBase {
  public:

    /** Constructor to access an array in the DataStore.
     *
     *  @param name       Name under which the object is stored in the DataStore.
     *  @param durability Decides durability map used for getting the accessed object.
     */
    StoreAccessorBase(const std::string& name, DataStore::EDurability durability):
      m_name(name), m_durability(durability) {};

    /** Destructor.
     *
     *  Virtual because this is a base class.
     */
    virtual ~StoreAccessorBase() {};


    /** Return name under which the object is saved in the DataStore. */
    const std::string& getName() const { return m_name; }

    /** Return durability with which the object is saved in the DataStore. */
    DataStore::EDurability getDurability() const { return m_durability; }

    /** Return pair of name and durability under which stored object is saved.  */
    AccessorParams getAccessorParams() const {return make_pair(m_name, m_durability);};

    /** Check if two store accessors point to the same object/array. */
    virtual bool operator==(const StoreAccessorBase& other) {
      return getAccessorParams() == other.getAccessorParams();
    }

    /** Check if two store accessors point to a different object/array. */
    virtual bool operator!=(const StoreAccessorBase& other) {
      return !(*this == other);
    }

  protected:
    /** Store name under which TClonesArray is saved. */
    std::string m_name;

    /**Store durability under which the TClonesArray is saved. */
    DataStore::EDurability m_durability;

  };
}

#endif // STOREACCESSORBASE_H
