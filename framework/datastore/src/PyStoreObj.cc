#include <framework/datastore/PyStoreObj.h>

using namespace Belle2;

ClassImp(PyStoreObj)

PyStoreObj::PyStoreObj(const std::string& name, int durability):
  m_storeObjPtr(0)
{
  const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(DataStore::EDurability(durability));
  DataStore::StoreObjConstIter iter = map.find(name);
  if ((iter != map.end()) && !iter->second->isArray) {
    m_storeObjPtr = &(iter->second->ptr);
  }
}
