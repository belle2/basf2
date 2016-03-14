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
#include <framework/datastore/RelationEntry.h>
#include <framework/datastore/DependencyMap.h>
#include <framework/dataobjects/RelationContainer.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationIndexManager.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/StoreAccessorBase.h>

#include <TClonesArray.h>
#include <TClass.h>

#include <unordered_map>
#include <algorithm>
#include <cstdlib>

using namespace std;
using namespace Belle2;

namespace {
  /** Called by exit handlers to free all memory.
   *
   * This is important since ROOT exit handlers may remove some of our objects.
   * Without this function, we will free the memory when the DataStore instance is destroyed, which happens too late.
   */
  void cleanDataStore()
  {
    DataStore::Instance().reset();
  }

  /** Workaround for
   * https://sft.its.cern.ch/jira/browse/ROOT-6995
   */
  void fixAbsorbObjects(TClonesArray* from, TClonesArray* to)
  {
    to->AbsorbObjects(from, 0, from->GetEntriesFast() - 1);
  }
}

bool DataStore::s_DoCleanup = false;

DataStore& DataStore::Instance()
{
  static DataStore instance;
  return instance;
}


DataStore::DataStore() : m_initializeActive(true), m_dependencyMap(new DependencyMap)
{
}

DataStore::~DataStore()
{
  if (s_DoCleanup) {
    //release all memory in data store
    reset();
  }
  delete m_dependencyMap;
}

void DataStore::reset()
{
  m_initializeActive = true;
  m_dependencyMap->clear();

  for (int i = 0; i < c_NDurabilityTypes; i++)
    reset((EDurability)i);
}

void DataStore::reset(EDurability durability)
{
  for (auto& mapEntry : m_storeEntryMap[durability]) {
    //delete stored object/array
    delete mapEntry.second.object;
  }
  m_storeEntryMap[durability].clear();
  //invalidate any cached relations (expect RelationArrays to remain valid)
  RelationIndexManager::Instance().clear();
}

void DataStore::setInitializeActive(bool active)
{
  m_initializeActive = active;

  static bool firstCall = true;
  if (firstCall) {
    atexit(cleanDataStore);
    firstCall = false;
  }
}

std::string DataStore::defaultObjectName(std::string classname)
{
  if (classname == "genfit::Track")
    return "GF2Track";
  //Strip qualifiers like namespaces
  size_t colon = classname.rfind("::");
  if (colon != std::string::npos) {
    classname = classname.substr(colon + 2);
  }
  return classname;
}


bool DataStore::checkType(const StoreEntry& entry, const StoreAccessorBase& accessor) const
{
  // Check whether the existing entry and the requested object are both arrays or both single objects
  const char* entryType = (entry.isArray) ? "array" : "object";
  if (entry.isArray != accessor.isArray()) {
    B2FATAL("Existing entry '" << entry.name << "' is an " << entryType << " and the requested one an " << ((
              accessor.isArray()) ? "array" : "object"));
  }

  // Check whether the existing entry has the same type
  TClass* entryClass = entry.object->IsA();
  if (entry.isArray) {
    entryClass = static_cast<TClonesArray*>(entry.object)->GetClass();
  }
  if (!entryClass->InheritsFrom(accessor.getClass())) {
    B2FATAL("Existing " << accessor.readableName() << " of type " << entryClass->GetName() << " doesn't match requested type " <<
            accessor.getClass()->GetName());
  }

  return true;
}


bool DataStore::registerEntry(const std::string& name, EDurability durability,
                              const TClass* objClass, bool array, EStoreFlags storeFlags)
{
  const StoreAccessorBase accessor(name, durability, objClass, array);
  // Check whether this method is called in the initialization phase
  if (!m_initializeActive) {
    B2ERROR("Attempt to register " << accessor.readableName() <<
            " outside of the initialization phase. Please move calls to registerInDataStore() into your Module's initialize() function.");
    return false;
  }
  const bool dontwriteout = storeFlags & c_DontWriteOut;

  //add to current module's outputs
  m_dependencyMap->getCurrentModuleInfo().addEntry(name, DependencyMap::c_Output, (objClass == RelationContainer::Class()));

  // Check whether the map entry already exists
  const auto& it = m_storeEntryMap[durability].find(name);
  if (it != m_storeEntryMap[durability].end()) {
    StoreEntry& entry = it->second;

    // Complain about existing entry
    if (storeFlags & c_ErrorIfAlreadyRegistered) {
      B2ERROR("An " << accessor.readableName() << " of type " << entry.object->ClassName() <<
              " was already registered before. (Multiple calls to registerInDataStore() are fine if the c_ErrorIfAlreadyRegistered flag is not set. For objects you will want to make sure that you don't discard existing data from other modules in that case.");
      return false;
    }

    // Check whether the types match
    if (!checkType(entry, accessor)) return false;

    // Check whether the persistency type matches
    if (entry.dontWriteOut != dontwriteout) {
      B2WARNING("Existing " << accessor.readableName() << " has different persistency type than requested. Changing to " <<
                (dontwriteout ? "c_DontWriteOut" : "c_WriteOut") << ".");
      entry.dontWriteOut = dontwriteout;
    }

    B2DEBUG(100, "An " << accessor.readableName() << " was registered once more.");
    return true;
  }

  // check reserved names
  if (array and name == "ALL") {
    B2ERROR("Creating an array with the reserved name 'ALL' is not allowed!");
    return false;
  }

  // Add the DataStore entry
  StoreEntry& entry = m_storeEntryMap[durability][name];
  entry.isArray = array;
  entry.dontWriteOut = dontwriteout;
  if (array) {
    entry.object = new TClonesArray(objClass);
  } else {
    entry.object = static_cast<TObject*>(objClass->New());
  }
  entry.name = name;

  B2DEBUG(100, "Successfully registered " << accessor.readableName());
  return true;
}

bool DataStore::registerRelation(const StoreAccessorBase& fromArray, const StoreAccessorBase& toArray, EDurability durability,
                                 EStoreFlags storeFlags)
{
  if (!fromArray.isArray())
    B2FATAL(fromArray.readableName() << " is not an array!");
  if (!toArray.isArray())
    B2FATAL(toArray.readableName() << " is not an array!");

  const std::string& relName = relationName(fromArray.getName(), toArray.getName());
  /*
  if ((fromArray.notWrittenOut() or toArray.notWrittenOut()) and !(storeFlags & c_DontWriteOut)) {
    B2WARNING("You're trying to register a persistent relation " << relName << " from/to an array which is not written out (DataStore::c_DontWriteOut flag)! Relation will also not be saved!");
    storeFlags |= c_DontWriteOut;
  }
  */

  if (fromArray.getDurability() > durability or toArray.getDurability() > durability) {
    B2FATAL("Tried to create a relation '" << relName << "' with a durability larger than the StoreArrays it relates");
  }

  return DataStore::Instance().registerEntry(relName, durability, RelationContainer::Class(), false, storeFlags);
}


DataStore::StoreEntry* DataStore::getEntry(const StoreAccessorBase& accessor)
{
  const auto& it = m_storeEntryMap[accessor.getDurability()].find(accessor.getName());

  if (it != m_storeEntryMap[accessor.getDurability()].end() and checkType((it->second), accessor)) {
    return &(it->second);
  } else {
    return nullptr;
  }
}


TObject** DataStore::getObject(const StoreAccessorBase& accessor)
{
  StoreEntry* entry = getEntry(accessor);
  if (!entry) {
    return nullptr;
  }
  return &(entry->ptr);
}


bool DataStore::createObject(TObject* object, bool replace, const StoreAccessorBase& accessor)
{
  StoreEntry* entry = getEntry(accessor);
  if (!entry) {
    B2ERROR("No " << accessor.readableName() <<
            " exists in the DataStore, did you forget to register it in your Module's initialize() function? Note: direct accesses to it will crash!");
    return false;
  }

  if (entry->ptr && !replace) {
    B2ERROR("An " << accessor.readableName() << " was already created in the DataStore.");
    return false;
  }

  if (object) {
    delete entry->object;
    entry->object = object;
    entry->ptr = entry->object;
  } else {
    entry->recreate();
  }

  return true;
}

void DataStore::replaceData(const StoreAccessorBase& from, const StoreAccessorBase& to)
{
  StoreEntry* fromEntry = getEntry(from);
  StoreEntry* toEntry = getEntry(to);
  if (!fromEntry)
    B2FATAL("No " << from.readableName() << " exists in the DataStore!");
  if (!toEntry)
    B2FATAL("No " << to.readableName() << " exists in the DataStore!");

  if (from.isArray() != to.isArray() or from.getClass() != to.getClass()) {
    B2FATAL("cannot replace " << to.readableName() << " with " << from.readableName() << " (incompatible types)!");
  }

  if (!fromEntry->ptr) {
    //since we don't need to move any data, just invalidate toEntry instead.
    toEntry->ptr = nullptr;
  } else if (from.isArray()) {
    if (!toEntry->ptr)
      toEntry->ptr = toEntry->object;
    toEntry->getPtrAsArray()->Delete();

    fixAbsorbObjects(fromEntry->getPtrAsArray(), toEntry->getPtrAsArray());
    updateRelationsObjectCache(*toEntry);
  } else if (from.getClass() == RelationContainer::Class()) {
    if (!toEntry->ptr)
      toEntry->ptr = toEntry->object;
    RelationContainer* fromRel = static_cast<RelationContainer*>(fromEntry->ptr);
    RelationContainer* toRel = static_cast<RelationContainer*>(toEntry->ptr);

    toRel->elements().Delete();

    fixAbsorbObjects(&fromRel->elements(), &toRel->elements());

    //indices need a rebuild
    fromRel->setModified(true);
    toRel->setModified(true);
  } else {
    delete toEntry->object;

    toEntry->object = fromEntry->ptr->Clone();
    toEntry->ptr = toEntry->object;

    fromEntry->ptr = nullptr;
  }
}

void DataStore::updateRelationsObjectCache(StoreEntry& entry)
{
  const TClonesArray* array = static_cast<TClonesArray*>(entry.ptr);
  const int nEntries = array->GetEntriesFast();
  for (int i = 0; i < nEntries; i++) {
    RelationsObject* relobj = static_cast<RelationsObject*>(array->At(i));
    relobj->m_cacheArrayIndex = i;
    relobj->m_cacheDataStoreEntry = &entry;
  }
}

bool DataStore::findStoreEntry(const TObject* object, DataStore::StoreEntry*& entry, int& index)
{
  // check whether the cached information is (still) valid
  if (entry && entry->ptr && (index >= 0)) {
    const TClonesArray* array = static_cast<TClonesArray*>(entry->ptr);
    if (array->At(index) == object) return true;
    B2INFO("findStoreEntry: cached index invalid, probably harmless but odd : " << entry->name << " idx " << index);
    index = array->IndexOf(object);
    if (index >= 0) return true;
    B2INFO("findStoreEntry: cached entry was also wrong");
  }
  entry = nullptr;
  index = -1;

  //searching for nullptr should be safe
  if (!object)
    return false;

  // search for the object and set the entry and index
  const TClass* objectClass = object->IsA();
  for (auto& mapEntry : m_storeEntryMap[c_Event]) {
    if (mapEntry.second.ptr && mapEntry.second.isArray) {
      const TClonesArray* array = static_cast<TClonesArray*>(mapEntry.second.ptr);
      const TClass* arrayClass = array->GetClass();
      if (arrayClass == objectClass) {
        if (object == array->Last()) {
          //quickly find entry if it's at the end of the array
          index = array->GetLast();
        } else {
          if (arrayClass->InheritsFrom(RelationsObject::Class())) {
            //update cache for entire array
            updateRelationsObjectCache(mapEntry.second);

            //if found, m_cacheArrayIndex is now correct, otherwise still -1
            index = static_cast<const RelationsObject*>(object)->m_cacheArrayIndex;
          } else {
            //not a RelationsObject, so no caching
            index = array->IndexOf(object);
          }
        }

        if (index >= 0) {
          entry = &mapEntry.second;
          return true;
        }
      }
    }
  }
  return false;
}

void DataStore::getArrayNames(std::vector<std::string>& names, const std::string& arrayName, const TClass* arrayClass,
                              EDurability durability) const
{
  if (arrayName.empty()) {
    static std::unordered_map<const TClass*, string> classToArrayName;
    const auto& it = classToArrayName.find(arrayClass);
    if (it != classToArrayName.end()) {
      names.push_back(it->second);
    } else {
      std::string result = defaultArrayName(arrayClass->GetName());
      classToArrayName[arrayClass] = result;
      names.push_back(result);
    }
  } else if (arrayName == "ALL") {
    for (auto& mapEntry : m_storeEntryMap[durability]) {
      if (mapEntry.second.ptr && mapEntry.second.isArray) {
        TClonesArray* array = static_cast<TClonesArray*>(mapEntry.second.ptr);
        if (array->GetClass()->InheritsFrom(arrayClass)) {
          names.push_back(mapEntry.second.name);
        }
      }
    }
  } else {
    names.push_back(arrayName);
  }
}

void DataStore::addRelation(const TObject* fromObject, StoreEntry*& fromEntry, int& fromIndex, const TObject* toObject,
                            StoreEntry*& toEntry, int& toIndex, double weight)
{
  if (!fromObject or !toObject)
    return;

  // get entry from which the relation points
  if (!findStoreEntry(fromObject, fromEntry, fromIndex)) {
    B2FATAL("Couldn't find from-side entry for relation between " << fromObject->ClassName() << " and " << toObject->ClassName());
  }

  // get entry to which the relation points
  if (!findStoreEntry(toObject, toEntry, toIndex)) {
    B2FATAL("Couldn't find to-side entry for relation between " << fromObject->ClassName() << " and " << toObject->ClassName());
  }

  // get the relations from -> to
  const string& relationsName = relationName(fromEntry->name, toEntry->name);
  const StoreEntryIter& it = m_storeEntryMap[c_Event].find(relationsName);
  if (it == m_storeEntryMap[c_Event].end()) {
    B2FATAL("No relation '" << relationsName <<
            "' found. Please register it (using StoreArray::registerRelationTo()) before trying to add relations.");
  }
  StoreEntry* entry = &(it->second);

  // auto create relations if needed (both if null pointer, or uninitialised object read from TTree)
  if (!entry->ptr)
    entry->recreate();
  RelationContainer* relContainer = static_cast<RelationContainer*>(entry->ptr);
  if (relContainer->isDefaultConstructed()) {
    relContainer->setFromName(fromEntry->name);
    relContainer->setFromDurability(c_Event);
    relContainer->setToName(toEntry->name);
    relContainer->setToDurability(c_Event);
  }

  // add relation
  TClonesArray& relations = relContainer->elements();
  new(relations.AddrAt(relations.GetLast() + 1)) RelationElement(fromIndex, toIndex, weight);

  std::shared_ptr<RelationIndexContainer<TObject, TObject>> relIndex =
                                                           RelationIndexManager::Instance().getIndexIfExists<TObject, TObject>(relationsName, c_Event);
  if (relIndex) {
    // add it to index (so we avoid expensive rebuilding later)
    relIndex->index().insert(RelationIndex<TObject, TObject>::Element(fromIndex, toIndex, fromObject, toObject, weight));
  } else {
    //mark for rebuilding later on
    relContainer->setModified(true);
  }
}

std::vector<RelationEntry> DataStore::getRelationsWith(ESearchSide searchSide, const TObject* object, DataStore::StoreEntry*& entry,
                                                       int& index, const TClass* withClass, const std::string& withName)
{
  if (searchSide == c_BothSides) {
    std::vector<RelationEntry> result = getRelationsWith(c_ToSide, object, entry, index, withClass, withName);
    const std::vector<RelationEntry>& fromResult = getRelationsWith(c_FromSide, object, entry, index, withClass, withName);
    result.insert(result.end(), fromResult.begin(), fromResult.end());
    return result;
  }

  std::vector<RelationEntry> result;

  // get StoreEntry for 'object'
  if (!findStoreEntry(object, entry, index)) return result;

  // get names of store arrays to search
  std::vector<string> names;
  getArrayNames(names, withName, withClass);

  // loop over found store arrays
  for (const std::string& name : names) {
    const StoreEntryConstIter& arrayIter = m_storeEntryMap[c_Event].find(name);
    if (arrayIter == m_storeEntryMap[c_Event].end() or arrayIter->second.ptr == nullptr) continue;

    // get the relations from -> to
    const string& relationsName = (searchSide == c_ToSide) ? relationName(entry->name, name) : relationName(name, entry->name);
    RelationIndex<TObject, TObject> relIndex(relationsName, c_Event);
    if (!relIndex)
      continue;

    //get relations with object
    if (searchSide == c_ToSide) {
      for (const auto& rel : relIndex.getElementsFrom(object)) {
        TObject* const toObject = const_cast<TObject*>(rel.to);
        if (toObject)
          result.emplace_back(toObject, rel.weight);
      }
    } else {
      for (const auto& rel : relIndex.getElementsTo(object)) {
        TObject* const fromObject = const_cast<TObject*>(rel.from);
        if (fromObject)
          result.emplace_back(fromObject, rel.weight);
      }
    }
  }

  return result;
}

RelationEntry DataStore::getRelationWith(ESearchSide searchSide, const TObject* object, DataStore::StoreEntry*& entry, int& index,
                                         const TClass* withClass, const std::string& withName)
{
  if (searchSide == c_BothSides) {
    RelationEntry result = getRelationWith(c_ToSide, object, entry, index, withClass, withName);
    if (!result.object) {
      result = getRelationWith(c_FromSide, object, entry, index, withClass, withName);
    }
    return result;
  }

  // get StoreEntry for 'object'
  if (!findStoreEntry(object, entry, index)) return RelationEntry(nullptr);

  // get names of store arrays to search
  std::vector<string> names;
  getArrayNames(names, withName, withClass);

  // loop over found store arrays
  for (const std::string& name : names) {
    const StoreEntryConstIter& arrayIter = m_storeEntryMap[c_Event].find(name);
    if (arrayIter == m_storeEntryMap[c_Event].end() or arrayIter->second.ptr == nullptr) continue;

    // get the relations from -> to
    const string& relationsName = (searchSide == c_ToSide) ? relationName(entry->name, name) : relationName(name, entry->name);
    RelationIndex<TObject, TObject> relIndex(relationsName, c_Event);
    if (!relIndex)
      continue;

    // get first element
    if (searchSide == c_ToSide) {
      const RelationIndex<TObject, TObject>::Element* element = relIndex.getFirstElementFrom(object);
      if (element && element->to) {
        return RelationEntry(const_cast<TObject*>(element->to), element->weight);
      }
    } else {
      const RelationIndex<TObject, TObject>::Element* element = relIndex.getFirstElementTo(object);
      if (element && element->from) {
        return RelationEntry(const_cast<TObject*>(element->from), element->weight);
      }
    }
  }

  return RelationEntry(nullptr);
}

std::vector<std::string> DataStore::getListOfRelatedArrays(const StoreAccessorBase& array) const
{
  std::vector<std::string> arrays;
  if (!array.isArray()) {
    B2ERROR("getListOfRelatedArrays(): " << array.readableName() << " is not an array!");
    return arrays;
  }

  //loop over all arrays
  EDurability durability = array.getDurability();
  for (auto& mapEntry : m_storeEntryMap[durability]) {
    if (mapEntry.second.isArray) {
      const std::string& name = mapEntry.second.name;

      //check both from & to 'array'
      for (int searchSide = 0; searchSide < c_BothSides; searchSide++) {
        const string& relationsName = (searchSide == c_ToSide) ? relationName(array.getName(), name) : relationName(name, array.getName());
        const StoreEntryConstIter& it = m_storeEntryMap[durability].find(relationsName);
        if (it != m_storeEntryMap[durability].end())
          arrays.push_back(name);
      }
    }
  }

  return arrays;
}
std::vector<std::string> DataStore::getListOfArrays(const TClass* arrayClass, EDurability durability) const
{
  std::vector<std::string> arrays;
  getArrayNames(arrays, "ALL", arrayClass, durability);
  return arrays;
}

std::vector<std::string> DataStore::getListOfObjects(const TClass* objClass, EDurability durability) const
{
  vector<string> list;
  const DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(DataStore::EDurability(durability));
  for (const auto& entrypair : map) {
    if (!entrypair.second.isArray) {
      const TObject* obj = entrypair.second.object;
      if (obj and dynamic_cast<const RelationContainer*>(obj))
        continue; //ignore relations in list

      if (obj->IsA()->InheritsFrom(objClass))
        list.push_back(entrypair.first);
    }
  }
  return list;
}

void DataStore::invalidateData(EDurability durability)
{
  B2DEBUG(100, "Invalidating objects for durability " << durability);
  for (auto& mapEntry : m_storeEntryMap[durability]) {
    mapEntry.second.ptr = nullptr;
  }
}

bool DataStore::requireInput(const StoreAccessorBase& accessor)
{
  if (m_initializeActive) {
    m_dependencyMap->getCurrentModuleInfo().addEntry(accessor.getName(), DependencyMap::c_Input,
                                                     (accessor.getClass() == RelationContainer::Class()));
  }

  if (!getEntry(accessor)) {
    B2ERROR("The required " << accessor.readableName() << " does not exist. Maybe you forgot the module that registers it?");
    return false;
  }
  return true;
}

bool DataStore::optionalInput(const StoreAccessorBase& accessor)
{
  if (m_initializeActive) {
    m_dependencyMap->getCurrentModuleInfo().addEntry(accessor.getName(), DependencyMap::c_OptionalInput,
                                                     (accessor.getClass() == RelationContainer::Class()));
  }

  return (getEntry(accessor) != nullptr);
}

bool DataStore::requireRelation(const StoreAccessorBase& fromArray, const StoreAccessorBase& toArray, EDurability durability)
{
  if (!fromArray.isArray())
    B2FATAL(fromArray.readableName() << " is not an array!");
  if (!toArray.isArray())
    B2FATAL(toArray.readableName() << " is not an array!");

  const std::string& relName = relationName(fromArray.getName(), toArray.getName());
  return DataStore::Instance().requireInput(StoreAccessorBase(relName, durability, RelationContainer::Class(), false));
}

bool DataStore::optionalRelation(const StoreAccessorBase& fromArray, const StoreAccessorBase& toArray, EDurability durability)
{
  if (!fromArray.isArray())
    B2FATAL(fromArray.readableName() << " is not an array!");
  if (!toArray.isArray())
    B2FATAL(toArray.readableName() << " is not an array!");

  const std::string& relName = relationName(fromArray.getName(), toArray.getName());
  return DataStore::Instance().optionalInput(StoreAccessorBase(relName, durability, RelationContainer::Class(), false));
}
