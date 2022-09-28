/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/modules/KlongDecayReconstructor/KlongMomentumCalculatorExpertModule.h>

// framework aux
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/KLMCluster.h>

// decay descriptor
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>

// utilities
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <analysis/utility/ParticleCopy.h>

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
  Module(), m_pdgCode(0)

{
  // set module description (e.g. insert text)
  setDescription("This module is used to employ kinematic constraints to determine the momentum of Klongs for two body B decays containing a K_L0 and something else. The module creates a list of K_L0 candidates whose K_L0 momentum is reconstructed by combining the reconstructed direction (from either the ECL or KLM) of the K_L0 and kinematic constraints of the intial state.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Add parameters
  addParam("decayString", m_decayString,
           "Input DecayDescriptor string.");
  addParam("cut", m_cutParameter, "Selection criteria to be applied", std::string(""));
  addParam("maximumNumberOfCandidates", m_maximumNumberOfCandidates,
           "Don't reconstruct channel if more candidates than given are produced.", -1);
  addParam("decayMode", m_decayModeID, "User-specified decay mode identifier (saved in 'decayModeID' extra-info for each Particle)",
           0);
  addParam("writeOut", m_writeOut,
           "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);
  addParam("recoList", m_recoList,
           "Suffix attached to the output K_L list, if not defined it is set to '_reco' \n", std::string("_reco"));

}

void KlongMomentumCalculatorExpertModule::initialize()
{
  StoreArray<Particle>().isRequired();

  // clear everything, initialize private members
  m_pdgCode = 0;
  m_listName = "";
  m_isSelfConjugatedParticle = false;
  m_generator = nullptr;

  // obtain the input and output particle lists from the decay string
  bool valid = m_decaydescriptor.init(m_decayString);
  if (!valid)
    B2ERROR("Invalid input DecayString: " << m_decayString);

  // Mother particle
  const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();

  m_pdgCode = mother->getPDGCode();
  m_listName = mother->getFullName();

  m_antiListName = ParticleListName::antiParticleListName(m_listName);
  m_isSelfConjugatedParticle = (m_listName == m_antiListName);

  m_klistName = m_recoList;

  // Daughters
  bool k_check = false;
  int nProducts = m_decaydescriptor.getNDaughters();
  for (int i = 0; i < nProducts; ++i) {
    const DecayDescriptorParticle* daughter =
      m_decaydescriptor.getDaughter(i)->getMother();
    StoreObjPtr<ParticleList>().isRequired(daughter->getFullName());
    if (daughter->getPDGCode() == Const::Klong.getPDGCode()) {
      m_klistName = daughter->getFullName() + m_klistName;
      k_check = true;
    }
  }

  if (!k_check)
    B2FATAL("This module is meant to reconstruct decays with a K_L0 in the final state. There is no K_L0 in this decay!");

  m_generator = std::make_unique<ParticleGenerator>(m_decayString, m_cutParameter);

  DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
  m_koutputList.registerInDataStore(m_klistName, flags);

  m_cut = Variable::Cut::compile(m_cutParameter);

}


void KlongMomentumCalculatorExpertModule::event()
{
  m_koutputList.create();
  m_koutputList->initialize(Const::Klong.getPDGCode(), m_klistName);

  m_generator->init();

  int numberOfCandidates = 0;
  while (m_generator->loadNext()) {

    Particle particle = m_generator->getCurrentParticle();
    double m_b = particle.getPDGMass();
    const std::vector<Particle*> daughters = particle.getDaughters();

    if (daughters.size() < 2)
      B2FATAL("Reconstructing particle as a daughter of a decay with less then 2 daughters!");

    if (daughters.size() > 3)
      B2FATAL("Higher multiplicity (>2) missing momentum decays not implemented yet!");

    ROOT::Math::PxPyPzEVector BMomentum;
    ROOT::Math::PxPyPzEVector KMomentum;
    int idx = 0;
    bool is_physical = calculateBtoKlongX(BMomentum, KMomentum, daughters, m_b, idx);

    if (!is_physical)
      continue;

    Particle* kparticle = nullptr;
    for (auto daughter : daughters) {
      if (daughter->getPDGCode() == Const::Klong.getPDGCode()) {
        kparticle = ParticleCopy::copyParticle(daughter);
        kparticle->set4Vector(KMomentum);
        break;
      }
    }
    particle.set4Vector(BMomentum);

    if (!m_cut->check(&particle))
      continue;

    numberOfCandidates++;

    if (m_maximumNumberOfCandidates > 0 and numberOfCandidates > m_maximumNumberOfCandidates) {
      m_koutputList->clear();
      break;
    }

    m_koutputList->addParticle(kparticle);
    kparticle->addExtraInfo("permID", idx);

  } //while

} //event

bool KlongMomentumCalculatorExpertModule::calculateBtoKlongX(ROOT::Math::PxPyPzEVector& BMomentum,
    ROOT::Math::PxPyPzEVector& KMomentum,
    const std::vector<Particle*> daughters,
    double m_b,
    int& idx)
{

  ROOT::Math::PxPyPzEVector pDaughters;
  for (auto daughter : daughters) {
    if (daughter->getPDGCode() != Const::Klong.getPDGCode()) {
      pDaughters += daughter->get4Vector();
    }
  }

  ROOT::Math::PxPyPzEVector klDaughters;
  for (auto daughter : daughters) {
    if (daughter->getPDGCode() == Const::Klong.getPDGCode()) {
      klDaughters += daughter->getKLMCluster()->getMomentum();
    }
  }

  double k_mag1 = 0.;
  double k_mag2 = 0.;
  double m_k = Const::Klong.getMass();
  double m_j = 0;

  for (auto daughter : daughters) {
    if (daughter->getPDGCode() != Const::Klong.getPDGCode()) {
      m_j = daughter->getPDGMass();
      idx = daughter->getArrayIndex() + idx * 100;
    }
  }

  if (daughters.size() == 3) {
    m_j = pDaughters.M();
  }

  double s_p = (klDaughters.Vect().Unit()).Dot(pDaughters.Vect());
  double m_sum = (m_b * m_b) - (m_j * m_j) - (m_k * m_k);
  double e_j = pDaughters.E();

  double s_p2 = s_p * s_p;
  double m_sum2 = m_sum * m_sum;
  double s_pm = s_p * m_sum;
  double e_j2 = e_j * e_j;
  double m_k2 = m_k * m_k;

  k_mag1 = (s_pm + std::sqrt((s_p2) * (m_sum2) - 4 * ((e_j2) - (s_p2)) * ((e_j2) * (m_k2) - (m_sum2) / 4))) / (2 * (e_j2 - s_p2));
  k_mag2 = (s_pm - std::sqrt((s_p2) * (m_sum2) - 4 * ((e_j2) - (s_p2)) * ((e_j2) * (m_k2) - (m_sum2) / 4))) / (2 * (e_j2 - s_p2));


  ROOT::Math::PxPyPzEVector missDaughters;

  if (k_mag1 > 0)
    missDaughters = k_mag1 * klDaughters / klDaughters.P();
  else
    missDaughters = k_mag2 * klDaughters / klDaughters.P();

  missDaughters.SetE(std::sqrt(m_k * m_k + missDaughters.P2()));

  if (isnan(missDaughters.P()))
    return false;

  for (auto daughter : daughters) {
    if (daughter->getPDGCode() == Const::Klong.getPDGCode()) {
      KMomentum = missDaughters;
    }
  }

  ROOT::Math::PxPyPzEVector mom = pDaughters + missDaughters;
  mom.SetE(std::sqrt(m_b * m_b + mom.P2()));
  if (isnan(mom.P()))
    return false;
  BMomentum = mom;

  return true;
}

