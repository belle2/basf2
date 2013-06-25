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
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreAccessorBase.h>

#include <TClonesArray.h>
#include <TClass.h>

#include <boost/foreach.hpp>

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


bool DataStore::checkType(const StoreEntry& entry, const StoreAccessorBase& accessor) const
{
  // Check whether the existing entry and the requested object are both arrays or both single objects
  const char* entryType = (entry.isArray) ? "array" : "object";
  if (entry.isArray != accessor.isArray()) {
    B2FATAL("Existing entry '" << entry.name << "' is an " << entryType << " and the requested one an " << ((accessor.isArray()) ? "array" : "object"));
    return false;
  }

  // Check whether the existing entry has the same type
  TClass* entryClass = entry.object->IsA();
  if (entry.isArray) {
    entryClass = static_cast<TClonesArray*>(entry.object)->GetClass();
  }
  if (!entryClass->InheritsFrom(accessor.getClass())) {
    B2FATAL("Existing " << entryType << " '" << entry.name << "' of type " << entryClass->GetName() << " doesn't match requested type " << accessor.getClass()->GetName());
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
  info.addEntry(name, ModuleInfo::c_Output, (objClass == RelationContainer::Class()));

  // Check whether the map entry already exists
  const StoreObjConstIter& it = m_storeObjMap[durability].find(name);
  if (it != m_storeObjMap[durability].end()) {
    StoreEntry* entry = it->second;

    // Complain about existing entry
    if (errorIfExisting) {
      B2ERROR("The object '" << name << "' of type " << entry->object->ClassName() << " was already registered before.");
      return false;
    }

    // Check whether the types match
    if (!checkType(*entry, StoreAccessorBase(name, durability, objClass, array))) return false;

    // Check whether the persistency type matches
    if (entry->isTransient != transient) {
      B2WARNING("Existing object '" << name << "' has different persistency type than the requested one. Using persistent option.");
      entry->isTransient = false;
    }

    B2DEBUG(100, "An entry with name " << name << " and durability " << durability << " already exists.");
    return true;
  }

  // check reserved names
  if (array and name == "ALL") {
    B2ERROR("Creating an array with the reserved name 'ALL' is not allowed!");
    return false;
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


bool DataStore::hasEntry(const StoreAccessorBase& accessor)
{
  const StoreObjConstIter& it = m_storeObjMap[accessor.getDurability()].find(accessor.getName());

  if (it != m_storeObjMap[accessor.getDurability()].end()) {
    return checkType(*(it->second), accessor);
  } else {
    return false;
  }
}


TObject** DataStore::getObject(const StoreAccessorBase& accessor)
{
  if (!hasEntry(accessor)) {
    return 0;
  }
  return &(m_storeObjMap[accessor.getDurability()][accessor.getName()]->ptr);
}


bool DataStore::createObject(TObject* object, bool replace, const StoreAccessorBase& accessor)
{
  if (!hasEntry(accessor)) {
    B2ERROR("No entry with name " << accessor.getName() << " and durability " << accessor.getDurability() << " exists in the DataStore.");
    return false;
  }

  StoreEntry* entry = m_storeObjMap[accessor.getDurability()][accessor.getName()];
  if (entry->ptr && !replace) {
    B2ERROR("An object with name " << accessor.getName() << " and durability " << accessor.getDurability() << " was already created in the DataStore.");
    return false;
  }

  if (object) {
    delete entry->object;
    entry->object = object;
  } else {
    if (accessor.isArray()) {
      static_cast<TClonesArray*>(entry->object)->Delete();
    } else {
      delete entry->object;
      entry->object = static_cast<TObject*>(accessor.getClass()->New());
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

  //searching for NULL should be safe
  if (!object)
    return false;

  // search for the object and set the entry and index
  const TClass* objectClass = object->IsA();
  for (StoreObjIter iter = m_storeObjMap[c_Event].begin(); iter != m_storeObjMap[c_Event].end(); ++iter) {
    if (iter->second->ptr && iter->second->isArray) {
      TClonesArray* array = static_cast<TClonesArray*>(iter->second->ptr);
      const TClass* arrayClass = array->GetClass();
      if (arrayClass == objectClass) {
        if (object == array->Last()) {
          //quickly find entry if it's at the end of the array
          index = array->GetLast();
        } else {
          index = array->IndexOf(object);
        }

        if (index >= 0) {
          entry = iter->second;
          return true;
        }
      }
    }
  }
  return false;
}

void DataStore::getArrayNames(std::vector<std::string>& names, const std::string& arrayName, const TClass* arrayClass)
{
  if (arrayName == "ALL") {
    for (StoreObjIter iter = m_storeObjMap[c_Event].begin(); iter != m_storeObjMap[c_Event].end(); ++iter) {
      if (iter->second->ptr && iter->second->isArray) {
        TClonesArray* array = static_cast<TClonesArray*>(iter->second->ptr);
        if (array->GetClass()->InheritsFrom(arrayClass)) {
          names.push_back(iter->second->name);
        }
      }
    }
  } else if (arrayName.empty()) {
    names.push_back(defaultArrayName(arrayClass->GetName()));
  } else {
    names.push_back(arrayName);
  }
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
  const string& relationsName = relationName(fromEntry->name, toEntry->name);
  const StoreObjConstIter& it = m_storeObjMap[c_Event].find(relationsName);
  if (it == m_storeObjMap[c_Event].end()) {
    B2ERROR("No relation '" << relationsName << "' found. Please register it (using RelationArray::registerPersistent()) before trying to add relations.");
    return false;
  }
  StoreEntry* entry = it->second;

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

std::vector<RelationEntry> DataStore::getRelationsFromTo(const TObject* fromObject, DataStore::StoreEntry*& fromEntry, int& fromIndex, const TClass* toClass, const std::string& toName)
{
  std::vector<RelationEntry> result;

  // get entry from which the relations point
  if (!findStoreEntry(fromObject, fromEntry, fromIndex)) return result;

  // get names of store arrys to which the relations point
  std::vector<string> toNames;
  getArrayNames(toNames, toName, toClass);

  // loop over to store arrays
  for (std::vector<string>::const_iterator toNameIt = toNames.begin(); toNameIt != toNames.end(); ++toNameIt) {
    const StoreObjIter& arrayIter = m_storeObjMap[c_Event].find(*toNameIt);
    if (arrayIter == m_storeObjMap[c_Event].end() or arrayIter->second->ptr == NULL) continue;

    // get the relations from -> to
    const string& relationsName = relationName(fromEntry->name, *toNameIt);
    RelationIndex<TObject, TObject> relIndex(relationsName, c_Event);
    if (!relIndex)
      continue;

    //hack alert, since boost::multi_index requires unique types for the indices
    typedef RelationIndex<TObject, TObject>::Element relElement_t;
    //get relations with fromObject
    BOOST_FOREACH(const relElement_t & rel, relIndex.getElementsFrom(fromObject)) {
      TObject* const toObject = const_cast<TObject*>(rel.to);
      if (toObject)
        result.push_back(RelationEntry(toObject, rel.weight));
    }
  }

  return result;
}

std::vector<RelationEntry> DataStore::getRelationsToFrom(const TObject* toObject, DataStore::StoreEntry*& toEntry, int& toIndex, const TClass* fromClass, const std::string& fromName)
{
  std::vector<RelationEntry> result;

  // get entry to which the relations point
  if (!findStoreEntry(toObject, toEntry, toIndex)) return result;

  // get names of store arrys from which the relations point
  std::vector<string> fromNames;
  getArrayNames(fromNames, fromName, fromClass);

  // loop over from store arrays
  for (std::vector<string>::const_iterator fromNameIt = fromNames.begin(); fromNameIt != fromNames.end(); ++fromNameIt) {
    const StoreObjIter& arrayIter = m_storeObjMap[c_Event].find(*fromNameIt);
    if (arrayIter == m_storeObjMap[c_Event].end() or arrayIter->second->ptr == NULL) continue;

    // get the relations from -> to
    const string& relationsName = relationName(*fromNameIt, toEntry->name);
    RelationIndex<TObject, TObject> relIndex(relationsName, c_Event);
    if (!relIndex)
      continue;

    //hack alert, since boost::multi_index requires unique types for the indices
    typedef RelationIndex<TObject, TObject>::Element relElement_t;
    //get relations with toObject
    BOOST_FOREACH(const relElement_t & rel, relIndex.getElementsTo(toObject)) {
      TObject* const fromObject = const_cast<TObject*>(rel.from);
      if (fromObject)
        result.push_back(RelationEntry(fromObject, rel.weight));
    }
  }

  return result;
}

std::vector<RelationEntry> DataStore::getRelationsWith(const TObject* object, DataStore::StoreEntry*& entry, int& index, const TClass* withClass, const std::string& name)
{
  std::vector<RelationEntry> result = getRelationsFromTo(object, entry, index, withClass, name);

  const std::vector<RelationEntry>& fromResult = getRelationsToFrom(object, entry, index, withClass, name);

  result.insert(result.end(), fromResult.begin(), fromResult.end());
  return result;
}

RelationEntry DataStore::getRelationFromTo(const TObject* fromObject, DataStore::StoreEntry*& fromEntry, int& fromIndex, const TClass* toClass, const std::string& toName)
{
  // get entry from which the relations point
  if (!findStoreEntry(fromObject, fromEntry, fromIndex)) return RelationEntry(0);

  // get names of store arrys to which the relations point
  std::vector<string> toNames;
  getArrayNames(toNames, toName, toClass);

  // loop over to store arrays
  for (std::vector<string>::const_iterator toNameIt = toNames.begin(); toNameIt != toNames.end(); ++toNameIt) {
    const StoreObjIter& arrayIter = m_storeObjMap[c_Event].find(*toNameIt);
    if (arrayIter == m_storeObjMap[c_Event].end() or arrayIter->second->ptr == NULL) continue;

    // get the relations from -> to
    const string& relationsName = relationName(fromEntry->name, *toNameIt);
    RelationIndex<TObject, TObject> relIndex(relationsName, c_Event);
    if (!relIndex)
      continue;

    // get first element
    const RelationIndex<TObject, TObject>::Element* element = relIndex.getFirstElementFrom(fromObject);
    if (element && element->to) {
      return RelationEntry(const_cast<TObject*>(element->to), element->weight);
    }
  }

  return RelationEntry(0);
}

RelationEntry DataStore::getRelationToFrom(const TObject* toObject, DataStore::StoreEntry*& toEntry, int& toIndex, const TClass* fromClass, const std::string& fromName)
{
  // get entry to which the relations point
  if (!findStoreEntry(toObject, toEntry, toIndex)) return RelationEntry(0);

  // get names of store arrys from which the relations point
  std::vector<string> fromNames;
  getArrayNames(fromNames, fromName, fromClass);

  // loop over from store arrays
  for (std::vector<string>::const_iterator fromNameIt = fromNames.begin(); fromNameIt != fromNames.end(); ++fromNameIt) {
    const StoreObjIter& arrayIter = m_storeObjMap[c_Event].find(*fromNameIt);
    if (arrayIter == m_storeObjMap[c_Event].end() or arrayIter->second->ptr == NULL) continue;

    // get the relations from -> to
    const string& relationsName = relationName(*fromNameIt, toEntry->name);
    RelationIndex<TObject, TObject> relIndex(relationsName, c_Event);
    if (!relIndex)
      continue;

    // get first element
    const RelationIndex<TObject, TObject>::Element* element = relIndex.getFirstElementTo(toObject);
    if (element && element->from) {
      return RelationEntry(const_cast<TObject*>(element->from), element->weight);
    }
  }

  return RelationEntry(0);
}

RelationEntry DataStore::getRelationWith(const TObject* object, DataStore::StoreEntry*& entry, int& index, const TClass* withClass, const std::string& name)
{
  RelationEntry result = getRelationFromTo(object, entry, index, withClass, name);
  if (!result.object) {
    result = getRelationToFrom(object, entry, index, withClass, name);
  }
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

bool DataStore::require(const StoreAccessorBase& accessor)
{
  if (m_initializeActive) {
    ModuleInfo& info = m_moduleInfo[m_currentModule];
    info.addEntry(accessor.getName(), ModuleInfo::c_Input, (accessor.getClass() == RelationContainer::Class()));
  }

  if (!hasEntry(accessor)) {
    B2ERROR("The required DataStore entry with name " << accessor.getName() << " and durability " << accessor.getDurability() << " does not exist. Maybe you forgot the module that creates it?");
    return false;
  }
  return true;
}

bool DataStore::optionalInput(const StoreAccessorBase& accessor)
{
  if (m_initializeActive) {
    ModuleInfo& info = m_moduleInfo[m_currentModule];
    info.addEntry(accessor.getName(), ModuleInfo::c_OptionalInput, (accessor.getClass() == RelationContainer::Class()));
  }

  return hasEntry(accessor);
}
