/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/modules/KlongDecayReconstructor/KlongMomentumCalculatorExpertModule.h>

// framework aux
#include <framework/logging/Logger.h>

#include <framework/datastore/StoreArray.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>

// decay descriptor
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>

// particle combiner
#include <analysis/ParticleCombiner/ParticleCombiner.h>

// utilities
#include <analysis/utility/ParticleCopy.h>
#include <analysis/utility/KlongCalculatorUtils.h>

#include <Math/Vector4D.h>

#include <memory>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(KlongMomentumCalculatorExpert);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

KlongMomentumCalculatorExpertModule::KlongMomentumCalculatorExpertModule() :
  Module()

{
  // set module description (e.g. insert text)
  setDescription(R"DOC(
This module is used to employ kinematic constraints to determine the momentum of Klongs for two
body decays containing a K_L0 and something else. The module creates a list of K_L0 candidates whose K_L0 momentum is
reconstructed by combining the reconstructed direction of the K_L0 (either from the ECL or the KLM cluster) and
kinematic constraints of the initial state.
)DOC");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Add parameters
  addParam("decayString", m_decayString,
           "Input DecayDescriptor string.");
  addParam("maximumNumberOfCandidates", m_maximumNumberOfCandidates,
           "Don't reconstruct channel if more candidates than given are produced.", 10000);
  addParam("writeOut", m_writeOut,
           "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);
  addParam("recoList", m_recoList,
           "Suffix attached to the output K_L list, if not defined it is set to '_reco' \n", std::string("_reco"));
}

void KlongMomentumCalculatorExpertModule::initialize()
{
  StoreArray<Particle>().isRequired();

  // obtain the input and output particle lists from the decay string
  bool valid = m_decaydescriptor.init(m_decayString);
  if (!valid)
    B2ERROR("Invalid input DecayString: " << m_decayString);

  m_klistName = m_recoList;

  // Daughters
  bool k_check = false;
  int nProducts = m_decaydescriptor.getNDaughters();
  if (nProducts !=2)
    B2FATAL("The module currently only works for final states with exactly two particles!");

  for (int i = 0; i < nProducts; ++i) {
    const DecayDescriptorParticle* daughter =
      m_decaydescriptor.getDaughter(i)->getMother();
    StoreObjPtr<ParticleList>().isRequired(daughter->getFullName());
    if (daughter->getPDGCode() == Const::Klong.getPDGCode()) {
      if (k_check)
        B2FATAL("More than one K_L is detected! This module accepts only one K_L in the final state.");

      m_klistName = daughter->getFullName() + m_klistName;
      k_check = true;
    }
  }

  if (!k_check)
    B2FATAL("This module is meant to reconstruct decays with a K_L0 in the final state. There is no K_L0 in this decay!");

  m_generator = std::make_unique<ParticleGenerator>(m_decayString);

  DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
  m_koutputList.registerInDataStore(m_klistName, flags);
}

void KlongMomentumCalculatorExpertModule::event()
{
  m_koutputList.create();
  m_koutputList->initialize(Const::Klong.getPDGCode(), m_klistName);

  m_generator->init();

  int numberOfCandidates = 0;
  while (m_generator->loadNext()) {

    Particle particle = m_generator->getCurrentParticle();
    const double motherMass = particle.getPDGMass();
    const std::vector<Particle*> daughters = particle.getDaughters();

    ROOT::Math::PxPyPzEVector MotherMomentum;
    ROOT::Math::PxPyPzEVector KMomentum;
    int idx = 0;
    bool is_physical = KlongCalculatorUtils::calculateBtoKlongX(MotherMomentum, KMomentum, daughters, motherMass, idx);

    if (!is_physical)
      continue;

    numberOfCandidates++;

    if (m_maximumNumberOfCandidates > 0 and numberOfCandidates > m_maximumNumberOfCandidates) {
      m_koutputList->clear();
      break;
    }

    for (auto daughter : daughters) {
      if (daughter->getPDGCode() == Const::Klong.getPDGCode()) {
        Particle* kparticle = ParticleCopy::copyParticle(daughter);
        kparticle->set4Vector(KMomentum);
        m_koutputList->addParticle(kparticle);
        kparticle->addExtraInfo("permID", idx);
        break;
      }
    }
  } //while
} //event
