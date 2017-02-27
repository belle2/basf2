/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Marko Staric                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/DBStore.h>

#include <framework/database/Database.h>
#include <framework/logging/Logger.h>

#include <TClonesArray.h>
#include <TClass.h>

#include <list>

using namespace std;
using namespace Belle2;

DBStore::~DBStore()
{
  //release all memory in db store
  for (auto& packageEntry : m_dbEntries) {
    for (auto& moduleEntry : packageEntry.second) {
      delete moduleEntry.second.object;
    }
    packageEntry.second.clear();
  }
  m_dbEntries.clear();
}


DBStore& DBStore::Instance()
{
  static DBStore instance;
  return instance;
}


bool DBStore::checkType(const DBEntry& dbEntry, const TClass* objClass, bool array) const
{
  // Check whether the existing entry and the requested object are both arrays or both single objects
  if (dbEntry.isArray != array) {
    B2FATAL("Existing entry '" << dbEntry.package << "/" << dbEntry.module << "' is an " << ((dbEntry.isArray) ? "array" : "object") <<
            " and the requested one an " << ((array) ? "array" : "object"));
  }

  // Check whether the existing entry and the requested object have the same type
  if (!dbEntry.objClass->InheritsFrom(objClass)) {
    B2FATAL("Existing entry '" << dbEntry.package << "/" << dbEntry.module << "' of type " <<
            dbEntry.objClass->GetName() << " doesn't match requested type " <<
            objClass->GetName());
  }

  return true;
}

bool DBStore::checkType(const DBEntry& dbEntry, const TObject* object) const
{
  // Get class information from object
  if (object->InheritsFrom(IntraRunDependency::Class())) {
    object = static_cast<const IntraRunDependency*>(object)->getAnyObject();
  }
  TClass* objClass = object->IsA();
  bool array = (objClass == TClonesArray::Class());
  if (array) {
    objClass = static_cast<const TClonesArray*>(object)->GetClass();
  }

  return checkType(dbEntry, objClass, array);
}


void DBStore::updateEntry(DBEntry& dbEntry, const std::pair<TObject*, IntervalOfValidity>& objectIov)
{
  if (dbEntry.intraRunDependency) {
    delete dbEntry.intraRunDependency;
    dbEntry.intraRunDependency = 0;
  } else {
    delete dbEntry.object;
  }
  dbEntry.object = 0;
  if (objectIov.first && objectIov.first->InheritsFrom(IntraRunDependency::Class())) {
    dbEntry.intraRunDependency = static_cast<IntraRunDependency*>(objectIov.first);
    dbEntry.object = dbEntry.intraRunDependency->getObject(*m_event);
    m_intraRunDependencies.push_back(&dbEntry);
  } else {
    dbEntry.object = objectIov.first;
  }
  dbEntry.iov = objectIov.second;
}


DBEntry* DBStore::getEntry(const std::string& package, const std::string& module, const TClass* objClass, bool array)
{
  // Check whether the map entry already exists
  const auto& packageEntry = m_dbEntries.find(package);
  if (packageEntry != m_dbEntries.end()) {
    const auto& moduleEntry = packageEntry->second.find(module);
    if (moduleEntry != packageEntry->second.end()) {
      DBEntry& dbEntry = moduleEntry->second;

      // Check whether the types match
      if (!checkType(dbEntry, objClass, array)) return nullptr;

      B2DEBUG(100, "A DBEntry " << package << "/" << module << " was requested once more.");
      return &dbEntry;
    }
  }

  // Add the DBStore entry
  DBEntry& dbEntry = m_dbEntries[package][module];
  dbEntry.package = package;
  dbEntry.module = module;
  dbEntry.objClass = objClass;
  dbEntry.isArray = array;
  B2DEBUG(100, "A DBEntry " << package << "/" << module << " was created.");

  // If no object was obtained from the database yet, but it should be available, get it
  if (m_event.isValid()) {
    auto objectIov = Database::Instance().getData(*m_event, package, module);
    if (!objectIov.first) return &dbEntry;
    updateEntry(dbEntry, objectIov);

    // Check whether the types match
    if (!checkType(dbEntry, objectIov.first)) return nullptr;
  }

  return &dbEntry;
}


void DBStore::update()
{
  m_intraRunDependencies.clear();

  // Get list of objects that are outside their interval of validity
  list<Database::DBQuery> invalidEntries;
  for (auto& packageEntry : m_dbEntries) {
    for (auto& moduleEntry : packageEntry.second) {
      if (!moduleEntry.second.iov.contains(*m_event)) {
        invalidEntries.push_back(Database::DBQuery(packageEntry.first, moduleEntry.first));
      }
    }
  }

  // Request new objects and IoVs from database
  Database::Instance().getData(*m_event, invalidEntries);

  // Update DBStore entries
  DBCallbackMap callbacks;
  for (auto& query : invalidEntries) {
    auto& dbEntry = m_dbEntries[query.package][query.module];
    if (!dbEntry.object && query.object) {
      if (!checkType(dbEntry, query.object)) continue;
    }
    TObject* old = dbEntry.object;
    updateEntry(dbEntry, make_pair(query.object, query.iov));
    if ((old != 0) || (dbEntry.object != 0)) {
      for (auto& callback : dbEntry.callbackFunctions) {
        callbacks[callback.first] = callback.second;
      }
    }
  }

  // Callbacks
  for (auto& callback : callbacks) callback.second();
}


void DBStore::updateEvent()
{
  // loop over intra-run dependent conditions and update the objects if needed
  DBCallbackMap callbacks;
  for (auto& dbEntry : m_intraRunDependencies) {
    TObject* old = dbEntry->object;
    dbEntry->object = dbEntry->intraRunDependency->getObject(*m_event);
    if (dbEntry->object != old) {
      for (auto& callback : dbEntry->callbackFunctions) {
        callbacks[callback.first] = callback.second;
      }
    }
  }

  // Callbacks
  for (auto& callback : callbacks) callback.second();
}


void DBStore::reset()
{
  m_intraRunDependencies.clear();

  for (auto& packageEntry : m_dbEntries) {
    for (auto& moduleEntry : packageEntry.second) {
      DBEntry& dbEntry = moduleEntry.second;
      //delete dbEntry.object;
      dbEntry.object = 0;
      dbEntry.intraRunDependency = 0;
      dbEntry.iov = IntervalOfValidity();
    }
  }

  StoreObjPtr<EventMetaData> event;
  m_event = event;
}

void DBStore::addConstantOverride(const std::string& package, const std::string& module, TObject* obj, bool oneRun)
{
  // Add the DBStore entry
  DBEntry& dbEntry = m_dbEntries[package][module];
  dbEntry.package = package;
  dbEntry.module = module;
  // delete the object if it is not identical to the new one
  if (!dbEntry.intraRunDependency && dbEntry.object != obj) delete dbEntry.object;
  dbEntry.object = obj;
  dbEntry.isArray = dynamic_cast<TClonesArray*>(obj) != nullptr;
  if (dbEntry.isArray) {
    dbEntry.objClass = dynamic_cast<TClonesArray*>(obj)->GetClass();
  } else {
    dbEntry.objClass = obj->IsA();
  }
  dbEntry.iov = IntervalOfValidity(0, 0, -1, -1);
  if (oneRun) {
    const int exp = m_event->getExperiment();
    const int run = m_event->getRun();
    dbEntry.iov = IntervalOfValidity(exp, run, exp, run);
  }
  // we need to remove this entry from the intraRunDependencies list now.
  // Otherwise it will reset the object on the next event call
  delete dbEntry.intraRunDependency;
  dbEntry.intraRunDependency = nullptr;
  for (auto it = m_intraRunDependencies.begin(); it != m_intraRunDependencies.end(); ++it) {
    if (*it == &dbEntry) {
      m_intraRunDependencies.erase(it);
      break;
    }
  }
  B2WARNING("An override for DBEntry " << package << "/" << module << " was created.");
  // run callbacks
  for (auto& callback : dbEntry.callbackFunctions) {
    callback.second();
  }
}

void DBStore::addCallback(const std::string& package, const std::string& module, DBCallback callback, DBCallbackId id)
{
  const auto& packageEntry = m_dbEntries.find(package);
  if (packageEntry != m_dbEntries.end()) {
    const auto& moduleEntry = packageEntry->second.find(module);
    if (moduleEntry != packageEntry->second.end()) {
      DBEntry& dbEntry = moduleEntry->second;
      dbEntry.callbackFunctions[id] = callback;
      return;
    }
  }
  B2ERROR("Cannot add callback to unkown DB entry " << package << "/" << module << ".");
}
