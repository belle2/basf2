#include <framework/pybasf2/PyStoreObj.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreAccessorBase.h>
#include <framework/dataobjects/RelationContainer.h>
#include <framework/logging/Logger.h>

#include <TObject.h>
#include <TClass.h>

using namespace Belle2;
using namespace std;

PyStoreObj::PyStoreObj(const std::string& name, int durability):
  m_storeObjPtr(nullptr),
  m_class(nullptr),
  m_name(name),
  m_durability(durability)
{
  attach();
}

void PyStoreObj::attach()
{
  DataStore::StoreEntry* entry = DataStore::Instance().getEntry(StoreAccessorBase(m_name, DataStore::EDurability(m_durability),
                                 TObject::Class(), false));
  if (entry) {
    m_storeObjPtr = &(entry->ptr);
    m_class = entry->object->IsA();
  }
}

TClass* PyStoreObj::getClass(const std::string& name)
{
  TClass* cl = TClass::GetClass(("Belle2::" + name).c_str());
  if (!cl)
    cl = TClass::GetClass(name.c_str());
  if (!cl) {
    B2ERROR("Class 'Belle2::" << name << "' (or '" << name <<
            "') not found! Note that you may need to load the corresponding library first, e.g. for some PXD object:\n  from ROOT import gSystem\n  gSystem.Load('libpxd_dataobjects')\nAfterwards, creating objects of this type should work.");
  }
  return cl;
}

bool PyStoreObj::create(bool replace)
{
  if (!m_class)
    return false;

  if (!DataStore::Instance().createObject(0, replace, StoreAccessorBase(m_name, DataStore::EDurability(m_durability), m_class,
                                          false)))
    return false;

  attach();
  return true;
}

bool PyStoreObj::registerInDataStore(std::string className, int storeFlags)
{
  m_class = getClass(className);
  if (!m_class)
    return false;

  return DataStore::Instance().registerEntry(m_name, DataStore::EDurability(m_durability), m_class, false, storeFlags);
}

vector<string> PyStoreObj::list(int durability)
{
  vector<string> list;
  const DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(DataStore::EDurability(durability));
  for (const auto& entrypair : map) {
    if (!entrypair.second.isArray) {
      const TObject* obj = entrypair.second.object;
      if (obj and dynamic_cast<const RelationContainer*>(obj))
        continue; //ignore relations in list
      list.push_back(entrypair.first);
    }
  }
  return list;
}

void PyStoreObj::printList(int durability)
{
  for (auto n : list(durability))
    B2INFO(n);
}
