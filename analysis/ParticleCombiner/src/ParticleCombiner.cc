/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck, Anze Zupanc                                 *
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

    // check if input lists can contain copies
    // remember (list_i, list_j) pairs, where list_i and list_j can contain copies
    m_inputListsCollide = false;
    m_collidingLists.clear();
    for (unsigned int i = 0; i < m_numberOfLists; ++i) {
      const DecayDescriptorParticle* daughter_i = decaydescriptor.getDaughter(i)->getMother();
      for (unsigned int j = i + 1; j < m_numberOfLists; ++j) {
        const DecayDescriptorParticle* daughter_j = decaydescriptor.getDaughter(j)->getMother();

        if (abs(daughter_i->getPDGCode()) != abs(daughter_j->getPDGCode()))
          continue;

        if (daughter_i->getLabel() != daughter_j->getLabel()) {
          m_inputListsCollide = true;
          m_collidingLists.push_back(std::make_pair(i, j));
        }
      }
    }

  }

  void ParticleGenerator::init()
  {
    m_iParticleType = 0;

    m_particleIndexGenerator.init(std::vector<unsigned int> {}); // ParticleIndexGenerator will be initialised on first call
    m_listIndexGenerator.init(m_numberOfLists); // ListIndexGenerator must be initialised here!
    m_usedCombinations.clear();
    m_indices.resize(m_numberOfLists);
    m_particles.resize(m_numberOfLists);

    if (m_inputListsCollide)
      initIndicesToUniqueIDMap();
  }

  void ParticleGenerator::initIndicesToUniqueIDMap()
  {
    m_indicesToUniqueIDs.clear();

    unsigned inputParticlesCount = 0;
    for (unsigned int i = 0; i < m_numberOfLists; ++i)
      inputParticlesCount +=  m_plists[i]->getListSize();

    m_indicesToUniqueIDs.reserve(inputParticlesCount);

    int uniqueID = 1;

    for (unsigned i = 0; i < m_collidingLists.size(); i++) {
      StoreObjPtr<ParticleList> listA =  m_plists[m_collidingLists[i].first];
      StoreObjPtr<ParticleList> listB =  m_plists[m_collidingLists[i].second];

      bool sameSign = (listA->getPDGCode() == listB->getPDGCode());

      // if sameSign == true then
      // 1. compare FS to FS particles in lists A and B
      // 2. compare anti-FS to anti-FS particles in lists A and B
      // else
      // 1. compare FS to anti-FS particles in lists A and B
      // 2. compare anti-FS to FS particles in lists A and B
      // and in either case compare
      // 3. compare SC to SC particles in lists A and B
      if (sameSign) {
        fillIndicesToUniqueIDMap(listA->getList(ParticleList::c_FlavorSpecificParticle, false),
                                 listB->getList(ParticleList::c_FlavorSpecificParticle, false), uniqueID);
        fillIndicesToUniqueIDMap(listA->getList(ParticleList::c_FlavorSpecificParticle, true),
                                 listB->getList(ParticleList::c_FlavorSpecificParticle, true),  uniqueID);
      } else {
        fillIndicesToUniqueIDMap(listA->getList(ParticleList::c_FlavorSpecificParticle, false),
                                 listB->getList(ParticleList::c_FlavorSpecificParticle, true),  uniqueID);
        fillIndicesToUniqueIDMap(listA->getList(ParticleList::c_FlavorSpecificParticle, true),
                                 listB->getList(ParticleList::c_FlavorSpecificParticle, false), uniqueID);
      }

      fillIndicesToUniqueIDMap(listA->getList(ParticleList::c_SelfConjugatedParticle),
                               listB->getList(ParticleList::c_SelfConjugatedParticle),  uniqueID);
    }

    // assign unique ids to others as well
    for (unsigned i = 0; i < m_numberOfLists; i++) {
      StoreObjPtr<ParticleList> listA =  m_plists[i];

      fillIndicesToUniqueIDMap(listA->getList(ParticleList::c_FlavorSpecificParticle, true),  uniqueID);
      fillIndicesToUniqueIDMap(listA->getList(ParticleList::c_FlavorSpecificParticle, false), uniqueID);
      fillIndicesToUniqueIDMap(listA->getList(ParticleList::c_SelfConjugatedParticle), uniqueID);
    }
  }

  void ParticleGenerator::fillIndicesToUniqueIDMap(const std::vector<int>& listA, const std::vector<int>& listB, int& uniqueID)
  {
    const Particle* A, *B;
    bool copies = false;
    for (unsigned i = 0; i < listA.size(); i++) {
      bool aIsAlreadyIn = m_indicesToUniqueIDs.count(listA[i]) ? true : false;

      if (not aIsAlreadyIn)
        m_indicesToUniqueIDs[ listA[i] ] = uniqueID++;

      for (unsigned j = 0; j < listB.size(); j++) {
        bool bIsAlreadyIn = m_indicesToUniqueIDs.count(listB[j]) ? true : false;

        if (bIsAlreadyIn)
          continue;

        // are these two particles copies
        A = m_particleArray[ listA[i] ];
        B = m_particleArray[ listB[j] ];
        copies = B->isCopyOf(A);

        if (copies)
          m_indicesToUniqueIDs[ listB[j] ] = m_indicesToUniqueIDs[ listA[i] ];
      }
    }
  }


  void ParticleGenerator::fillIndicesToUniqueIDMap(const std::vector<int>& listA, int& uniqueID)
  {
    for (unsigned i = 0; i < listA.size(); i++) {
      bool aIsAlreadyIn = m_indicesToUniqueIDs.count(listA[i]) ? true : false;

      if (not aIsAlreadyIn)
        m_indicesToUniqueIDs[ listA[i] ] = uniqueID++;
    }
  }


  bool ParticleGenerator::loadNext()
  {

    bool loadedNext = false;
    while (true) {
      switch (m_iParticleType) {
        case 0: loadedNext = loadNextParticle(false); break; //Particles
        case 1: loadedNext = loadNextParticle(true); break; //Anti-Particles
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
    //TLorentzVector performance is quite horrible, do it ourselves.
    double px = 0;
    double py = 0;
    double pz = 0;
    double E = 0;
    for (const Particle* d : m_particles) {
      px += d->getPx();
      py += d->getPy();
      pz += d->getPz();
      E += d->getEnergy();
    }
    const TLorentzVector vec(px, py, pz, E);

    switch (m_iParticleType) {
      case 0: return Particle(vec, m_pdgCode, m_isSelfConjugated ? Particle::c_Unflavored : Particle::c_Flavored, m_indices,
                                m_particleArray.getPtr());
      case 1: return Particle(vec, -m_pdgCode, m_isSelfConjugated ? Particle::c_Unflavored : Particle::c_Flavored, m_indices,
                                m_particleArray.getPtr());
      case 2: return Particle(vec, m_pdgCode, Particle::c_Unflavored, m_indices, m_particleArray.getPtr());
      default: B2FATAL("You called getCurrentParticle although loadNext should have returned false!");
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
    std::set<int> indexSet;
    if (not m_inputListsCollide)
      indexSet.insert(m_indices.begin(), m_indices.end());
    else
      for (unsigned int i = 0; i < m_numberOfLists; i++)
        indexSet.insert(m_indicesToUniqueIDs.at(m_indices[i]));

    bool elementInserted = m_usedCombinations.insert(indexSet).second;
    return elementInserted;
  }

  bool ParticleGenerator::inputListsCollide(const std::pair<unsigned, unsigned>& pair) const
  {
    for (unsigned i = 0; i < m_collidingLists.size(); i++)
      if (pair == m_collidingLists[i])
        return true;

    return false;
  }

  int ParticleGenerator::getUniqueID(int index) const
  {
    if (not m_inputListsCollide)
      return 0;

    if (not m_indicesToUniqueIDs.count(index))
      return -1;

    return m_indicesToUniqueIDs.at(index);
  }
}
