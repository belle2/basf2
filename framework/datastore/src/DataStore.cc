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
#include <framework/dataobjects/RelationContainer.h>

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


bool DataStore::findStoreEntry(const TObject* object, DataStore::StoreEntry*& entry, int& index)
{
  if (!entry) {
    TClass* objectClass = object->IsA();
    for (StoreObjIter iter = m_storeObjMap[c_Event].begin(); iter != m_storeObjMap[c_Event].end(); ++iter) {
      if (iter->second->ptr && iter->second->isArray) {
        TClonesArray* array = static_cast<TClonesArray*>(iter->second->ptr);
        TClass* arrayClass = array->GetClass();
        if (arrayClass == objectClass) {
          index = array->IndexOf(object);
          if (index >= 0) {
            entry = iter->second;
            return true;
          }
        }
      }
    }
    return false;
  }

  if (!entry->ptr) return false;
  index = static_cast<TClonesArray*>(entry->ptr)->IndexOf(object);
  return (index >= 0);
}

bool DataStore::addRelation(const TObject* fromObject, DataStore::StoreEntry*& fromEntry, int& fromIndex, const TObject* toObject, double weight)
{
  if (fromIndex < 0) {
    if (!findStoreEntry(fromObject, fromEntry, fromIndex)) return false;
  }
  StoreEntry* toEntry = 0;
  int toIndex = -1;
  if (!findStoreEntry(toObject, toEntry, toIndex)) return false;

  string relationsName = relationName(fromEntry->name, toEntry->name);
  if (m_storeObjMap[c_Event].find(relationsName) == m_storeObjMap[c_Event].end()) return false;
  TObject* entry = m_storeObjMap[c_Event][relationsName]->ptr;
  if (!entry) return false;

  TClonesArray& relations = static_cast<RelationContainer*>(entry)->elements();
  new(relations.AddrAt(relations.GetLast() + 1)) RelationElement(fromIndex, toIndex, weight);
  return true;
}

std::vector<RelationEntry> DataStore::getRelationsTo(TObject* fromObject, DataStore::StoreEntry*& fromEntry, int& fromIndex, TClass* toClass, std::string name)
{
  std::vector<RelationEntry> result;

  if (fromIndex < 0) {
    if (!findStoreEntry(fromObject, fromEntry, fromIndex)) return result;
  }
  string toName = name;
  if (name.empty()) name = toClass->GetName();
  if (m_storeObjMap[c_Event].find(toName) == m_storeObjMap[c_Event].end()) return result;
  TClonesArray* toArray = static_cast<TClonesArray*>(m_storeObjMap[c_Event][toName]->ptr);
  if (!toArray) return result;

  string relationsName = relationName(fromEntry->name, toName);
  if (m_storeObjMap[c_Event].find(relationsName) == m_storeObjMap[c_Event].end()) return result;
  TObject* entry = m_storeObjMap[c_Event][relationsName]->ptr;
  if (!entry) return result;

  TClonesArray& relations = static_cast<RelationContainer*>(entry)->elements();
  for (int iRelation = 0; iRelation < relations.GetEntriesFast(); iRelation++) {
    RelationElement* element = static_cast<RelationElement*>(relations[iRelation]);
    if (element->getFromIndex() == (unsigned int)fromIndex) {
      for (unsigned int iToIndex = 0; iToIndex < element->getSize(); iToIndex++) {
        TObject* toObject = toArray->At(element->getToIndex(iToIndex));
        if (toObject) result.push_back(RelationEntry(toObject, element->getWeight(iToIndex)));
      }
    }
  }

  return result;
}

std::vector<RelationEntry> DataStore::getRelationsFrom(TObject* toObject, DataStore::StoreEntry*& toEntry, int& toIndex, TClass* fromClass, std::string name)
{
  std::vector<RelationEntry> result;

  if (toIndex < 0) {
    if (!findStoreEntry(toObject, toEntry, toIndex)) return result;
  }
  string fromName = name;
  if (name.empty()) name = fromClass->GetName();
  if (m_storeObjMap[c_Event].find(fromName) == m_storeObjMap[c_Event].end()) return result;
  TClonesArray* fromArray = static_cast<TClonesArray*>(m_storeObjMap[c_Event][fromName]->ptr);
  if (!fromArray) return result;

  string relationsName = relationName(fromName, toEntry->name);
  if (m_storeObjMap[c_Event].find(relationsName) == m_storeObjMap[c_Event].end()) return result;
  TObject* entry = m_storeObjMap[c_Event][relationsName]->ptr;
  if (!entry) return result;

  TClonesArray& relations = static_cast<RelationContainer*>(entry)->elements();
  for (int iRelation = 0; iRelation < relations.GetEntriesFast(); iRelation++) {
    RelationElement* element = static_cast<RelationElement*>(relations[iRelation]);
    for (unsigned int iToIndex = 0; iToIndex < element->getSize(); iToIndex++) {
      if (element->getToIndex(iToIndex) == (unsigned int)toIndex) {
        TObject* fromObject = fromArray->At(element->getFromIndex());
        if (fromObject) result.push_back(RelationEntry(fromObject, element->getWeight(iToIndex)));
      }
    }
  }

  return result;
}

std::vector<RelationEntry> DataStore::getRelationsWith(TObject* object, DataStore::StoreEntry*& entry, int& index, TClass* withClass, std::string name)
{
  std::vector<RelationEntry> result = getRelationsTo(object, entry, index, withClass, name);

  std::vector<RelationEntry> fromResult = getRelationsFrom(object, entry, index, withClass, name);
  for (unsigned int i = 0; i < fromResult.size(); i++) {
    fromResult[i].weight = -fromResult[i].weight;
  }

  result.insert(result.end(), fromResult.begin(), fromResult.end());
  return result;
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
