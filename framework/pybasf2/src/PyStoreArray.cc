#include <framework/pybasf2/PyStoreArray.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreAccessorBase.h>

using namespace Belle2;

PyStoreArray::PyStoreArray(const std::string& name, int durability):
  m_storeArray(0)
{
  DataStore::StoreEntry* entry = DataStore::Instance().getEntry(StoreAccessorBase(name, DataStore::EDurability(durability), TObject::Class(), true));
  if (entry)
    m_storeArray = static_cast<TClonesArray*>(entry->ptr);
}

TObject* PyStoreArray::appendNew()
{
  if (!m_storeArray) {
    B2ERROR("Invalid PyStoreArray, check name?");
    return NULL;
  }

  return m_storeArray->ConstructedAt(getEntries());
}
TObject* PyStoreArray::operator [](int i) const
{
  return m_storeArray->At(i);
}

void PyStoreArray::list(int durability)
{
  const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(DataStore::EDurability(durability));
  for (const auto & entrypair : map) {
    if (entrypair.second.isArray) {
      B2INFO(entrypair.first);
    }
  }
}
