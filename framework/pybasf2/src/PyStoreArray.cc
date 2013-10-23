#include <framework/pybasf2/PyStoreArray.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>

using namespace Belle2;

PyStoreArray::PyStoreArray(const std::string& name, int durability):
  m_storeArray(0)
{
  const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(DataStore::EDurability(durability));
  DataStore::StoreObjConstIter iter = map.find(name);
  if ((iter != map.end()) && iter->second->isArray) {
    m_storeArray = static_cast<TClonesArray*>(iter->second->ptr);
  }
}

TObject* PyStoreArray::appendNew()
{
  if (!m_storeArray) {
    B2ERROR("Invalid PyStoreArray, check name?");
    return NULL;
  }

  return m_storeArray->ConstructedAt(getEntries());
}

void PyStoreArray::list(int durability)
{
  const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(DataStore::EDurability(durability));
  for (const auto & entrypair : map) {
    if (entrypair.second->isArray) {
      B2INFO(entrypair.first);
    }
  }
}
