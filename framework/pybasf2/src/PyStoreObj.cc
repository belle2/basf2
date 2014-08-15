#include <framework/pybasf2/PyStoreObj.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreAccessorBase.h>
#include <framework/dataobjects/RelationContainer.h>
#include <framework/logging/Logger.h>

#include <TObject.h>
#include <TClass.h>

using namespace Belle2;

PyStoreObj::PyStoreObj(const std::string& name, int durability):
  m_storeObjPtr(0),
  m_name(name),
  m_durability(durability)
{
  DataStore::StoreEntry* entry = DataStore::Instance().getEntry(StoreAccessorBase(name, DataStore::EDurability(durability), TObject::Class(), false));
  if (entry)
    m_storeObjPtr = &(entry->ptr);
  m_storeObjPtr = DataStore::Instance().getObject(StoreAccessorBase(name, DataStore::EDurability(durability), TObject::Class(), false));
}

TClass* PyStoreObj::getClass(const std::string& name)
{
  TClass* cl = TClass::GetClass(("Belle2::" + name).c_str());
  if (!cl) {
    B2ERROR("Class 'Belle2::" << name << "' not found! Note that you may need to load the corresponding library first, e.g. for some PXD object:\n  from ROOT import gSystem\n  gSystem.Load('libpxd_dataobjects')\nAfterwards, creating objects of this type should work.");
  }
  return cl;
}

bool PyStoreObj::create(bool replace)
{
  TClass* cl = getClass(m_name);
  if (!cl)
    return false;

  return DataStore::Instance().createObject(0, replace, StoreAccessorBase(m_name, DataStore::EDurability(m_durability), cl, false));
}

bool PyStoreObj::registerInDataStore(int storeFlags)
{
  TClass* cl = getClass(m_name);
  if (!cl)
    return false;

  return DataStore::Instance().registerEntry(m_name, DataStore::EDurability(m_durability), cl, false, storeFlags);
}

void PyStoreObj::list(int durability)
{
  const DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(DataStore::EDurability(durability));
  for (const auto & entrypair : map) {
    if (!entrypair.second.isArray) {
      const TObject* obj = entrypair.second.object;
      if (obj and dynamic_cast<const RelationContainer*>(obj))
        continue; //ignore relations in list
      B2INFO(entrypair.first);
    }
  }
}
