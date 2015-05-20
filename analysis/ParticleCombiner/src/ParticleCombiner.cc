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

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>

#include <framework/logging/Logger.h>

#include <algorithm>

namespace Belle2 {

  void ParticleIndexGenerator::init(const std::vector<unsigned>& _sizes)
  {

    m_numberOfLists = _sizes.size();
    sizes = _sizes;

    m_iCombination = 0;

    indices.resize(m_numberOfLists);
    for (unsigned int i = 0; i < m_numberOfLists; ++i) {
      indices[i] = 0;
    }

    m_nCombinations = 1;
    for (unsigned int i = 0; i < m_numberOfLists; ++i)
      m_nCombinations *= sizes[i];

    if (m_numberOfLists == 0) {
      m_nCombinations = 0;
    }

  }

  bool ParticleIndexGenerator::loadNext()
  {

    if (m_iCombination == m_nCombinations) {
      return false;
    }

    if (m_iCombination > 0) {

      for (unsigned int i = 0; i < m_numberOfLists; i++) {
        indices[i]++;
        if (indices[i] < sizes[i]) break;
        indices[i] = 0;
      }

    }

    m_iCombination++;
    return true;
  }

  const std::vector<unsigned int>& ParticleIndexGenerator::getCurrentIndices() const
  {
    return indices;
  }

  void ListIndexGenerator::init(unsigned int _numberOfLists)
  {
    m_numberOfLists = _numberOfLists;
    m_types.resize(m_numberOfLists);

    m_iCombination = 0;
    m_nCombinations = (1 << m_numberOfLists) - 1;

  }

  bool ListIndexGenerator::loadNext()
  {

    if (m_iCombination == m_nCombinations)
      return false;

    for (unsigned int i = 0; i < m_numberOfLists; ++i) {
      bool useSelfConjugatedDaughterList = (m_iCombination & (1 << i));
      m_types[i] = useSelfConjugatedDaughterList ? ParticleList::c_SelfConjugatedParticle : ParticleList::c_FlavorSpecificParticle;
    }

    ++m_iCombination;
    return true;
  }

  const std::vector<ParticleList::EParticleType>& ListIndexGenerator::getCurrentIndices() const
  {
    return m_types;
  }

  ParticleGenerator::ParticleGenerator(std::string decayString) : m_iParticleType(0), m_listIndexGenerator(),
    m_particleIndexGenerator()
  {

    DecayDescriptor decaydescriptor;
    bool valid = decaydescriptor.init(decayString);
    if (!valid)
      B2ERROR("Invalid input DecayString: " << decayString);

    // Mother particle
    const DecayDescriptorParticle* mother = decaydescriptor.getMother();
    m_pdgCode = mother->getPDGCode();

    // Daughters
    m_numberOfLists = decaydescriptor.getNDaughters();
    for (unsigned int i = 0; i < m_numberOfLists; ++i) {
      const DecayDescriptorParticle* daughter = decaydescriptor.getDaughter(i)->getMother();
      StoreObjPtr<ParticleList> list(daughter->getFullName());
      m_plists.push_back(list);
    }

    m_isSelfConjugated = decaydescriptor.isSelfConjugated();

  }

  void ParticleGenerator::init()
  {
    m_iParticleType = 0;

    m_particleIndexGenerator.init(std::vector<unsigned int> {}); // ParticleIndexGenerator will be initialised on first call
    m_listIndexGenerator.init(m_numberOfLists); // ListIndexGenerator must be initialised here!
    m_usedCombinations.clear();
    m_indices.resize(m_numberOfLists);
    m_particles.resize(m_numberOfLists);
  }

  bool ParticleGenerator::loadNext()
  {

    bool loadedNext = false;
    while (true) {
      switch (m_iParticleType) {
        case 0: loadedNext = loadNextParticle(true); break; //Anti-Particles
        case 1: loadedNext = loadNextParticle(false); break; //Particles
        case 2: loadedNext = loadNextSelfConjugatedParticle(); break;
        default: return false;
      }
      if (loadedNext)
        return true;
      ++m_iParticleType;

      if (m_iParticleType == 2) {
        std::vector<unsigned int> sizes(m_numberOfLists);
        for (unsigned int i = 0; i < m_numberOfLists; ++i) {
          sizes[i] = m_plists[i]->getList(ParticleList::c_SelfConjugatedParticle, false).size();
        }
        m_particleIndexGenerator.init(sizes);
      } else {
        m_listIndexGenerator.init(m_numberOfLists);
      }

    }
  }

  bool ParticleGenerator::loadNextParticle(bool useAntiParticle)
  {
    while (true) {

      // Load next index combination if available
      if (m_particleIndexGenerator.loadNext()) {

        const auto& m_types = m_listIndexGenerator.getCurrentIndices();
        const auto& indices = m_particleIndexGenerator.getCurrentIndices();

        for (unsigned int i = 0; i < m_numberOfLists; i++) {
          const auto& list = m_plists[i]->getList(m_types[i], m_types[i] == ParticleList::c_FlavorSpecificParticle ? useAntiParticle : false);
          m_indices[i] =  list[ indices[i] ];
          m_particles[i] = m_particleArray[ m_indices[i] ];
        }

        if (not currentCombinationHasDifferentSources()) continue;
        if (not currentCombinationIsUnique()) continue;
        return true;
      }

      // Load next list combination if available and reset indexCombiner
      if (m_listIndexGenerator.loadNext()) {
        const auto& m_types = m_listIndexGenerator.getCurrentIndices();
        std::vector<unsigned int> sizes(m_numberOfLists);
        for (unsigned int i = 0; i < m_numberOfLists; ++i) {
          sizes[i] = m_plists[i]->getList(m_types[i], m_types[i] == ParticleList::c_FlavorSpecificParticle ? useAntiParticle : false).size();
        }
        m_particleIndexGenerator.init(sizes);
        continue;
      }
      return false;
    }

  }

  bool ParticleGenerator::loadNextSelfConjugatedParticle()
  {
    while (true) {

      // Load next index combination if available
      if (m_particleIndexGenerator.loadNext()) {

        const auto& indices = m_particleIndexGenerator.getCurrentIndices();

        for (unsigned int i = 0; i < m_numberOfLists; i++) {
          m_indices[i] = m_plists[i]->getList(ParticleList::c_SelfConjugatedParticle, false) [ indices[i] ];
          m_particles[i] = m_particleArray[ m_indices[i] ];
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
      vec += m_particles[i]->get4Vector();
    }

    switch (m_iParticleType) {
      case 0: return Particle(vec, -m_pdgCode, m_isSelfConjugated ? Particle::c_Unflavored : Particle::c_Flavored, m_indices,
                                m_particleArray.getPtr());
      case 1: return Particle(vec, m_pdgCode, m_isSelfConjugated ? Particle::c_Unflavored : Particle::c_Flavored, m_indices,
                                m_particleArray.getPtr());
      case 2: return Particle(vec, m_pdgCode, Particle::c_Unflavored, m_indices, m_particleArray.getPtr());
      default: B2FATAL("Thomas you have a serious bug in the particle combiner");
    }

    return Particle(); // This should never happen
  }

  bool ParticleGenerator::currentCombinationHasDifferentSources()
  {

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
        for (unsigned i = 0; i < daughters.size(); i++) stack.push_back(m_particleArray[daughters[i]]);
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
