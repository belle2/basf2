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
#include <framework/database/IntraRunDependency.h>
#include <string>
#include <vector>
#include <functional>

class TClass;


namespace Belle2 {
  /**
   * Caches an object or array that is read from the database.
   * See DBStore::m_dbEntryMap.
   */
  struct DBEntry {
    DBEntry() : package(""), module(""), objClass(0), isArray(false), object(0), intraRunDependency(0) {};

    std::string package; /**< Package name of the entry in the database. Equal to the key in the first map. **/
    std::string module; /**< Module name of the entry in the database. Equal to the key in the second map. **/
    const TClass* objClass;   /**< type of the object **/
    bool isArray;   /**< flag indicating that the object is a TClonesArray **/
    TObject* object;   /**< pointer to the currently valid object that was returned from the database. **/
    IntervalOfValidity iov;   /**< the interval of validity of the object. **/
    IntraRunDependency* intraRunDependency;   /**< conditions data for cases where it changes during a run. **/
    std::vector<std::function<void()>> callbackFunctions;   /**< vector of callback functions. **/
  };
}
