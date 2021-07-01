/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/ParticleCombiner/ParticleCombiner.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>

#include <framework/logging/Logger.h>

#include <mdst/dataobjects/ECLCluster.h>

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

      // TF SC this does not yet account for double counting so will produce:
      // { 000, 100, 200, ... 010, 110, .... } even if the first and second
      // place are the same particle list
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

  ParticleGenerator::ParticleGenerator(const std::string& decayString, const std::string& cutParameter) : m_iParticleType(0),
    m_listIndexGenerator(),
    m_particleIndexGenerator()
  {

    DecayDescriptor decaydescriptor;
    bool valid = decaydescriptor.init(decayString);
    if (!valid)
      B2ERROR("Invalid input DecayString: " << decayString);

    m_properties = decaydescriptor.getProperty();

    // Mother particle
    const DecayDescriptorParticle* mother = decaydescriptor.getMother();
    m_pdgCode = mother->getPDGCode();
    m_properties |= mother->getProperty();

    // Daughters
    m_numberOfLists = decaydescriptor.getNDaughters();
    for (unsigned int i = 0; i < m_numberOfLists; ++i) {
      // Get the mother of the subdecaystring of the ith daughter
      // eg. "B -> [D -> K pi] [tau -> pi pi pi]". The 0th daughter is the
      // *decaystring* D -> K pi whose mother is the D.
      const DecayDescriptorParticle* daughter = decaydescriptor.getDaughter(i)->getMother();
      StoreObjPtr<ParticleList> list(daughter->getFullName());
      m_plists.push_back(list);

      int daughterProperty = daughter->getProperty();
      m_daughterProperties.push_back(daughterProperty);
    }

    m_cut = Variable::Cut::compile(cutParameter);

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
          m_collidingLists.emplace_back(i, j);
        }
      }
    }

  }


  ParticleGenerator::ParticleGenerator(const DecayDescriptor& decaydescriptor, const std::string& cutParameter) : m_iParticleType(0),
    m_listIndexGenerator(),
    m_particleIndexGenerator()
  {
    bool valid = decaydescriptor.isInitOK();
    if (!valid)
      B2ERROR("Given decaydescriptor failed to initialized");

    m_properties = decaydescriptor.getProperty();

    // Mother particle
    const DecayDescriptorParticle* mother = decaydescriptor.getMother();
    m_pdgCode = mother->getPDGCode();
    m_properties |= mother->getProperty();

    // Daughters
    m_numberOfLists = decaydescriptor.getNDaughters();
    for (unsigned int i = 0; i < m_numberOfLists; ++i) {
      // Get the mother of the subdecaystring of the ith daughter
      // eg. "B -> [D -> K pi] [tau -> pi pi pi]". The 0th daughter is the
      // *decaystring* D -> K pi whose mother is the D.
      const DecayDescriptorParticle* daughter = decaydescriptor.getDaughter(i)->getMother();
      StoreObjPtr<ParticleList> list(daughter->getFullName());
      m_plists.push_back(list);

      int daughterProperty = daughter->getProperty();
      m_daughterProperties.push_back(daughterProperty);
    }

    m_cut = Variable::Cut::compile(cutParameter);

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
          m_collidingLists.emplace_back(i, j);
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

    for (auto& collidingList : m_collidingLists) {
      StoreObjPtr<ParticleList> listA =  m_plists[collidingList.first];
      StoreObjPtr<ParticleList> listB =  m_plists[collidingList.second];

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
    for (int i : listA) {
      bool aIsAlreadyIn = m_indicesToUniqueIDs.count(i) ? true : false;

      if (not aIsAlreadyIn)
        m_indicesToUniqueIDs[ i ] = uniqueID++;

      for (int j : listB) {
        bool bIsAlreadyIn = m_indicesToUniqueIDs.count(j) ? true : false;

        if (bIsAlreadyIn)
          continue;

        // are these two particles copies
        A = m_particleArray[ i ];
        B = m_particleArray[ j ];
        copies = B->isCopyOf(A);

        if (copies)
          m_indicesToUniqueIDs[ j ] = m_indicesToUniqueIDs[ i ];
      }
    }
  }


  void ParticleGenerator::fillIndicesToUniqueIDMap(const std::vector<int>& listA, int& uniqueID)
  {
    for (int i : listA) {
      bool aIsAlreadyIn = m_indicesToUniqueIDs.count(i) ? true : false;

      if (not aIsAlreadyIn)
        m_indicesToUniqueIDs[ i ] = uniqueID++;
    }
  }


  bool ParticleGenerator::loadNext(bool loadAntiParticle)
  {

    bool loadedNext = false;
    /**
     * Three cases are distinguished:
     * First, particles matching the flavor specified in the decay string are used to form combinations.
     * Secondly, the anti-particles of flavored particles are used, but only if requested.
     * Lastly, self-conjugated particles are handled specifically.
     */
    while (true) {
      if (m_iParticleType == 0) {
        loadedNext = loadNextParticle(false);
      } else if (m_iParticleType == 1 and loadAntiParticle) {
        loadedNext = loadNextParticle(true);
      } else if (m_iParticleType == 2) {
        loadedNext = loadNextSelfConjugatedParticle();
      } else {
        return false;
      }

      if (loadedNext) return true;
      else ++m_iParticleType;

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

        m_current_particle = createCurrentParticle();
        if (!m_cut->check(&m_current_particle))
          continue;

        if (not currentCombinationIsUnique()) continue;

        if (not currentCombinationIsECLCRUnique()) continue;

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

        m_current_particle = createCurrentParticle();
        if (!m_cut->check(&m_current_particle))
          continue;

        if (not currentCombinationIsUnique()) continue;

        if (not currentCombinationIsECLCRUnique()) continue;

        return true;
      }

      return false;
    }

  }

  Particle ParticleGenerator::createCurrentParticle() const
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
                                m_properties, m_daughterProperties,
                                m_particleArray.getPtr());
      case 1: return Particle(vec, -m_pdgCode, m_isSelfConjugated ? Particle::c_Unflavored : Particle::c_Flavored, m_indices,
                                m_properties, m_daughterProperties,
                                m_particleArray.getPtr());
      case 2: return Particle(vec, m_pdgCode, Particle::c_Unflavored, m_indices,
                                m_properties, m_daughterProperties,
                                m_particleArray.getPtr());
      default: B2FATAL("You called getCurrentParticle although loadNext should have returned false!");
    }

    return Particle(); // This should never happen
  }

  Particle ParticleGenerator::getCurrentParticle() const
  {
    return m_current_particle;
  }

  bool ParticleGenerator::currentCombinationHasDifferentSources()
  {
    std::vector<Particle*> stack = m_particles;
    static std::vector<int> sources; // stack for particle sources
    sources.clear();

    // recursively check all daughters and daughters of daughters
    while (!stack.empty()) {
      Particle* p = stack.back();
      stack.pop_back();
      const std::vector<int>& daughters = p->getDaughterIndices();

      if (daughters.empty()) {
        int source = p->getMdstSource();
        for (int i : sources) {
          if (source == i) return false;
        }
        sources.push_back(source);
      } else {
        for (int daughter : daughters) stack.push_back(m_particleArray[daughter]);
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
    for (const auto& collidingList : m_collidingLists)
      if (pair == collidingList)
        return true;

    return false;
  }

  bool ParticleGenerator::currentCombinationIsECLCRUnique()
  {
    unsigned nECLSource = 0;
    std::vector<Particle*> stack = m_particles;
    static std::vector<int> connectedregions;
    static std::vector<ECLCluster::EHypothesisBit> hypotheses;
    connectedregions.clear();
    hypotheses.clear();

    // recursively check all daughters and daughters of daughters
    while (!stack.empty()) {
      Particle* p = stack.back();
      stack.pop_back();
      const std::vector<int>& daughters = p->getDaughterIndices();

      if (daughters.empty()) {
        // Only test if the particle was created from an ECLCluster at source.
        // This CAN CHANGE if we change the cluster <--> track matching,
        // (currently we match nPhotons clusters to all track type particles,
        // i.e. electrons, pions, kaons etc. We might gain by matching
        // neutralHadron hypothesis clusters to kaons, for example).
        //
        // In the above case one would need to extend the check to ALL
        // particles with an associated ECLCluster. Then replace the following
        // active line with two lines...
        //
        // auto cluster = p->getECLCluster();
        // if (cluster) { // then do stuff
        if (p->getParticleSource() == Particle::EParticleSourceObject::c_ECLCluster) {
          nECLSource++;
          auto* cluster = p->getECLCluster();
          connectedregions.push_back(cluster->getConnectedRegionId());
          hypotheses.push_back(p->getECLClusterEHypothesisBit());
        }
      } else {
        for (int daughter : daughters) stack.push_back(m_particleArray[daughter]);
      }
    }

    // less than two particles from an ECL source is fine
    // (unless cluster <--> track matching changes)
    if (nECLSource < 2) return true;

    // yes this is a nested for loop but it's fast, we promise
    for (unsigned icr = 0; icr < connectedregions.size(); ++icr)
      for (unsigned jcr = icr + 1; jcr < connectedregions.size(); ++jcr)
        if (connectedregions[icr] == connectedregions[jcr])
          if (hypotheses[icr] != hypotheses[jcr]) return false;

    return true;
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
