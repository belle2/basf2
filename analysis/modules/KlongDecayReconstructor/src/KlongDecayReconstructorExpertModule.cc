/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/modules/KlongDecayReconstructor/KlongDecayReconstructorExpertModule.h>

// framework aux
#include <framework/logging/Logger.h>

// decay descriptor
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>

// utilities
#include <analysis/DecayDescriptor/ParticleListName.h>

#include <memory>

using namespace std;

namespace Belle2 {

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

  REG_MODULE(KlongDecayReconstructorExpert)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

  KlongDecayReconstructorExpertModule::KlongDecayReconstructorExpertModule() :
    Module(), m_pdgCode(0), m_isSelfConjugatedParticle(false)

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
             "Suffix attached to the original K_L input list to identify the output list of the FindKlongMomentum module; this is the input for this module, if not defined it is set to '_reco' \n",
             std::string("_reco"));

  }

  void KlongDecayReconstructorExpertModule::initialize()
  {
    m_particles.isRequired();

    // clear everything, initialize private members
    m_listName = "";
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

    std::string newDecayString;
    std::string kListName;
    newDecayString = m_listName + " -> ";

    bool k_check = false;

    // Daughters
    int nProducts = m_decaydescriptor.getNDaughters();
    for (int i = 0; i < nProducts; ++i) {
      const DecayDescriptorParticle* daughter = m_decaydescriptor.getDaughter(i)->getMother();
      if (daughter->getPDGCode() != Const::Klong.getPDGCode()) {
        StoreObjPtr<ParticleList>().isRequired(daughter->getFullName());
        newDecayString = newDecayString + daughter->getFullName() + " ";
      } else {
        StoreObjPtr<ParticleList>().isRequired(daughter->getFullName() + m_recoList);
        kListName = daughter->getFullName() + m_recoList;
        k_check = true;
      }
    }

    if (!k_check)
      B2FATAL("This module is meant to reconstruct decays with a K_L0 in the final state. There is no K_L0 in this decay!");
    newDecayString = newDecayString + kListName;

    m_generator = std::make_unique<ParticleGenerator>(newDecayString, m_cutParameter);

    DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
    m_outputList.registerInDataStore(m_listName, flags);
    if (!m_isSelfConjugatedParticle) {
      m_outputAntiList.registerInDataStore(m_antiListName, flags);
    }

    m_cut = Variable::Cut::compile(m_cutParameter);

  }

  void KlongDecayReconstructorExpertModule::event()
  {
    m_outputList.create();
    m_outputList->initialize(m_pdgCode, m_listName);

    if (!m_isSelfConjugatedParticle) {
      m_outputAntiList.create();
      m_outputAntiList->initialize(-1 * m_pdgCode, m_antiListName);

      m_outputList->bindAntiParticleList(*(m_outputAntiList));
    }

    m_generator->init();

    int numberOfCandidates = 0;
    while (m_generator->loadNext()) {

      Particle particle = m_generator->getCurrentParticle();

      bool is_physical = true;

      const std::vector<Particle*> daughters = particle.getDaughters();

      if (daughters.size() < 2)
        B2FATAL("Reconstructing particle as a daughter of a decay with less than 2 daughters!");

      if (daughters.size() > 3)
        B2FATAL("Higher multiplicity (>2) missing momentum decays not implemented yet!");

      int e_check = 0;
      TLorentzVector pDaughters;
      for (auto daughter : daughters) {
        if (daughter->getPDGCode() != Const::Klong.getPDGCode()) {
          pDaughters += daughter->get4Vector();
          e_check = daughter->getArrayIndex() + e_check * 100;
        }
      }


      TLorentzVector klDaughters;
      for (auto daughter : daughters) {
        if (daughter->getPDGCode() == Const::Klong.getPDGCode()) {
          klDaughters += daughter->get4Vector();
          if (e_check != daughter->getExtraInfo("permID")) {
            is_physical = false;
          }
        }
      }
      double m_b = particle.getPDGMass();

      TLorentzVector mom = pDaughters + klDaughters;
      mom.SetE(TMath::Sqrt(mom.Vect().Mag2() + m_b * m_b));
      if ((!isnan(mom.Vect().Mag())) && is_physical)
        particle.set4Vector(mom);
      if (isnan(mom.Vect().Mag()))
        is_physical = false;

      if (!m_cut->check(&particle))
        continue;

      if (!is_physical)
        continue;

      numberOfCandidates++;

      if (m_maximumNumberOfCandidates > 0 and numberOfCandidates > m_maximumNumberOfCandidates) {
        m_outputList->clear();
        break;
      }

      Particle* newParticle = m_particles.appendNew(particle);

      m_outputList->addParticle(newParticle);
      newParticle->addExtraInfo("decayModeID", m_decayModeID);

    } //while

  } //event

} // end Belle2 namespace

