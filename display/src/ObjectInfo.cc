#include <display/ObjectInfo.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/RelationsObject.h>

#include <TString.h>

using namespace Belle2;

TString ObjectInfo::getName(const TObject* obj)
{
  if (!obj)
    B2FATAL("ObjectInfo::getName() got null?");
  if (const RelationsObject* relObj = dynamic_cast<const RelationsObject*>(obj)) {
    return relObj->getName();
  }
  return "";
}

TString ObjectInfo::getInfo(const TObject* obj)
{
  if (!obj)
    B2FATAL("ObjectInfo::getInfo() got null?");
  if (const RelationsObject* relObj = dynamic_cast<const RelationsObject*>(obj)) {
    return relObj->getInfoHTML();
  }
  return "";
}

std::pair<std::string, int> ObjectInfo::getDataStorePosition(const TObject* obj)
{
  std::string name;
  int index = -1;
  if (const RelationsObject* relObj = dynamic_cast<const RelationsObject*>(obj)) {
    name = relObj->getArrayName();
    index = relObj->getArrayIndex();
  } else {
    //somewhat manual way to find location of object (might not inherit from RelationInterface)
    DataStore::StoreEntry* entry = nullptr;
    DataStore::Instance().findStoreEntry(obj, entry, index);
    if (entry)
      name = entry->name;
  }
  if (index == -1) {
    //this thing might be in a StoreObjPtr...
    for (const auto pair : DataStore::Instance().getStoreEntryMap(DataStore::c_Event)) {
      if (pair.second.object == obj) {
        name = pair.second.name;
        break;
      }
    }
  }

  return std::make_pair(name, index);
}

TString ObjectInfo::getIdentifier(const TObject* obj)
{
  auto pos = getDataStorePosition(obj);
  if (pos.second != -1)
    return TString::Format("%s[%d]", pos.first.c_str(), pos.second);
  return pos.first;
}

