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
#include <map>
#include <functional>
#include <utility>

class TClass;


namespace Belle2 {

  typedef std::function<void()> DBCallback;  /**< Type for database callback functions **/
  typedef std::pair<void*, void*> DBCallbackId;  /**< Type for unique identifiers of database callback functions **/
  typedef std::map<DBCallbackId, DBCallback> DBCallbackMap;  /**< Type for maps of identifiers to database callback functions **/

  /**
   * Caches an object or array that is read from the database.
   * See DBStore::m_dbEntryMap.
   */
  struct DBEntry {
    /** Name of the entry in the database. Equal to the key in the map */
    std::string name{""};
    /** type of the object */
    const TClass* objClass {nullptr};
    /** flag indicating that the object is a TClonesArray */
    bool isArray{false};
    /** pointer to the currently valid object that was returned from the database. */
    TObject* object{nullptr};
    /** the interval of validity of the object. */
    IntervalOfValidity iov;
    /** conditions data for cases where it changes during a run. */
    IntraRunDependency* intraRunDependency{nullptr};
    /** map of identifiers to callback functions. */
    DBCallbackMap callbackFunctions;
  };
}
