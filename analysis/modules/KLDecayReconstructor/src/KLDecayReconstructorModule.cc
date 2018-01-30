/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Benjamin Oberhof                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

// Own include
#include <analysis/modules/KLDecayReconstructor/KLDecayReconstructorModule.h>

// framework aux
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>

// decay descriptor
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>

// utilities
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <analysis/utility/ParticleCopy.h>

using namespace std;

namespace Belle2 {

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

  REG_MODULE(KLDecayReconstructor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

  KLDecayReconstructorModule::KLDecayReconstructorModule() :
    Module()

  {
    // set module description (e.g. insert text)
    setDescription("This module is used to reconstruct B momentum for two body decays in a K_L0 and something else. The K_L0 momentum is reconstructed by taking reconstructed direction (ECL or KLM) and kinematic constraints.");
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
             "Suffix attached to the original K_L input list to identify the output list of the FindKLMomentum module; this is the input for this module, if not defined it is set to '_reco' \n",
             std::string("_reco"));

    // initializing the rest of private memebers
    m_pdgCode   = 0;
    m_isSelfConjugatedParticle = 0;
    m_generator = 0;
  }

  void KLDecayReconstructorModule::initialize()
  {
    // clear everything
    m_pdgCode = 0;
    m_listName = "";

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

    bool k_check = 0;

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
        k_check = 1;
      }
    }

    if (!k_check)
      B2FATAL("This module is meant to reconstruct decays with a K_L0 in the final state. There is no K_L0 in this decay!");
    newDecayString = newDecayString + kListName;

    m_generator = std::unique_ptr<ParticleGenerator>(new ParticleGenerator(newDecayString, m_cutParameter));

    StoreObjPtr<ParticleList> particleList(m_listName);
    DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
    particleList.registerInDataStore(flags);
    if (!m_isSelfConjugatedParticle) {
      StoreObjPtr<ParticleList> antiParticleList(m_antiListName);
      antiParticleList.registerInDataStore(flags);
    }

    m_cut = Variable::Cut::compile(m_cutParameter);

  }

  void KLDecayReconstructorModule::event()
  {
    StoreArray<Particle> particles;

    StoreObjPtr<ParticleList> outputList(m_listName);
    outputList.create();
    outputList->initialize(m_pdgCode, m_listName);

    if (!m_isSelfConjugatedParticle) {
      StoreObjPtr<ParticleList> outputAntiList(m_antiListName);
      outputAntiList.create();
      outputAntiList->initialize(-1 * m_pdgCode, m_antiListName);

      outputList->bindAntiParticleList(*(outputAntiList));
    }

    m_generator->init();

    int numberOfCandidates = 0;
    while (m_generator->loadNext()) {

      Particle&& particle = m_generator->getCurrentParticle();

      bool is_physical = 1;

      const std::vector<Particle*> daughters = particle.getDaughters();

      if (daughters.size() < 2)
        B2FATAL("Reconstructing particle as a daughter of a decay with less then 2 daughters!");

      if (daughters.size() > 3)
        B2FATAL("Higher multiplicity (>2) missing momentum decays not implemented yet!");

      int e_check = 0;
      TLorentzVector pDaughters;
      for (unsigned i = 0; i < daughters.size(); i++) {
        if (daughters[i]->getPDGCode() != Const::Klong.getPDGCode()) {
          pDaughters += daughters[i]->get4Vector();
          e_check = daughters[i]->getArrayIndex() + e_check * 100;
        }
      }


      TLorentzVector klDaughters;
      for (unsigned i = 0; i < daughters.size(); i++) {
        if (daughters[i]->getPDGCode() == Const::Klong.getPDGCode()) {
          klDaughters += daughters[i]->get4Vector();
          if (e_check != daughters[i]->getExtraInfo("permID")) {
            is_physical = 0;
          }
        }
      }
      double m_b = particle.getPDGMass();

      TLorentzVector mom = pDaughters + klDaughters;
      mom.SetE(TMath::Sqrt(mom.Vect().Mag2() + m_b * m_b));
      if ((!isnan(mom.Vect().Mag())) && is_physical)
        particle.set4Vector(mom);
      if (isnan(mom.Vect().Mag()))
        is_physical = 0;

      if (!m_cut->check(&particle))
        continue;

      if (!is_physical)
        continue;

      numberOfCandidates++;

      if (m_maximumNumberOfCandidates > 0 and numberOfCandidates > m_maximumNumberOfCandidates) {
        outputList->clear();
        break;
      }

      Particle* newParticle = particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      outputList->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
      newParticle->addExtraInfo("decayModeID", m_decayModeID);

    } //while

  } //event

} // end Belle2 namespace

