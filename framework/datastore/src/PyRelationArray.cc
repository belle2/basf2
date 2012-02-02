#include <framework/datastore/PyRelationArray.h>

using namespace Belle2;

PyRelationArray::PyRelationArray(const std::string& name, int durability):
  TObject(),
  m_relations(name, DataStore::EDurability(durability))
{
  if (!isValid())
    return;

  //build index
  for (int i = 0; i < m_relations.getEntries(); i++) {
    const RelationElement& relation = m_relations[i];
    const int from = relation.getFromIndex();

    if (m_toindicesMap.find(from) != m_toindicesMap.end()) {
      //assuming that RelationArray has been consolidate()'ed
      B2FATAL("PyRelationArray: key already exists!");
    }

    m_toindicesMap[from] = relation.getToIndices();
  }
}

std::vector<unsigned int> PyRelationArray::getToIndices(unsigned int from) const
{
  std::map<unsigned int, std::vector<unsigned int> >::const_iterator iter = m_toindicesMap.find(from);
  if (iter == m_toindicesMap.end())
    return std::vector<unsigned int>();

  return iter->second;
}

ClassImp(PyRelationArray);
