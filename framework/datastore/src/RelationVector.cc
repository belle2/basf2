/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/datastore/RelationVector.h>
#include <framework/datastore/RelationArray.h>

using namespace Belle2;

void RelationVectorBase::apply(int index,
                               const std::function<void(std::vector<RelationElement::index_type>&, std::vector<RelationElement::weight_type>&, size_t)>& f)
{
  const TObject* obj = m_relations.at(index).object;
  float objweight = m_relations.at(index).weight;

  for (const auto& name : m_relationNames) {
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

void RelationVectorBase::add(const RelationVectorBase& other)
{
  if ((other.m_name != m_name) or (other.m_index != m_index))
    B2FATAL("Trying to add RelationVectorBase for " << m_name << m_index << " and " << other.m_name << other.m_index << "!");

  m_relations.insert(m_relations.end(), other.m_relations.begin(), other.m_relations.end());
  if (!other.m_relations.empty())
    m_relationNames.insert(m_relationNames.end(), other.m_relationNames.begin(), other.m_relationNames.end());
}
