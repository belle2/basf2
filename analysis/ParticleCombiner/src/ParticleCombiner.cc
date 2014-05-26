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

#include <algorithm>

namespace Belle2 {

  ListCombiner::ListCombiner(unsigned int numberOfLists) : numberOfLists(numberOfLists), iCombination(0), nCombinations(0), currentType(PCombinerList::c_Particle), types(numberOfLists) {  }

  void ListCombiner::init(PCombinerList::EParticleType _currentType)
  {
    iCombination = 0;
    currentType = _currentType;
    if (numberOfLists == 0)
      nCombinations = 0;
    else
      nCombinations = currentType == PCombinerList::c_SelfConjugatedParticle ? 1 : (1 << numberOfLists) - 1;
  }

  bool ListCombiner::loadNext()
  {

    if (iCombination == nCombinations)
      return false;

    for (unsigned int i = 0; i < numberOfLists; ++i) {
      bool useSelfConjugatedDaughterList = (iCombination & (1 << i));
      types[i] = useSelfConjugatedDaughterList ? PCombinerList::c_SelfConjugatedParticle : currentType;
    }

    ++iCombination;
    return true;
  }

  const std::vector<PCombinerList::EParticleType>& ListCombiner::getCurrentTypes() const
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

    if (numberOfLists == 0) {
      nCombinations = 0;
    }

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


  ParticleCombiner::ParticleCombiner(const std::vector<PCombinerList>& inputLists, bool isCombinedParticleSelfConjugated) : listCombiner(inputLists.size()), indexCombiner(inputLists.size()), m_isCombinedParticleSelfConjugated(isCombinedParticleSelfConjugated)
  {

    for (unsigned i = 0; i < inputLists.size(); i++) {
      PCombinerList list = inputLists[i];

      if (list.getParticleCollectionName() != std::string("Particles")) {
        B2ERROR("PCombinerList does not refer to the default Particle collection");
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

    while (true) {

      // Load next index combination if available
      if (indexCombiner.loadNext()) {
        if (loadNextCombination())
          return true;
        else
          continue;
      }

      // Load next list combination if available and reset indexCombiner
      if (listCombiner.loadNext()) {
        const auto& types = listCombiner.getCurrentTypes();
        std::vector<unsigned int> sizes(numberOfLists);
        for (unsigned int i = 0; i < numberOfLists; ++i) {
          sizes[i] = plists[i].getList(types[i]).size();
        }
        indexCombiner.init(sizes);
        continue;
      }

      // Load next type combination if available
      if (loadNextType()) {
        listCombiner.init((PCombinerList::EParticleType)(iType));
        continue;
      }

      return false;
    }

  }

  bool ParticleCombiner::loadNextType()
  {
    ++iType;
    return iType < nTypes;
  }

  Particle ParticleCombiner::getCurrentParticle(int pdg, int pdgbar) const
  {

    TLorentzVector vec(0., 0., 0., 0.);
    for (unsigned i = 0; i < m_particles.size(); i++) {
      vec = vec + m_particles[i]->get4Vector();
    }

    PCombinerList::EParticleType outputType = getCurrentType();
    return Particle(vec, outputType == PCombinerList::c_AntiParticle ? pdgbar :  pdg,
                    outputType == PCombinerList::c_SelfConjugatedParticle ? Particle::c_Unflavored : Particle::c_Flavored,
                    m_indices, m_particles[0]->getArrayPointer());
  }

  PCombinerList::EParticleType ParticleCombiner::getCurrentCombinationType() const
  {
    switch (iType) {
      case 0: return PCombinerList::c_Particle;
      case 1: return PCombinerList::c_AntiParticle;
      case 2: return PCombinerList::c_SelfConjugatedParticle;
      default: B2FATAL("Thomas you have a serious bug in the particle combiner!");
    }
    return PCombinerList::c_Particle; // This line can never be reached, as long B2FATAL kills the program!
  }


  PCombinerList::EParticleType ParticleCombiner::getCurrentType() const
  {
    if (m_isCombinedParticleSelfConjugated)
      return PCombinerList::c_SelfConjugatedParticle;
    return getCurrentCombinationType();
  }

  bool ParticleCombiner::loadNextCombination()
  {

    StoreArray<Particle> Particles;

    const auto& types = listCombiner.getCurrentTypes();
    const auto& indices = indexCombiner.getCurrentIndices();

    for (unsigned int i = 0; i < numberOfLists; i++) {
      m_indices[i] = plists[i].getList(types[i])[ indices[i] ];
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

  bool ParticleCombiner::isDecaySelfConjugated(std::vector<PCombinerList>& plists)
  {

    std::vector<int> decay, decaybar;
    for (unsigned i = 0; i < plists.size(); i++) {
      decay.push_back(plists[i].getPDG());
      decaybar.push_back(plists[i].getPDGbar());
    }
    std::sort(decay.begin(), decay.end());
    std::sort(decaybar.begin(), decaybar.end());
    return decay == decaybar;

  }

}
