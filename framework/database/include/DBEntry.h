/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Marko Staric                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/database/IntervalOfValidity.h>
#include <string>

class TClass;


namespace Belle2 {
  /**
   * Caches an object or array that is read from the database.
   * See DBStore::m_dbEntryMap.
   */
  struct DBEntry {
    DBEntry() : name(""), objClass(0), isArray(false), object(0) {};

    std::string name; /**< Name of the entry in the database. Equal to the key in the map. **/
    const TClass* objClass;   /**< type of the object **/
    bool isArray;   /**< flag indicating that the object is a TClonesArray **/
    TObject* object;   /**< pointer to the currently valid object that was returned from the database. **/
    IntervalOfValidity iov;   /**< the interval of validity of the object. **/
  };
}
