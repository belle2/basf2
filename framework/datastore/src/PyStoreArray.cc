#include <framework/datastore/PyStoreArray.h>

using namespace Belle2;

ClassImp(PyStoreArray)

PyStoreArray::PyStoreArray(const std::string& name, int durability):
  m_storeArray(0)
{
  const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(DataStore::EDurability(durability));
  DataStore::StoreObjConstIter iter = map.find(name);
  if ((iter != map.end()) && iter->second->isArray) {
    m_storeArray = reinterpret_cast<TClonesArray**>(&(iter->second->ptr));
  }
}

