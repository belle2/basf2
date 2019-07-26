/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor : Soumen Halder and Saurabh Sandilya                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/FSRCorrection/BelleBremRecoveryModule.h>
#include <analysis/utility/ParticleCopy.h>
// framework aux
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/RelationArray.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/Track.h>

// utilities
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <cmath>
#include <algorithm>
#include <TMatrixFSym.h>

#include <map>
#include <vector>
using namespace std;

namespace Belle2 {

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

  REG_MODULE(BelleBremRecovery)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

  BelleBremRecoveryModule::BelleBremRecoveryModule() :
    Module(), m_pdgCode(0)

  {
    // set module description (e.g. insert text)
    setDescription("Takes the charged particle from the given charged particle list and copies them to the output list and adds the 4-vector of  all the photons (considered as radiative) to the charged particle, if the given criteria for maximum angle and minimum energy are fulfilled.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("inputListName", m_inputListName,
             "The initial charged particle list containing the charged particles to correct, should already exists.");
    addParam("outputListName", m_outputListName, "The output charged particle list containing the corrected charged particles.");
    addParam("gammaListName", m_gammaListName, "The gammas list containing possibly radiative gammas, should already exist.");
    addParam("angleThreshold", m_angleThres,
             "The maximum angle in radian  between the charged particle  and the (radiative) gamma to be accepted.", 0.05);
    addParam("minimumEnergy", m_minimumEnergy, "The minimum energy of the (radiative) gamma to be accepted.", 0.05);
    addParam("multiplePhotons", m_isMultiPho, "If only the nearest photon to add then make it False otherwise true", true);
    addParam("writeOut", m_writeOut,
             "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);

  }

  void BelleBremRecoveryModule::initialize()
  {
    // check the validity of output ParticleList name
    bool valid = m_decaydescriptor.init(m_outputListName);
    if (!valid)
      B2ERROR("[BelleBremRecoveryModule] Invalid output ParticleList name: " << m_outputListName);

    // output particle
    const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();
    m_pdgCode  = mother->getPDGCode();
    m_outputAntiListName = ParticleListName::antiParticleListName(m_outputListName);

    // get exiting particle lists
    if (m_inputListName == m_outputListName) {
      B2ERROR("[BelleBremRecoveryModule] Input and output particle list names are the same: " << m_inputListName);
    } else if (!m_decaydescriptor.init(m_inputListName)) {
      B2ERROR("[BelleBremRecoveryModule] Invalid input particle list name: " << m_inputListName);
    } else {
      m_inputparticleList.isRequired(m_inputListName);
    }

    if (!m_decaydescriptorGamma.init(m_gammaListName)) {
      B2ERROR("[BelleBremRecoveryModule] Invalid gamma particle list name: " << m_gammaListName);
    } else {
      m_gammaList.isRequired(m_gammaListName);
    }

    // make output list
    DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
    m_outputparticleList.registerInDataStore(m_outputListName, flags);
    m_outputAntiparticleList.registerInDataStore(m_outputAntiListName, flags);

    StoreArray<Particle> particles;
    StoreArray<PIDLikelihood> pidlikelihoods;
    particles.registerRelationTo(pidlikelihoods);
  }


  void BelleBremRecoveryModule::event()
  {
    StoreArray<Particle> particles;
    StoreArray<MCParticle> mcParticles;

    RelationArray particlesToMCParticles(particles, mcParticles);

    // new output particle list
    m_outputparticleList.create();
    m_outputparticleList->initialize(m_pdgCode, m_outputListName);

    m_outputAntiparticleList.create();
    m_outputAntiparticleList->initialize(-1 * m_pdgCode, m_outputAntiListName);
    m_outputAntiparticleList->bindAntiParticleList(*(m_outputparticleList));

    std::unordered_map<int, int> usedGammas;
    // loop over charged particles, correct them and add them to the output list
    const unsigned int nLep = m_inputparticleList->getListSize();
    for (unsigned i = 0; i < nLep; i++) {
      const Particle* lepton = m_inputparticleList->getParticle(i);
      TLorentzVector lepton4Vector = lepton->get4Vector();
      TLorentzVector new4Vec = lepton->get4Vector();
      std::vector<Particle*> selectedGammas;
      // look for all possible (radiative) gamma
      const unsigned int nGam = m_gammaList->getListSize();
      for (unsigned j = 0; j < nGam; j++) {
        Particle* gamma = m_gammaList->getParticle(j);
        // check if gamma energy is within allowed energy range
        if (gamma->getEnergy() < m_minimumEnergy) continue;
        //This will prevent double counting of bremphoton
        bool NotgammaUsed = usedGammas.find(gamma->getMdstArrayIndex()) == usedGammas.end();
        if (!NotgammaUsed) continue;
        // get angle (in lab system)
        TVector3 pi = lepton->getMomentum();
        TVector3 pj = gamma->getMomentum();
        double angle = (pi.Angle(pj));
        if (m_angleThres > angle) {
          gamma->addExtraInfo("theta_e_gamma", angle);
          selectedGammas.push_back(gamma);
        }
      }
      //sorting the bremphotons in assending order of the angle with the charged particle
      std::sort(selectedGammas.begin(), selectedGammas.end(), [](const Particle * photon1, const Particle * photon2) {
        return photon1->getExtraInfo("theta_e_gamma") < photon2->getExtraInfo("theta_e_gamma");
      });

      //How many brem photons to add (nearest or all)
      if (!m_isMultiPho && selectedGammas.size() > 1) {
        std::for_each(selectedGammas.begin() + 1, selectedGammas.end(), [](const auto & fsrgamma) {
          fsrgamma->removeExtraInfo();
        });
        selectedGammas.erase(selectedGammas.begin() + 1, selectedGammas.end());
      }
      //Preparing 4-momentum vector of charged particle by adding bremphoton momenta
      for (auto const& fsrgamma : selectedGammas)
        new4Vec += fsrgamma->get4Vector();
      Particle correctedLepton(new4Vec, lepton->getPDGCode(), Particle::EFlavorType::c_Flavored, Particle::c_Track,
                               lepton->getTrack()->getArrayIndex());
      correctedLepton.appendDaughter(lepton, false);

      const TMatrixFSym& lepErrorMatrix = lepton->getMomentumVertexErrorMatrix();
      TMatrixFSym corLepMatrix(c_DimMatrix);
      for (auto const& fsrgamma : selectedGammas) {
        usedGammas[fsrgamma->getMdstArrayIndex()] = fsrgamma->getMdstArrayIndex();
        //this extrainfo associated to the brephoton provides angle with the charged particle
        const TMatrixFSym& fsrErrorMatrix = fsrgamma->getMomentumVertexErrorMatrix();
        for (int irow = 0; irow < c_DimMatrix; irow++)
          for (int icol = irow; icol < c_DimMatrix; icol++) {
            if (irow > 3 || icol > 3)
              corLepMatrix(irow, icol) = lepErrorMatrix(irow, icol);
            else
              corLepMatrix(irow, icol) = lepErrorMatrix(irow, icol) + fsrErrorMatrix(irow, icol);
          }
        correctedLepton.appendDaughter(fsrgamma, false);
        B2INFO("[BelleBremRecoveryModule] Found a radiative gamma and added its 4-vector to the charge particle");
        correctedLepton.setMomentumVertexErrorMatrix(corLepMatrix);
      }
      if (int(selectedGammas.size()) == 0)
        correctedLepton.setMomentumVertexErrorMatrix(lepton->getMomentumVertexErrorMatrix());
      correctedLepton.setVertex(lepton->getVertex());
      correctedLepton.setPValue(lepton->getPValue());
      // add the mc relation
      Particle* newLepton = particles.appendNew(correctedLepton);
      const MCParticle* mcLepton = lepton->getRelated<MCParticle>();
      const PIDLikelihood* pid = lepton->getPIDLikelihood();
      if (pid)
        newLepton->addRelationTo(pid);
      if (mcLepton != nullptr) newLepton->addRelationTo(mcLepton);
      m_outputparticleList->addParticle(newLepton);
    }
  }
} // end Belle2 namespace

