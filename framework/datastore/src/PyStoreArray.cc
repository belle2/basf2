#include <framework/datastore/PyStoreArray.h>

#include <framework/datastore/DataStore.h>

using namespace Belle2;

PyStoreArray::PyStoreArray(const std::string& name, int durability):
  m_storeArray(0)
{
  const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(DataStore::EDurability(durability));
  DataStore::StoreObjConstIter iter = map.find(name);
  if ((iter != map.end()) && iter->second->isArray) {
    m_storeArray = reinterpret_cast<TClonesArray*>(iter->second->ptr);
  }
}
