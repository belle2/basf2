#include <framework/pybasf2/PyStoreArray.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreAccessorBase.h>

using namespace Belle2;
using namespace std;

PyStoreArray::PyStoreArray(const std::string& name, int durability):
  m_storeArray(0)
{
  StoreAccessorBase accessor(name, DataStore::EDurability(durability), TObject::Class(), true);
  DataStore::StoreEntry* entry = DataStore::Instance().getEntry(accessor);
  if (entry)
    m_storeArray = static_cast<TClonesArray*>(entry->ptr);
  else
    B2ERROR("PyStoreArray: " << accessor.readableName() << " does not exist!");
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
  if (!m_storeArray) {
    B2ERROR("Invalid PyStoreArray, check name?");
    return NULL;
  }
  return m_storeArray->At(i);
}

std::vector<std::string> PyStoreArray::list(int durability)
{
  vector<string> list;
  const DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(DataStore::EDurability(durability));
  for (const auto & entrypair : map) {
    if (entrypair.second.isArray) {
      list.push_back(entrypair.first);
    }
  }
  return list;
}

void PyStoreArray::printList(int durability)
{
  for (auto n : list(durability))
    B2INFO(n);
}
