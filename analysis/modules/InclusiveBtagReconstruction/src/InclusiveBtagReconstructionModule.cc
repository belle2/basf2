/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/InclusiveBtagReconstruction/InclusiveBtagReconstructionModule.h>

#include <analysis/DecayDescriptor/ParticleListName.h>

#include <unordered_set>
#include <map>
#include <vector>
#include <TLorentzVector.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(InclusiveBtagReconstruction)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

InclusiveBtagReconstructionModule::InclusiveBtagReconstructionModule() : Module()
{
  // Set module properties
  setDescription("Inclusive Btag reconstruction");

  // Parameter definitions
  addParam("upsilonListName", m_upsilonListName, "Name of the ParticleList to be filled with Upsilon(4S) -> B:sig anti-B:tag",
           std::string("Upsilon(4S)"));
  addParam("bsigListName", m_bsigListName, "Name of the Bsig ParticleList", std::string(""));
  addParam("btagListName", m_btagListName, "Name of the Btag ParticleList", std::string(""));
  addParam("inputListsNames", m_inputListsNames, "List of names of the ParticleLists which are used to reconstruct Btag from");
}

InclusiveBtagReconstructionModule::~InclusiveBtagReconstructionModule() = default;

void InclusiveBtagReconstructionModule::initialize()
{
  m_bsigList.isRequired(m_bsigListName);

  for (const std::string& inputListName : m_inputListsNames) {
    StoreObjPtr<ParticleList> inputList(inputListName);
    inputList.isRequired();
  }

  m_btagList.registerInDataStore(m_btagListName);
  m_antiBtagList.registerInDataStore(ParticleListName::antiParticleListName(m_btagListName));
  m_upsilonList.registerInDataStore(m_upsilonListName);
}

void InclusiveBtagReconstructionModule::event()
{
  bool valid = m_decaydescriptor.init(m_bsigListName);
  if (!valid)
    B2ERROR("Invalid Bsig list name: " << m_bsigListName);

  const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();
  int pdgCode  = mother->getPDGCode();

  m_btagList.create();
  m_btagList->initialize(-pdgCode, m_btagList.getName());

  m_antiBtagList.create();
  m_antiBtagList->initialize(pdgCode, m_antiBtagList.getName());
  m_btagList->bindAntiParticleList(*m_antiBtagList);

  m_upsilonList.create();
  m_upsilonList->initialize(300553, m_upsilonList.getName());

  const unsigned int n = m_bsigList->getListSize();
  for (unsigned i = 0; i < n; i++) { // find Btag(s) for each Bsig
    const Particle* bsig = m_bsigList->getParticle(i);
    const std::vector<const Particle*>& bsigFinalStateDaughters = bsig->getFinalStateDaughters();
    std::unordered_set<int> mdstSourcesOfBsigFinalStateDaughters;
    std::map<int, std::vector<int>> btagDaughtersMap;

    for (const Particle* daughter : bsigFinalStateDaughters) {
      mdstSourcesOfBsigFinalStateDaughters.insert(daughter->getMdstSource());
    }
    auto mdstSourcesEnd = mdstSourcesOfBsigFinalStateDaughters.end();

    // make a map of Btag daughters
    for (const std::string& inputListName : m_inputListsNames) {
      StoreObjPtr<ParticleList> inputList(inputListName);
      const unsigned int m = inputList->getListSize();
      for (unsigned j = 0; j < m; j++) {
        const Particle* particle = inputList->getParticle(j);
        const std::vector<const Particle*>& particleFinalStateDaughters = particle->getFinalStateDaughters();

        // check if particle shares something with bsig...
        bool append = true;
        for (const Particle* daughter : particleFinalStateDaughters) {
          int mdstSource = daughter->getMdstSource();
          if (mdstSourcesOfBsigFinalStateDaughters.find(mdstSource) != mdstSourcesEnd) {
            append = false;
            break;
          }
          if (append) {
            auto it = btagDaughtersMap.find(mdstSource);
            if (it != btagDaughtersMap.end()) { // check for mdstSource overlaps
              it->second.push_back(particle->getArrayIndex());
            } else {
              btagDaughtersMap[mdstSource] = {particle->getArrayIndex()};
            }
          }
        }
      }
    }

    // combine map entries to form Btag candidates
    Map2Vector map2vector;
    std::vector<std::vector<int>> btagCandidates;
    map2vector.convert(btagDaughtersMap, btagCandidates);

    for (std::vector<int> daughterIndices : btagCandidates) {
      std::map<int, size_t> nonFinalStateIndicesCount;
      TLorentzVector momentum;
      for (int index : daughterIndices) {
        // check if there are non-final-state particles. If yes, the momentum should be added just once.
        if ((m_particles[index]->getFinalStateDaughters()).size() > 1) {
          auto it = nonFinalStateIndicesCount.find(index);
          if (it != nonFinalStateIndicesCount.end()) {
            nonFinalStateIndicesCount[index]++;
            continue;
          } else {
            nonFinalStateIndicesCount[index] = 1;
          }
        }
        momentum += m_particles[index]->get4Vector();
      }
      // check the number of the daughters to make sure that the not-final-state particles are not mixed with the other particles that come from the same mdstSource
      bool rightDaughtersCount = true;
      for (auto& it : nonFinalStateIndicesCount) {
        if (it.second != (m_particles[(it.first)]->getFinalStateDaughters()).size()) {
          rightDaughtersCount = false;
          break;
        }
      }
      if (rightDaughtersCount == false) {
        continue;
      }

      //remove repeated index in daughterIndices
      std::vector<int>::iterator it;
      std::sort(daughterIndices.begin(), daughterIndices.end());
      it = std::unique(daughterIndices.begin(), daughterIndices.end());
      daughterIndices.resize(std::distance(daughterIndices.begin(), it));

      Particle btagCandidate(momentum, -1 * bsig->getPDGCode(), bsig->getFlavorType(), daughterIndices, bsig->getArrayPointer());
      Particle* btag = m_particles.appendNew(btagCandidate);
      m_btagList->addParticle(btag);

      Particle upsilon(momentum + bsig->get4Vector(), 300553, Particle::c_Unflavored, { bsig->getArrayIndex(), btag->getArrayIndex() },
                       bsig->getArrayPointer());
      m_upsilonList->addParticle(m_particles.appendNew(upsilon));
    }
  }
}

void Map2Vector::convert(std::map<int, std::vector<int>>& input, std::vector<std::vector<int>>& output)
{
  makeEntries(input.begin(), input.end(), 0, output);
}

void Map2Vector::makeEntries(std::map<int, std::vector<int>>::iterator positionOnTheMap,
                             const std::map<int, std::vector<int>>::const_iterator& end,
                             unsigned i, std::vector<std::vector<int>>& output)
{
  if (positionOnTheMap == end) {
    output.push_back(m_combination);
  } else {
    std::vector<int>& v = positionOnTheMap->second;
    ++positionOnTheMap;
    for (int k : v) {
      if (i < m_combination.size()) m_combination[i] = k;
      else m_combination.push_back(k);
      makeEntries(positionOnTheMap, end, i + 1, output);
    }
  }
};
