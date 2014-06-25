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
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <algorithm>

namespace Belle2 {

  void ParticleIndexGenerator::init(const std::vector<unsigned>& _sizes)
  {

    numberOfLists = _sizes.size();
    sizes = _sizes;

    iCombination = 0;

    indices.resize(numberOfLists);
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

  bool ParticleIndexGenerator::loadNext()
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

  const std::vector<unsigned int>& ParticleIndexGenerator::getCurrentIndices() const
  {
    return indices;
  }

  void ListIndexGenerator::init(unsigned int _numberOfLists)
  {
    numberOfLists = _numberOfLists;
    types.resize(numberOfLists);

    iCombination = 0;
    nCombinations = (1 << numberOfLists) - 1;

  }

  bool ListIndexGenerator::loadNext()
  {

    if (iCombination == nCombinations)
      return false;

    for (unsigned int i = 0; i < numberOfLists; ++i) {
      bool useSelfConjugatedDaughterList = (iCombination & (1 << i));
      types[i] = useSelfConjugatedDaughterList ? ParticleList::c_SelfConjugatedParticle : ParticleList::c_FlavorSpecificParticle;
    }

    ++iCombination;
    return true;
  }

  const std::vector<ParticleList::EParticleType>& ListIndexGenerator::getCurrentIndices() const
  {
    return types;
  }

  ParticleGenerator::ParticleGenerator(std::string decayString) : iParticleType(0), listIndexGenerator(), particleIndexGenerator()
  {

    DecayDescriptor decaydescriptor;
    bool valid = decaydescriptor.init(decayString);
    if (!valid)
      B2ERROR("Invalid input DecayString: " << decayString);

    // Mother particle
    const DecayDescriptorParticle* mother = decaydescriptor.getMother();
    m_pdgCode = mother->getPDGCode();

    // Daughters
    numberOfLists = decaydescriptor.getNDaughters();
    for (unsigned int i = 0; i < numberOfLists; ++i) {
      const DecayDescriptorParticle* daughter = decaydescriptor.getDaughter(i)->getMother();
      StoreObjPtr<ParticleList> list(daughter->getFullName());
      m_plists.push_back(list);
    }

    m_isSelfConjugated = decaydescriptor.isSelfConjugated();

  }

  void ParticleGenerator::init()
  {

    if (not m_isSelfConjugated) {
      iParticleType = 0;
    } else {
      iParticleType = 1;
    }

    particleIndexGenerator.init(std::vector<unsigned int> {}); // ParticleIndexGenerator will be initialised on first call
    listIndexGenerator.init(numberOfLists); // ListIndexGenerator must be initialised here!
    m_usedCombinations.clear();
    m_indices.resize(numberOfLists);
    m_particles.resize(numberOfLists);
  }

  bool ParticleGenerator::loadNext()
  {

    bool loadedNext = false;
    while (true) {
      switch (iParticleType) {
        case 0: loadedNext = loadNextParticle(true); break; //Anti-Particles
        case 1: loadedNext = loadNextParticle(false); break; //Particles
        case 2: loadedNext = loadNextSelfConjugatedParticle(); break;
        default: return false;
      }
      if (loadedNext)
        return true;
      ++iParticleType;

      if (iParticleType == 2) {
        std::vector<unsigned int> sizes(numberOfLists);
        for (unsigned int i = 0; i < numberOfLists; ++i) {
          sizes[i] = m_plists[i]->getList(ParticleList::c_SelfConjugatedParticle, false).size();
        }
        particleIndexGenerator.init(sizes);
      } else {
        listIndexGenerator.init(numberOfLists);
      }

    }
  }

  bool ParticleGenerator::loadNextParticle(bool useAntiParticle)
  {

    StoreArray<Particle> Particles;
    while (true) {

      // Load next index combination if available
      if (particleIndexGenerator.loadNext()) {

        const auto& types = listIndexGenerator.getCurrentIndices();
        const auto& indices = particleIndexGenerator.getCurrentIndices();

        for (unsigned int i = 0; i < numberOfLists; i++) {
          const auto& liste = m_plists[i]->getList(types[i], types[i] == ParticleList::c_FlavorSpecificParticle ? useAntiParticle : false);
          m_indices[i] =  liste[ indices[i] ];
          m_particles[i] = Particles[ m_indices[i] ];
        }

        if (not currentCombinationHasDifferentSources()) continue;
        if (not currentCombinationIsUnique()) continue;
        return true;
      }

      // Load next list combination if available and reset indexCombiner
      if (listIndexGenerator.loadNext()) {
        const auto& types = listIndexGenerator.getCurrentIndices();
        std::vector<unsigned int> sizes(numberOfLists);
        for (unsigned int i = 0; i < numberOfLists; ++i) {
          sizes[i] = m_plists[i]->getList(types[i], types[i] == ParticleList::c_FlavorSpecificParticle ? useAntiParticle : false).size();
        }
        particleIndexGenerator.init(sizes);
        continue;
      }
      return false;
    }

  }

  bool ParticleGenerator::loadNextSelfConjugatedParticle()
  {

    StoreArray<Particle> Particles;
    while (true) {

      // Load next index combination if available
      if (particleIndexGenerator.loadNext()) {

        const auto& indices = particleIndexGenerator.getCurrentIndices();

        for (unsigned int i = 0; i < numberOfLists; i++) {
          m_indices[i] = m_plists[i]->getList(ParticleList::c_SelfConjugatedParticle, false) [ indices[i] ];
          m_particles[i] = Particles[ m_indices[i] ];
        }

        if (not currentCombinationHasDifferentSources()) continue;
        if (not currentCombinationIsUnique()) continue;
        return true;
      }

      return false;
    }

  }


  Particle ParticleGenerator::getCurrentParticle() const
  {

    TLorentzVector vec(0., 0., 0., 0.);
    for (unsigned i = 0; i < m_particles.size(); i++) {
      vec = vec + m_particles[i]->get4Vector();
    }

    switch (iParticleType) {
      case 0: return Particle(vec, -m_pdgCode, m_isSelfConjugated ? Particle::c_Unflavored : Particle::c_Flavored, m_indices, m_particles[0]->getArrayPointer());
      case 1: return Particle(vec, m_pdgCode, m_isSelfConjugated ? Particle::c_Unflavored : Particle::c_Flavored, m_indices, m_particles[0]->getArrayPointer());
      case 2: return Particle(vec, m_pdgCode, Particle::c_Unflavored, m_indices, m_particles[0]->getArrayPointer());
      default: B2FATAL("Thomas you have a serious bug in the particle combiner");
    }

    return Particle(); // This should never happen
  }

  bool ParticleGenerator::currentCombinationHasDifferentSources()
  {

    StoreArray<Particle> Particles;
    std::vector<Particle*> stack = m_particles;
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


  bool ParticleGenerator::currentCombinationIsUnique()
  {
    const std::set<int> indexSet(m_indices.begin(), m_indices.end());
    bool elementInserted = m_usedCombinations.insert(indexSet).second;
    return elementInserted;
  }

}
