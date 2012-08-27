/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/datastore/DataStore.h>

using namespace std;
using namespace Belle2;


DataStore& DataStore::Instance()
{
  static DataStore instance;
  return instance;
}


DataStore::DataStore() : m_initializeActive(false)
{
}

DataStore::~DataStore()
{
  //release all memory in data store
  for (int i = 0; i < c_NDurabilityTypes; i++)
    reset((EDurability)i);
}


bool DataStore::checkType(const std::string& name, const StoreEntry* entry,
                          const TClass* objClass, bool array) const
{
  // Check whether the existing entry and the requested object are both arrays or both single objects
  const char* entryType = (entry->isArray) ? "array" : "object";
  if (entry->isArray != array) {
    B2FATAL("Existing entry '" << name << "' is an " << entryType << " and the requested one an " << ((array) ? "array" : "object"));
    return false;
  }

  // Check whether the existing entry has the same type
  TClass* entryClass = entry->object->IsA();
  if (entry->isArray) {
    entryClass = static_cast<TClonesArray*>(entry->object)->GetClass();
  }
  if (entryClass != objClass) {
    B2FATAL("Existing " << entryType << " '" << name << "' of type " << entryClass->GetName() << " doesn't match requested type " << objClass->GetName());
    return false;
  }

  return true;
}


bool DataStore::createEntry(const std::string& name, EDurability durability,
                            const TClass* objClass, bool array, bool transient, bool errorIfExisting)
{
  // Check whether this method is called in the initialization phase
  if (!m_initializeActive) {
    B2ERROR("Attempt to register object '" << name << "' outside the initialization phase.");
    return false;
  }

  // Check whether the map entry already exists
  if (m_storeObjMap[durability].find(name) != m_storeObjMap[durability].end()) {
    StoreEntry* entry = m_storeObjMap[durability][name];

    // Complain about existing entry
    if (errorIfExisting) {
      B2ERROR("The object '" << name << "' of type " << entry->object->ClassName() << " was already registered before.");
      return false;
    }

    // Check whether the types match
    if (!checkType(name, entry, objClass, array)) return false;

    // Check whether the persistency type matches
    if (entry->isTransient != transient) {
      B2WARNING("Existing object '" << name << "' has different persistency type than the requested one. Using persistent option.");
      entry->isTransient = false;
    }

    B2DEBUG(100, "An entry with name " << name << " and durability " << durability << " already exists.");
    return true;
  }

  // Add the DataStore entry
  StoreEntry* entry = new StoreEntry;
  m_storeObjMap[durability][name] = entry;
  entry->isArray = array;
  entry->isTransient = transient;
  if (array) {
    entry->object = new TClonesArray(objClass);
  } else {
    entry->object = static_cast<TObject*>(objClass->New());
  }
  entry->name = name;

  B2DEBUG(100, "Entry with name " << name << " and durability " << durability << " was registered.");
  return true;
}


bool DataStore::hasEntry(const std::string& name, EDurability durability,
                         const TClass* objClass, bool array)
{
  if (m_storeObjMap[durability].find(name) != m_storeObjMap[durability].end()) {
    return checkType(name, m_storeObjMap[durability][name], objClass, array);
  } else {
    return false;
  }
}


TObject** DataStore::getObject(const std::string& name, EDurability durability,
                               const TClass* objClass, bool array)
{
  if (!hasEntry(name, durability, objClass, array)) {
    return 0;
  }
  return &(m_storeObjMap[durability][name]->ptr);
}


bool DataStore::createObject(TObject* object, bool replace, const std::string& name, EDurability durability,
                             const TClass* objClass, bool array)
{
  if (!hasEntry(name, durability, objClass, array)) {
    B2ERROR("No entry with name " << name << " and durability " << durability << " exists in the DataStore.");
    return false;
  }

  StoreEntry* entry = m_storeObjMap[durability][name];
  if (entry->ptr && !replace) {
    B2ERROR("An object with name " << name << " and durability " << durability << " was already created in the DataStore.");
    return false;
  }

  if (object) {
    delete entry->object;
    entry->object = object;
  } else {
    if (array) {
      static_cast<TClonesArray*>(entry->object)->Delete();
    } else {
      delete entry->object;
      entry->object = static_cast<TObject*>(objClass->New());
//      entry->object->Clear();
    }
  }

  entry->ptr = entry->object;
  return true;
}


void DataStore::clearMaps(EDurability durability)
{
  B2DEBUG(100, "Start deletion process of durability " << durability);
  for (StoreObjIter iter = m_storeObjMap[durability].begin(); iter != m_storeObjMap[durability].end(); ++iter) {
    iter->second->ptr = 0;
  }
}

void DataStore::reset(EDurability durability)
{
  for (StoreObjIter iter = m_storeObjMap[durability].begin(); iter != m_storeObjMap[durability].end(); ++iter) {
    //delete stored object/array
    delete iter->second->object;
    //delete StoreEntry
    delete iter->second;
  }
  m_storeObjMap[durability].clear();
}

void DataStore::backwardCompatibleRegistration(const std::string& name, EDurability durability,
                                               const TClass* objClass, bool array)
{
  if (!hasEntry(name, durability, objClass, array)) {
    std::string type = (array ? "array" : "object");
    if (m_initializeActive) {
      B2WARNING("DATASTORE BACKWARD COMPATIBILITY ISSUE: Instead of creating an " << type << " '" << name << "' in your initialize() method, please use registerPersistent().");
    } else {
      B2ERROR("DATASTORE BACKWARD COMPATIBILITY ISSUE: You have to *register* the " << type << " '" << name << "' *in the initialize method* before you can use it! Unless you do this, your " << type  << " can not be saved by the output module!");
    }
    bool initializeActive = m_initializeActive;
    m_initializeActive = true;
    createEntry(name, durability, objClass, array, false, true);
    m_initializeActive = initializeActive;
  }
}

void DataStore::backwardCompatibleCreation(const std::string& name, EDurability durability,
                                           const TClass* objClass, bool array)
{
  TObject** ptr = getObject(name, durability, objClass, array);
  if (!ptr) return;
  if (!*ptr) {
    std::string type = (array ? "array" : "object");
    B2WARNING("DATASTORE BACKWARD COMPATIBILITY ISSUE: You have to *create* the " << type << " '" << name << "' before you can use it!");
    createObject(0, true, name, durability, objClass, array);
  }
}
