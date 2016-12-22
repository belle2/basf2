#include <framework/datastore/RelationVector.h>
#include <framework/datastore/RelationArray.h>

using namespace Belle2;

void RelationVectorBase::apply(int index,
                               const std::function<void(std::vector<RelationElement::index_type>&, std::vector<RelationElement::weight_type>&, size_t)>& f)
{
  const TObject* obj = m_relations.at(index).object;
  float objweight = m_relations.at(index).weight;

  for (auto name : m_relationNames) {
    RelationArray rel(name, DataStore::c_Event);

    //fill fromIndex, toIndex the rght way around
    unsigned int fromIndex, toIndex;
    StoreEntry* entry = nullptr;
    int otherIndex = -1;
    if (!DataStore::Instance().findStoreEntry(obj, entry, otherIndex))
      B2FATAL("RelationVectorBase points to object not in DataStore?");

    if (m_name == rel.getFromAccessorParams().first and entry->name == rel.getToAccessorParams().first) {
      fromIndex = m_index;
      toIndex = otherIndex;
    } else if (entry->name == rel.getFromAccessorParams().first and m_name == rel.getToAccessorParams().first) {
      toIndex = m_index;
      fromIndex = otherIndex;
    } else {
      continue;
    }

    //go through rel, see if we can find fromIndex and toIndex
    for (int i = 0; i < rel.getEntries(); i++) {
      if (rel[i].getFromIndex() == fromIndex) {
        auto& toIndices = const_cast<std::vector<RelationElement::index_type>&>(rel[i].getToIndices());
        auto& toWeights = const_cast<std::vector<RelationElement::weight_type>&>(rel[i].getWeights());
        for (size_t j = 0; j < toIndices.size(); j++) {
          //we also compare the weight here to handle duplicate relations properly
          if (toIndices[j] == toIndex and toWeights[j] == objweight) {
            f(toIndices, toWeights, j);
            rel.setModified(true);
            return;
          }
        }
      }
    }
  }

  B2FATAL("RelationVectorBase and DataStore differ!??");
}

/** Remove relation at given index. (Will decrease size() by one) */
void RelationVectorBase::remove(int index)
{
  apply(index, [](std::vector<RelationElement::index_type>& indices, std::vector<RelationElement::weight_type>& weights,
  size_t elidx) {
    indices.erase(indices.begin() + elidx);
    weights.erase(weights.begin() + elidx);
  });

  m_relations.erase(m_relations.begin() + index);
}

/** Set a new weight for the given relation. */
void RelationVectorBase::setWeight(int index, float weight)
{
  apply(index, [weight](std::vector<RelationElement::index_type>&, std::vector<RelationElement::weight_type>& weights, size_t elidx) {
    weights[elidx] = weight;
  });

  m_relations[index].weight = weight;
}

void RelationVectorBase::add(const RelationVectorBase& other)
{
  if (other.m_name != m_name or other.m_index != m_index)
    B2FATAL("Trying to add RelationVectorBase for " << m_name << m_index << " and " << other.m_name << other.m_index << "!");

  m_relations.insert(m_relations.end(), other.m_relations.begin(), other.m_relations.end());
  if (other.size() != 0)
    m_relationNames.insert(m_relationNames.end(), other.m_relationNames.begin(), other.m_relationNames.end());
}
