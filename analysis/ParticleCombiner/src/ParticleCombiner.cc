/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/ParticleCombiner/ParticleCombiner.h>

#include <analysis/dataobjects/Particle.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  ListCombiner::ListCombiner(unsigned int numberOfLists) : numberOfLists(numberOfLists), iCombination(0), nCombinations(0), currentType(ParticleList::c_Particle), types(numberOfLists) {  }

  void ListCombiner::init(ParticleList::EParticleType _currentType)
  {
    iCombination = 0;
    currentType = _currentType;
    nCombinations = currentType == ParticleList::c_SelfConjugatedParticle ? 1 : (1 << numberOfLists) - 1;
  }

  bool ListCombiner::loadNext()
  {

    if (iCombination == nCombinations)
      return false;

    for (unsigned int i = 0; i < numberOfLists; ++i) {
      bool useSelfConjugatedDaughterList = (iCombination & (1 << i));
      types[i] = useSelfConjugatedDaughterList ? ParticleList::c_SelfConjugatedParticle : currentType;
    }

    ++iCombination;
    return true;
  }

  const std::vector<ParticleList::EParticleType>& ListCombiner::getCurrentTypes() const
  {
    return types;
  }


  IndexCombiner::IndexCombiner(unsigned int numberOfLists) : numberOfLists(numberOfLists), iCombination(0), nCombinations(0), indices(numberOfLists), sizes(numberOfLists) { }

  void IndexCombiner::init(const std::vector<unsigned>& _sizes)
  {

    sizes = _sizes;
    iCombination = 0;

    for (unsigned int i = 0; i < numberOfLists; ++i) {
      indices[i] = 0;
    }

    nCombinations = 1;
    for (unsigned int i = 0; i < numberOfLists; ++i)
      nCombinations *= sizes[i];

  }

  bool IndexCombiner::loadNext()
  {

    if (iCombination == nCombinations) {
      return false;
    }

    if (iCombination > 0) {

      for (unsigned int i = 0; i < numberOfLists; i++) {
        indices[i]++;
        if (indices[i] < sizes[i]) break;
        indices[i] = 0;
      }

    }

    iCombination++;
    return true;
  }

  const std::vector<unsigned int>& IndexCombiner::getCurrentIndices() const
  {
    return indices;
  }


  ParticleCombiner::ParticleCombiner(const std::vector<std::string>& inputListNames, bool isCombinedParticleSelfConjugated) : listCombiner(inputListNames.size()), indexCombiner(inputListNames.size()), m_isCombinedParticleSelfConjugated(isCombinedParticleSelfConjugated)
  {

    for (unsigned i = 0; i < inputListNames.size(); i++) {
      StoreObjPtr<ParticleList> list(inputListNames[i]);
      if (!list) {
        B2ERROR("ParticleList " << inputListNames[i] << " not found");
        return;
      }
      if (list->getParticleCollectionName() != std::string("Particles")) {
        B2ERROR("ParticleList " << inputListNames[i] <<
                " does not refer to the default Particle collection");
        return;
      }
      plists.push_back(list);
    }

    numberOfLists = plists.size();
    m_indices.resize(numberOfLists);
    m_particles.resize(numberOfLists);

    nTypes = 3;
    iType = -1;

    m_isCombinedParticleSelfConjugated = isCombinedParticleSelfConjugated || isDecaySelfConjugated(plists);

  }


  bool ParticleCombiner::loadNext()
  {

    while (indexCombiner.loadNext()) {
      if (loadNextCombination())
        return true;
    }

    while (loadNextType()) {
      listCombiner.init((ParticleList::EParticleType)(iType));
      while (listCombiner.loadNext()) {

        m_usedCombinations.clear();
        const auto& types = listCombiner.getCurrentTypes();
        std::vector<unsigned int> sizes(numberOfLists);
        for (unsigned int i = 0; i < numberOfLists; ++i) {
          sizes[i] = plists[i]->getList(types[i]).size();
        }
        indexCombiner.init(sizes);

        while (indexCombiner.loadNext()) {
          if (loadNextCombination())
            return true;
        }

      }
    }

    return false;

  }

  bool ParticleCombiner::loadNextType()
  {
    ++iType;
    return iType < nTypes;
  }


  ParticleList::EParticleType ParticleCombiner::getCurrentCombinationType() const
  {
    switch (iType) {
      case 0: return ParticleList::c_Particle;
      case 1: return ParticleList::c_AntiParticle;
      case 2: return ParticleList::c_SelfConjugatedParticle;
      default: B2FATAL("Thomas you have a serious bug in the particle combiner!");
    }
    return ParticleList::c_Particle; // This line can never be reached, as long B2FATAL kills the program!
  }


  ParticleList::EParticleType ParticleCombiner::getCurrentType() const
  {
    if (m_isCombinedParticleSelfConjugated)
      return ParticleList::c_SelfConjugatedParticle;
    return getCurrentCombinationType();
  }

  bool ParticleCombiner::loadNextCombination()
  {

    StoreArray<Particle> Particles;

    const auto& types = listCombiner.getCurrentTypes();
    const auto& indices = indexCombiner.getCurrentIndices();

    for (unsigned int i = 0; i < numberOfLists; i++) {
      m_indices[i] = plists[i]->getList(types[i])[ indices[i] ];
      m_particles[i] = Particles[ m_indices[i] ];
    }

    if (not currentCombinationHasDifferentSources()) return false;
    if (not currentCombinationIsUnique()) return false;

    return true;
  }

  bool ParticleCombiner::currentCombinationHasDifferentSources()
  {

    StoreArray<Particle> Particles;
    std::vector<Particle*> stack = getCurrentParticles();
    static std::vector<int> sources; // stack for particle sources
    sources.clear();

    while (!stack.empty()) {
      Particle* p = stack.back();
      stack.pop_back();
      const std::vector<int>& daughters = p->getDaughterIndices();
      if (daughters.empty()) {
        int source = p->getMdstSource();
        for (unsigned i = 0; i < sources.size(); i++) {
          if (source == sources[i]) return false;
        }
        sources.push_back(source);
      } else {
        for (unsigned i = 0; i < daughters.size(); i++) stack.push_back(Particles[daughters[i]]);
      }
    }
    return true;
  }


  bool ParticleCombiner::currentCombinationIsUnique()
  {
    const std::vector<int>& indices = getCurrentIndices();
    const std::set<int> indexSet(indices.begin(), indices.end());

    bool elementInserted = m_usedCombinations.insert(indexSet).second;

    //if it was not inserted, index combination was already used
    return elementInserted;
  }

  bool ParticleCombiner::isDecaySelfConjugated(std::vector<StoreObjPtr<ParticleList> >& plists)
  {

    std::vector<int> decay, decaybar;
    for (unsigned i = 0; i < plists.size(); i++) {
      decay.push_back(plists[i]->getPDG());
      decaybar.push_back(plists[i]->getPDGbar());
    }
    std::sort(decay.begin(), decay.end());
    std::sort(decaybar.begin(), decaybar.end());
    return decay == decaybar;

  }

}
