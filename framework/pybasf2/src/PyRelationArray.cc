#include <framework/pybasf2/PyRelationArray.h>

#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

PyRelationArray::PyRelationArray(const std::string& name, int durability):
  m_relations(0)
{
  const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(DataStore::EDurability(durability));
  DataStore::StoreObjConstIter iter = map.find(name);
  if ((iter == map.end()) || iter->second->isArray) {
    return;
  }

  m_relations = static_cast<RelationContainer*>(iter->second->ptr);

  //build index
  if (m_relations) {
    for (int i = 0; i < m_relations->getEntries(); i++) {
      const RelationElement& relation = m_relations->elements(i);
      const int from = relation.getFromIndex();

      if (m_toindicesMap.find(from) != m_toindicesMap.end()) {
        //assuming that RelationArray has been consolidate()'ed
        B2FATAL("PyRelationArray: key already exists!");
      }

      m_toindicesMap[from] = relation.getToIndices();
    }
  }
}

std::vector<unsigned int> PyRelationArray::getToIndices(unsigned int from) const
{
  std::map<unsigned int, std::vector<unsigned int> >::const_iterator iter = m_toindicesMap.find(from);
  if (iter == m_toindicesMap.end())
    return std::vector<unsigned int>();

  return iter->second;
}
