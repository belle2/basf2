/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <analysis/utility/ParticleSubset.h>

#include <analysis/dataobjects/ParticleList.h>
#include <framework/datastore/StoreObjPtr.h>

#include <unordered_set>

using namespace Belle2;

namespace {
  void keepParticle(const Particle* p, std::unordered_set<int>* indicesToKeep)
  {
    indicesToKeep->insert(p->getArrayIndex());
    unsigned int n = p->getNDaughters();
    for (unsigned int i = 0; i < n; i++) {
      keepParticle(p->getDaughter(i), indicesToKeep);
    }
  }
}

void ParticleSubset::removeParticlesNotInLists(const std::vector<std::string>& listNames)
{
  std::unordered_set<int> indicesToKeep;
  for (const auto& l : listNames) {
    StoreObjPtr<ParticleList> list(l);
    if (!list)
      continue;

    if (list->getParticleCollectionName() == m_set->getName()) {
      const int n = list->getListSize();
      for (int i = 0; i < n; i++) {
        const Particle* p = list->getParticle(i);
        keepParticle(p, &indicesToKeep);
      }
    } else {
      B2ERROR("ParticleList " << l << " uses Particle array '" << list->getParticleCollectionName() <<
              "', but ParticleSubset uses different array '" << m_set->getName() << "'!");
    }
  }

  //remove everything not in indicesToKeep
  auto selector = [indicesToKeep](const Particle * p) -> bool {
    int idx = p->getArrayIndex();
    return indicesToKeep.count(idx) == 1;
  };
  select(selector);
}

void ParticleSubset::fixParticles(const std::map<int, int>& oldToNewMap)
{
  TClonesArray* arrayPtr = m_set->getPtr();
  for (Particle& p : *m_set) {
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
  for (const auto& entry : entryMap) {
    if (!entry.second.ptr or !entry.second.object->InheritsFrom(ParticleList::Class()))
      continue;

    auto* list = static_cast<ParticleList*>(entry.second.ptr);
    if (list->getParticleCollectionName() == m_set->getName()) {
      fixVector(list->m_scList, oldToNewMap);
      fixVector(list->m_fsList, oldToNewMap);
    }
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

