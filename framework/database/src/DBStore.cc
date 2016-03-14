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
  if (objectIov.first && objectIov.first->InheritsFrom(IntraRunDependency::Class())) {
    dbEntry.object = 0;
    dbEntry.intraRunDependency = static_cast<IntraRunDependency*>(objectIov.first);
    m_intraRunDependencies.push_back(&dbEntry);
  } else {
    dbEntry.object = objectIov.first;
    dbEntry.intraRunDependency = 0;
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
  for (auto& query : invalidEntries) {
    auto& dbEntry = m_dbEntries[query.package][query.module];
    if (!dbEntry.object && query.object) {
      if (!checkType(dbEntry, query.object)) continue;
    }
    updateEntry(dbEntry, make_pair(query.object, query.iov));
    // callbacks for updated objects could go here
  }
}


void DBStore::updateEvent()
{
  // loop over intra-run dependent conditions and update the objects if needed
  for (auto& dbEntry : m_intraRunDependencies) {
    dbEntry->object = dbEntry->intraRunDependency->getObject(*m_event);
    // callbacks for updated objects could go here
  }
}


void DBStore::reset()
{
  m_intraRunDependencies.clear();

  for (auto& packageEntry : m_dbEntries) {
    for (auto& moduleEntry : packageEntry.second) {
      DBEntry& dbEntry = moduleEntry.second;
      dbEntry.object = 0;
      dbEntry.intraRunDependency = 0;
      dbEntry.iov = IntervalOfValidity();
    }
  }

  StoreObjPtr<EventMetaData> event;
  m_event = event;
}
