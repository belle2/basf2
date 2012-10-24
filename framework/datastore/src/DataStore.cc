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

#include <framework/logging/Logger.h>
#include <framework/dataobjects/RelationContainer.h>

#include <TClonesArray.h>
#include <TClass.h>

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

  //add to current module's outputs
  ModuleInfo& info = m_moduleInfo[m_currentModule];
  if (objClass == RelationContainer::Class())
    info.outputRelations.insert(name);
  else
    info.outputs.insert(name);

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
  // check whether the cached information is (still) valid
  if (entry && entry->ptr && (index >= 0)) {
    if (static_cast<TClonesArray*>(entry->ptr)->At(index) == object) return true;
    index = static_cast<TClonesArray*>(entry->ptr)->IndexOf(object);
    if (index >= 0) return true;
  }
  entry = 0;
  index = -1;

  // search for the object and set the entry and index
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

bool DataStore::addRelation(const TObject* fromObject, DataStore::StoreEntry*& fromEntry, int& fromIndex, const TObject* toObject, double weight)
{
  // get entry from which the relation points
  if (!findStoreEntry(fromObject, fromEntry, fromIndex)) return false;

  // get entry to which the relation points
  StoreEntry* toEntry = 0;
  int toIndex = -1;
  if (!findStoreEntry(toObject, toEntry, toIndex)) return false;

  // get the relations from -> to
  string relationsName = relationName(fromEntry->name, toEntry->name);
  if (m_storeObjMap[c_Event].find(relationsName) == m_storeObjMap[c_Event].end()) return false;
  StoreEntry* entry = m_storeObjMap[c_Event][relationsName];

  // auto create relations if needed
  if (!entry->ptr) {
    delete entry->object;
    RelationContainer* relations = new RelationContainer;
    relations->setFromName(fromEntry->name);
    relations->setFromDurability(c_Event);
    relations->setToName(toEntry->name);
    relations->setToDurability(c_Event);
    entry->object = relations;
    entry->ptr = entry->object;
  }

  // add relation
  TClonesArray& relations = static_cast<RelationContainer*>(entry->ptr)->elements();
  new(relations.AddrAt(relations.GetLast() + 1)) RelationElement(fromIndex, toIndex, weight);
  return true;
}

std::vector<RelationEntry> DataStore::getRelationsFromTo(const TObject* fromObject, DataStore::StoreEntry*& fromEntry, int& fromIndex, TClass* toClass, std::string name)
{
  std::vector<RelationEntry> result;

  // get entry from which the relations point
  if (!findStoreEntry(fromObject, fromEntry, fromIndex)) return result;

  // get names of store arrys to which the relations point
  std::vector<string> toNames;
  if (name == "ALL") {
    for (StoreObjIter iter = m_storeObjMap[c_Event].begin(); iter != m_storeObjMap[c_Event].end(); ++iter) {
      if (iter->second->ptr && iter->second->isArray) {
        TClonesArray* array = static_cast<TClonesArray*>(iter->second->ptr);
        if (array->GetClass() == toClass) {
          toNames.push_back(iter->second->name);
        }
      }
    }
  } else if (name.empty()) {
    toNames.push_back(defaultArrayName(toClass->GetName()));
  } else {
    toNames.push_back(name);
  }

  // loop over to store arrays
  for (std::vector<string>::iterator toName = toNames.begin(); toName != toNames.end(); ++toName) {
    if (m_storeObjMap[c_Event].find(*toName) == m_storeObjMap[c_Event].end()) continue;
    TClonesArray* toArray = static_cast<TClonesArray*>(m_storeObjMap[c_Event][*toName]->ptr);
    if (!toArray) continue;

    // get the relations from -> to
    string relationsName = relationName(fromEntry->name, *toName);
    if (m_storeObjMap[c_Event].find(relationsName) == m_storeObjMap[c_Event].end()) continue;
    TObject* entry = m_storeObjMap[c_Event][relationsName]->ptr;
    if (!entry) continue;

    // loop over relations and collect those pointing from the fromObject
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
  }

  return result;
}

std::vector<RelationEntry> DataStore::getRelationsToFrom(const TObject* toObject, DataStore::StoreEntry*& toEntry, int& toIndex, TClass* fromClass, std::string name)
{
  std::vector<RelationEntry> result;

  // get entry to which the relations point
  if (!findStoreEntry(toObject, toEntry, toIndex)) return result;

  // get names of store arrys from which the relations point
  std::vector<string> fromNames;
  if (name == "ALL") {
    for (StoreObjIter iter = m_storeObjMap[c_Event].begin(); iter != m_storeObjMap[c_Event].end(); ++iter) {
      if (iter->second->ptr && iter->second->isArray) {
        TClonesArray* array = static_cast<TClonesArray*>(iter->second->ptr);
        if (array->GetClass() == fromClass) {
          fromNames.push_back(iter->second->name);
        }
      }
    }
  } else if (name.empty()) {
    fromNames.push_back(defaultArrayName(fromClass->GetName()));
  } else {
    fromNames.push_back(name);
  }

  // loop over from store arrays
  for (std::vector<string>::iterator fromName = fromNames.begin(); fromName != fromNames.end(); ++fromName) {
    if (m_storeObjMap[c_Event].find(*fromName) == m_storeObjMap[c_Event].end()) continue;
    TClonesArray* fromArray = static_cast<TClonesArray*>(m_storeObjMap[c_Event][*fromName]->ptr);
    if (!fromArray) continue;

    // get the relations from -> to
    string relationsName = relationName(*fromName, toEntry->name);
    if (m_storeObjMap[c_Event].find(relationsName) == m_storeObjMap[c_Event].end()) continue;
    TObject* entry = m_storeObjMap[c_Event][relationsName]->ptr;
    if (!entry) return result;

    // loop over relations and collect those pointing to the toObject
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
  }

  return result;
}

std::vector<RelationEntry> DataStore::getRelationsWith(const TObject* object, DataStore::StoreEntry*& entry, int& index, TClass* withClass, std::string name)
{
  std::vector<RelationEntry> result = getRelationsFromTo(object, entry, index, withClass, name);

  std::vector<RelationEntry> fromResult = getRelationsToFrom(object, entry, index, withClass, name);
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

bool DataStore::require(const std::string& name, EDurability durability,
                        const TClass* objClass, bool array)
{
  if (m_initializeActive) {
    ModuleInfo& info = m_moduleInfo[m_currentModule];
    if (objClass == RelationContainer::Class())
      info.inputRelations.insert(name);
    else
      info.inputs.insert(name);
  }

  if (!hasEntry(name, durability, objClass, array)) {
    B2ERROR("The required DataStore entry with name " << name << " and durability " << durability << " does not exist. Maybe you forgot the module that creates it?");
    return false;
  }
  return true;
}
