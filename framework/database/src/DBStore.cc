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

namespace {


  /** Called by exit handlers to free all memory.
   *
   * This is important since ROOT exit handlers may remove some of our objects.
   * Without this function, we will free the memory when the DBStore instance
   * is destroyed, which happens too late.
   */
  void cleanDBStore()
  {
    delete &DBStore::Instance();
  }
}

bool DBStore::s_DoCleanup = false;

DBStore::~DBStore()
{
  if (s_DoCleanup) {
    //release all memory in db store
    for (auto& mapEntry : m_dbEntries) {
      delete mapEntry.second.object;
    }
  }
  m_dbEntries.clear();
}


DBStore& DBStore::Instance()
{
  static DBStore instance;
  return instance;
}


std::string DBStore::defaultObjectName(std::string classname)
{
  //Strip qualifiers like namespaces
  size_t colon = classname.rfind("::");
  if (colon != std::string::npos) {
    classname = classname.substr(colon + 2);
  }
  return classname;
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
  TClass* objClass = object->IsA();
  bool array = (objClass == TClonesArray::Class());
  if (array) {
    objClass = static_cast<const TClonesArray*>(object)->GetClass();
  }

  return checkType(dbEntry, objClass, array);
}


DBEntry* DBStore::getEntry(const std::string& name, const TClass* objClass, bool array)
{
  // Check whether the map entry already exists
  const auto& mapEntry = m_dbEntries.find(name);
  if (mapEntry != m_dbEntries.end()) {
    DBEntry& dbEntry = mapEntry->second;

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
    dbEntry.object = objectIov.first;
    dbEntry.iov = objectIov.second;

    // Check whether the types match
    if (!checkType(dbEntry, objectIov.first)) return nullptr;
  }

  return &dbEntry;
}


void DBStore::update()
{
  // Get list of objects that are outside their interval of validity
  list<Database::DBQuery> invalidEntries;
  for (auto& mapEntry : m_dbEntries) {
    if (!mapEntry.second.iov.contains(*m_event)) {
      invalidEntries.push_back(Database::DBQuery(mapEntry.first));
    }
  }

  // Request new objects and IoVs from database
  Database::Instance().getData(*m_event, invalidEntries);

  // Update DBStore entries
  for (auto& query : invalidEntries) {
    auto& dbEntry = m_dbEntries[query.name];
    if (!dbEntry.object && query.object) {
      if (!checkType(dbEntry, query.object)) continue;
    }
    dbEntry.object = query.object;
    dbEntry.iov = query.iov;
    // callbacks for updated objects could go here
  }
}
