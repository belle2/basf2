#include <analysis/utility/ParticleSubset.h>

using namespace Belle2;

void ParticleSubset::fixParticles(const std::map<int, int>& oldToNewMap)
{
  TClonesArray* arrayPtr = m_set->getPtr();
  for (Particle & p : *m_set) {
    unsigned int n = p.m_daughterIndices.size();
    for (unsigned int i = 0; i < n; i++) {
      p.m_daughterIndices[i] = oldToNewMap.at(p.m_daughterIndices[i]);
    }

    p.m_arrayPointer = arrayPtr;
  }
}

void ParticleSubset::fixParticleLists(const std::map<int, int>& oldToNewMap)
{
  const auto& entryMap = DataStore::Instance().getStoreEntryMap(DataStore::c_Event);
  for (const auto & entry : entryMap) {
    if (!entry.second.ptr or !entry.second.object->InheritsFrom(ParticleList::Class()))
      continue;

    ParticleList* list = static_cast<ParticleList*>(entry.second.ptr);
    fixVector(list->m_scList, oldToNewMap);
    fixVector(list->m_fsList, oldToNewMap);
  }
}

void ParticleSubset::fixVector(std::vector<int>& vec, const std::map<int, int>& oldToNewMap)
{
  const std::vector<int> oldList(vec);
  vec.clear();
  for (const int idx : oldList) {
    const auto& it = oldToNewMap.find(idx);
    if (it != oldToNewMap.end())
      vec.push_back(it->second);
  }
}
