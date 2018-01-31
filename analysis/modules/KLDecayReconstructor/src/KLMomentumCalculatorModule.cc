/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2018 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: B.Oberhof, benjamin.oberhof@lnf.infn.it                  *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

// Own include
#include <analysis/modules/KLDecayReconstructor/KLMomentumCalculatorModule.h>

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

  REG_MODULE(KLMomentumCalculator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

  KLMomentumCalculatorModule::KLMomentumCalculatorModule() :
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
             "Suffix attached to the output K_L list, if not defined it is set to '_reco' \n", std::string("_reco"));

    // initializing the rest of private memebers
    //m_pdgCode   = 0;
    //m_isSelfConjugatedParticle = 0;
    //m_generator = 0;
  }

  void KLMomentumCalculatorModule::initialize()
  {
    // clear everything
    m_pdgCode = 0;
    m_listName = "";
    m_isSelfConjugatedParticle = 0;
    m_generator = 0;

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
    m_kpdgCode = Const::Klong.getPDGCode();

    // Daughters
    bool k_check = 0;
    int nProducts = m_decaydescriptor.getNDaughters();
    for (int i = 0; i < nProducts; ++i) {
      const DecayDescriptorParticle* daughter =
        m_decaydescriptor.getDaughter(i)->getMother();
      StoreObjPtr<ParticleList>().isRequired(daughter->getFullName());
      if (daughter->getPDGCode() == Const::Klong.getPDGCode()) {
        m_klistName = daughter->getFullName() + m_klistName;
        k_check = 1;
      }
    }

    if (!k_check)
      B2FATAL("This module is meant to reconstruct decays with a K_L0 in the final state. There is no K_L0 in this decay!");

    m_generator = std::unique_ptr<ParticleGenerator>(new ParticleGenerator(m_decayString, m_cutParameter));

    StoreObjPtr<ParticleList> KparticleList(m_klistName);
    DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
    KparticleList.registerInDataStore(flags);

    m_cut = Variable::Cut::compile(m_cutParameter);

  }

  void KLMomentumCalculatorModule::event()
  {
    StoreArray<Particle> particles;

    StoreObjPtr<ParticleList> koutputList(m_klistName);
    koutputList.create();
    koutputList->initialize(m_kpdgCode, m_klistName);

    m_generator->init();

    int numberOfCandidates = 0;
    while (m_generator->loadNext()) {

      Particle&& particle = m_generator->getCurrentParticle();

      TLorentzVector missDaughters;
      Particle* kparticle = 0;

      bool is_physical = 1;

      const std::vector<Particle*> daughters = particle.getDaughters();

      if (daughters.size() < 2)
        B2FATAL("Reconstructing particle as a daughter of a decay with less then 2 daughters!");

      if (daughters.size() > 3)
        B2FATAL("Higher multiplicity (>2) missing momentum decays not implemented yet!");

      TLorentzVector pDaughters;
      for (unsigned i = 0; i < daughters.size(); i++) {
        if (daughters[i]->getPDGCode() != Const::Klong.getPDGCode()) {
          pDaughters += daughters[i]->get4Vector();
        }
      }

      TLorentzVector klDaughters;
      for (unsigned i = 0; i < daughters.size(); i++) {
        if (daughters[i]->getPDGCode() == Const::Klong.getPDGCode()) {
          kparticle = ParticleCopy::copyParticle(daughters[i]);
          klDaughters += daughters[i]->get4Vector();
        }
      }

      double k_mag1 = 0.;
      double k_mag2 = 0.;
      double m_b = particle.getPDGMass();
      double m_k = Const::Klong.getMass();
      double m_j = 0;

      int idx = 0;

      for (unsigned i = 0; i < daughters.size(); i++) {
        if (daughters[i]->getPDGCode() != Const::Klong.getPDGCode()) {
          m_j = daughters[i]->getPDGMass();
          idx = daughters[i]->getArrayIndex() + idx * 100;
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

      if (k_mag1 > 0)
        missDaughters.SetVect(k_mag1 * (klDaughters.Vect().Unit()));
      else
        missDaughters.SetVect(k_mag2 * (klDaughters.Vect().Unit()));
      missDaughters.SetE(std::sqrt(m_k * m_k + missDaughters.Vect().Mag2()));

      for (unsigned i = 0; i < daughters.size(); i++) {
        if (daughters[i]->getPDGCode() == Const::Klong.getPDGCode()) {
          if (!isnan(missDaughters.Vect().Mag())) {
            kparticle->set4Vector(missDaughters);
          } else
            is_physical = 0;
        }
      }

      if (is_physical) {
        TLorentzVector mom = pDaughters + missDaughters;
        if (isnan(mom.Vect().Mag()))
          is_physical = 0;
      }

      if (!m_cut->check(&particle))
        continue;

      if (!is_physical)
        continue;

      numberOfCandidates++;

      if (m_maximumNumberOfCandidates > 0 and numberOfCandidates > m_maximumNumberOfCandidates) {
        koutputList->clear();
        break;
      }

      koutputList->addParticle(kparticle->getArrayIndex(), kparticle->getPDGCode(), kparticle->getFlavorType());
      kparticle->addExtraInfo("permID", idx);

    } //while

  } //event

} // end Belle2 namespace

