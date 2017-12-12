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
  for (auto& entry : m_dbEntries) {
    DBEntry& dbEntry = entry.second;
    if (dbEntry.intraRunDependency) {
      delete dbEntry.intraRunDependency;
      dbEntry.intraRunDependency = nullptr;
    } else {
      delete dbEntry.object;
    }
    dbEntry.object = nullptr;
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
    B2FATAL("Existing entry '" << dbEntry.name << "' is an " << ((dbEntry.isArray) ? "array" : "object") <<
            " and the requested one an " << ((array) ? "array" : "object"));
  }

  // Check whether the existing entry and the requested object have the same type
  if (!dbEntry.objClass->InheritsFrom(objClass)) {
    B2FATAL("Existing entry '" << dbEntry.name << "' of type " <<
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
    dbEntry.intraRunDependency = nullptr;
  } else {
    delete dbEntry.object;
  }
  dbEntry.object = nullptr;
  if (objectIov.first && objectIov.first->InheritsFrom(IntraRunDependency::Class())) {
    dbEntry.intraRunDependency = static_cast<IntraRunDependency*>(objectIov.first);
    dbEntry.object = dbEntry.intraRunDependency->getObject(*m_event);
    m_intraRunDependencies.insert(&dbEntry);
  } else {
    dbEntry.object = objectIov.first;
  }
  dbEntry.iov = objectIov.second;
}


DBEntry* DBStore::getEntry(const std::string& name, const TClass* objClass, bool array)
{
  // Check whether the map entry already exists
  const auto& entry = m_dbEntries.find(name);
  if (entry != m_dbEntries.end()) {
    DBEntry& dbEntry = entry->second;

    // Check whether the types match
    if (!checkType(dbEntry, objClass, array)) return nullptr;

    B2DEBUG(100, "A DBEntry " << name << " was requested once more.");
    return &dbEntry;
  }

  // Add the DBStore entry
  DBEntry& dbEntry = m_dbEntries[name];
  dbEntry.name = name;
  dbEntry.objClass = objClass;
  dbEntry.isArray = array;
  B2DEBUG(100, "A DBEntry " << name << " was created.");

  // If no object was obtained from the database yet, but it should be available, get it
  if (m_event.isValid()) {
    auto objectIov = Database::Instance().getData(*m_event, name);
    if (!objectIov.first) return &dbEntry;
    updateEntry(dbEntry, objectIov);

    // Check whether the types match
    if (!checkType(dbEntry, objectIov.first)) return nullptr;
  }

  return &dbEntry;
}


void DBStore::update()
{
  // Get list of objects that are outside their interval of validity
  list<Database::DBQuery> invalidEntries;
  for (auto& entry : m_dbEntries) {
    if (!entry.second.iov.contains(*m_event)) {
      //it's no longer valid, request update and remove from intrarun
      //dependency handling
      invalidEntries.push_back(Database::DBQuery(entry.first));
      m_intraRunDependencies.erase(&entry.second);
    }
  }

  // Request new objects and IoVs from database
  Database::Instance().getData(*m_event, invalidEntries);

  // Update DBStore entries
  DBCallbackMap callbacks;
  for (auto& query : invalidEntries) {
    auto& dbEntry = m_dbEntries[query.name];
    if (!dbEntry.object && query.object) {
      if (!checkType(dbEntry, query.object)) continue;
    }
    TObject* old = dbEntry.object;
    updateEntry(dbEntry, make_pair(query.object, query.iov));
    if ((old != nullptr) || (dbEntry.object != nullptr)) {
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

  for (auto& entry : m_dbEntries) {
    DBEntry& dbEntry = entry.second;
    if (dbEntry.intraRunDependency) {
      delete dbEntry.intraRunDependency;
      dbEntry.intraRunDependency = nullptr;
    } else {
      delete dbEntry.object;
    }
    dbEntry.object = nullptr;
    dbEntry.iov = IntervalOfValidity();
  }

  StoreObjPtr<EventMetaData> event;
  m_event = event;
}

void DBStore::addConstantOverride(const std::string& name, TObject* obj, bool oneRun)
{
  // Add the DBStore entry
  DBEntry& dbEntry = m_dbEntries[name];
  dbEntry.name = name;
  // delete the object if it is not identical to the new one
  if (!dbEntry.intraRunDependency && dbEntry.object != obj) delete dbEntry.object;
  dbEntry.object = obj;
  dbEntry.isArray = dynamic_cast<TClonesArray*>(obj) != nullptr;
  if (dbEntry.isArray) {
    dbEntry.objClass = dynamic_cast<TClonesArray*>(obj)->GetClass();
  } else {
    dbEntry.objClass = obj->IsA();
  }
  dbEntry.iov = IntervalOfValidity::always();
  if (oneRun) {
    const int exp = m_event->getExperiment();
    const int run = m_event->getRun();
    dbEntry.iov = IntervalOfValidity(exp, run, exp, run);
  }
  // we need to remove this entry from the intraRunDependencies list now.
  // Otherwise it will reset the object on the next event call
  delete dbEntry.intraRunDependency;
  dbEntry.intraRunDependency = nullptr;
  m_intraRunDependencies.erase(&dbEntry);
  B2WARNING("An override for DBEntry " << name << " was created.");
  // run callbacks
  for (auto& callback : dbEntry.callbackFunctions) {
    callback.second();
  }
}

void DBStore::addCallback(const std::string& name, DBCallback callback, DBCallbackId id)
{
  const auto& entry = m_dbEntries.find(name);
  if (entry != m_dbEntries.end()) {
    DBEntry& dbEntry = entry->second;
    dbEntry.callbackFunctions[id] = callback;
    return;
  }
  B2ERROR("Cannot add callback to unkown DB entry " << name << ".");
}
