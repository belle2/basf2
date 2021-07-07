/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/AllParticleCombiner/AllParticleCombinerModule.h>
#include <analysis/DecayDescriptor/ParticleListName.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(AllParticleCombiner)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

AllParticleCombinerModule::AllParticleCombinerModule() : Module()
{
  // Set module properties
  setDescription(R"DOC("This module combines all particles of the provided list to one mother particle.
  )DOC");

  // Parameter definitions
  addParam("inputListNames", m_inputListNames, "List of ParticleLists which are supposed to be combined", std::vector<std::string>());
  addParam("cut", m_cutString, "Selection criteria for the output ParticleList", std::string(""));
  addParam("writeOut", m_writeOut,
           "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);
  addParam("outputListName", m_outputListName,
           "Name of the output list created by the combination of all particles in the input lists.", std::string(""));

  // initializing the rest of private members
  m_pdgCode   = 0;
  m_isSelfConjugatedParticle = 0;
}

void AllParticleCombinerModule::initialize()
{
  m_particles.isRequired();

  bool valid = m_decaydescriptor.init(m_outputListName);
  if (!valid)
    B2ERROR("Invalid output ListName: " << m_outputListName);

  // Mother particle
  const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();

  m_pdgCode = mother->getPDGCode();
  m_antiListName = ParticleListName::antiParticleListName(mother->getFullName());
  m_isSelfConjugatedParticle = (m_outputListName == m_antiListName);

  DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
  m_outputList.registerInDataStore(m_outputListName, flags);
  if (!m_isSelfConjugatedParticle) {
    m_outputAntiList.registerInDataStore(m_antiListName, flags);
  }

  m_cut = Variable::Cut::compile(m_cutString);
}

void AllParticleCombinerModule::event()
{
  m_outputList.create();
  m_outputList->initialize(m_pdgCode, m_outputListName);

  if (!m_isSelfConjugatedParticle) {
    m_outputAntiList.create();
    m_outputAntiList->initialize(-1 * m_pdgCode, m_antiListName);

    m_outputList->bindAntiParticleList(*(m_outputAntiList));
  }

  unsigned short nParticleLists = m_inputListNames.size();
  if (nParticleLists == 0)
    B2ERROR("No particle lists found for AllParticleCombinerModule.");

  double px = 0;
  double py = 0;
  double pz = 0;
  double E = 0;
  std::vector<int> daughterIndices;

  for (unsigned short iList = 0; iList < nParticleLists; ++iList) {

    StoreObjPtr<ParticleList> plist(m_inputListNames[iList]);
    for (unsigned int i = 0; i < plist->getListSize(); ++i) {
      bool addParticle = true;
      Particle* particle = plist->getParticle(i, true);
      for (auto* daughter : particle->getFinalStateDaughters()) {
        int particleArrayIndex = daughter->getArrayIndex();
        if (std::find(daughterIndices.begin(), daughterIndices.end(), particleArrayIndex) != daughterIndices.end()) {
          addParticle = false;
          break;
        }
      }
      if (addParticle) {
        for (auto* daughter : particle->getFinalStateDaughters()) {
          int particleArrayIndex = daughter->getArrayIndex();
          daughterIndices.push_back(particleArrayIndex);
        }
        px += particle->getPx();
        py += particle->getPy();
        pz += particle->getPz();
        E += particle->getEnergy();
      }
    }
  }

  const TLorentzVector vec(px, py, pz, E);

  Particle combinedParticle = Particle(vec, m_pdgCode, m_isSelfConjugatedParticle ? Particle::c_Unflavored : Particle::c_Flavored,
                                       daughterIndices, m_particles.getPtr());

  Particle* newParticle = m_particles.appendNew(combinedParticle);
  if (m_cut->check(newParticle)) {
    m_outputList->addParticle(newParticle);
  }
}
