/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/modules/BremsCorrection/BelleBremRecoveryModule.h>
// framework aux
#include <framework/logging/Logger.h>
#include <framework/datastore/RelationArray.h>

// dataobjects
#include <mdst/dataobjects/Track.h>

// utilities
#include <analysis/DecayDescriptor/ParticleListName.h>

// variables
#include <analysis/variables/ECLVariables.h>

#include <algorithm>
#include <TMatrixFSym.h>

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
    setDescription(R"DOC(
                   Takes the charged particle from the given charged particle list (``inputListName``) and
                   copies it to the output list (``outputListName``). The 4-vector of the nearest (all) photon(s)
                   from ``gammaListName`` (considered as radiative) is added to the charged particle, if it is
                   found inside the cone around the charged particle with the given maximum angle (``angleThreshold``).
                   )DOC");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("inputListName", m_inputListName,
             "The initial charged particle list containing the charged particles to correct, should already exist.");
    addParam("outputListName", m_outputListName, "The output charged particle list containing the corrected charged particles.");
    addParam("gammaListName", m_gammaListName, "The gammas list containing possibly radiative gammas, should already exist.");
    addParam("angleThreshold", m_angleThres,
             "The maximum angle in radians between the charged particle  and the (radiative) gamma to be accepted.", 0.05);
    addParam("multiplePhotons", m_isMultiPho, "If only the nearest photon to add then make it False otherwise true", true);
    addParam("writeOut", m_writeOut,
             "Set to true if you want to write out the output list to a root file", false);

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

    // get existing particle lists
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

    m_particles.registerRelationTo(m_pidlikelihoods);
  }


  void BelleBremRecoveryModule::event()
  {
    RelationArray particlesToMCParticles(m_particles, m_mcParticles);

    // new output particle list
    m_outputparticleList.create();
    m_outputparticleList->initialize(m_pdgCode, m_outputListName);

    m_outputAntiparticleList.create();
    m_outputAntiparticleList->initialize(-1 * m_pdgCode, m_outputAntiListName);
    m_outputAntiparticleList->bindAntiParticleList(*(m_outputparticleList));

    // loop over charged particles, correct them and add them to the output list
    const unsigned int nLep = m_inputparticleList->getListSize();
    const unsigned int nGam = m_gammaList->getListSize();
    for (unsigned i = 0; i < nLep; i++) {
      const Particle* lepton = m_inputparticleList->getParticle(i);
      TLorentzVector lepton4Vector = lepton->get4Vector();
      TLorentzVector new4Vec = lepton->get4Vector();
      std::vector<Particle*> selectedGammas;
      double bremsGammaEnergySum = 0.0;
      // look for all possible (radiative) gamma
      for (unsigned j = 0; j < nGam; j++) {
        Particle* gamma = m_gammaList->getParticle(j);
        // get angle (in lab system)
        TVector3 pi = lepton->getMomentum();
        TVector3 pj = gamma->getMomentum();
        double angle = (pi.Angle(pj));
        //Instead of first-come-first-serve, serve all charged particle equally.
        // https://indico.belle2.org/event/946/contributions/4007/attachments/1946/2967/SCunliffe190827.pdf
        if (m_angleThres > angle) {
          gamma->writeExtraInfo("theta_e_gamma", angle);
          selectedGammas.push_back(gamma);
        }
      }
      //sorting the bremphotons in ascending order of the angle with the charged particle
      std::sort(selectedGammas.begin(), selectedGammas.end(), [](const Particle * photon1, const Particle * photon2) {
        return photon1->getExtraInfo("theta_e_gamma") < photon2->getExtraInfo("theta_e_gamma");
      });
      //Preparing 4-momentum vector of charged particle by adding bremphoton momenta
      for (auto const& fsrgamma : selectedGammas) {
        new4Vec += fsrgamma->get4Vector();
        bremsGammaEnergySum += Variable::eclClusterE(fsrgamma);
        if (!m_isMultiPho) break;
      }
      Particle correctedLepton(new4Vec, lepton->getPDGCode(), Particle::EFlavorType::c_Flavored, Particle::c_Track,
                               lepton->getTrack()->getArrayIndex());
      correctedLepton.appendDaughter(lepton, false);

      //Calculating matrix element of corrected charged particle
      const TMatrixFSym& lepErrorMatrix = lepton->getMomentumVertexErrorMatrix();
      TMatrixFSym corLepMatrix = lepErrorMatrix;
      for (auto const& fsrgamma : selectedGammas) {
        const TMatrixFSym& fsrErrorMatrix = fsrgamma->getMomentumVertexErrorMatrix();
        for (int irow = 0; irow <= 3 ; irow++)
          for (int icol = irow; icol <= 3; icol++)
            corLepMatrix(irow, icol) += fsrErrorMatrix(irow, icol);
        correctedLepton.appendDaughter(fsrgamma, false);
        B2DEBUG(19, "[BelleBremRecoveryModule] Found a radiative gamma and added its 4-vector to the charge particle");
        if (!m_isMultiPho) break;
      }
      correctedLepton.setMomentumVertexErrorMatrix(corLepMatrix);
      correctedLepton.setVertex(lepton->getVertex());
      correctedLepton.setPValue(lepton->getPValue());
      correctedLepton.addExtraInfo("bremsCorrected", float(selectedGammas.size() > 0));
      correctedLepton.addExtraInfo("bremsCorrectedPhotonEnergy", bremsGammaEnergySum);
      // add the mc relation
      Particle* newLepton = m_particles.appendNew(correctedLepton);
      const MCParticle* mcLepton = lepton->getRelated<MCParticle>();
      const PIDLikelihood* pid = lepton->getPIDLikelihood();
      if (pid)
        newLepton->addRelationTo(pid);
      if (mcLepton != nullptr) newLepton->addRelationTo(mcLepton);
      m_outputparticleList->addParticle(newLepton);
    }
  }
} // end Belle2 namespace

