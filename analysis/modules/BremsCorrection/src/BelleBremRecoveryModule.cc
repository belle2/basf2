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
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <Math/VectorUtil.h>
#include <TMath.h>

#include <vector>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(BelleBremRecovery);

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
  addParam("usePhotonOnlyOnce", m_usePhotonOnlyOnce,
           "If false, a photon is used for correction of the closest charged particle in the inputList. "
           "If true, a photon is allowed to be used for correction multiple times. "
           "WARNING: One cannot use a photon twice to reconstruct a composite particle. Thus, for example, if e+ and e- are corrected with a gamma, they cannot form a J/psi -> e+ e- candidate.",
           false);

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
  // new output particle list
  m_outputparticleList.create();
  m_outputparticleList->initialize(m_pdgCode, m_outputListName);

  m_outputAntiparticleList.create();
  m_outputAntiparticleList->initialize(-1 * m_pdgCode, m_outputAntiListName);
  m_outputAntiparticleList->bindAntiParticleList(*(m_outputparticleList));

  const unsigned int nLep = m_inputparticleList->getListSize();
  const unsigned int nGam = m_gammaList->getListSize();

  if (nLep == 0) return;

  const std::string extraInfoSuffix = "BelleBremRecovery_" + m_inputListName + "_" + m_gammaListName;

  std::vector<std::vector<Particle*>> selectedGammas_nLep; // only used if m_usePhotonOnlyOnce is true
  if (m_usePhotonOnlyOnce) selectedGammas_nLep.resize(nLep);

  // loop over charged particles
  for (unsigned iLep = 0; iLep < nLep; iLep++) {
    std::vector<Particle*> selectedGammas;

    const Particle* lepton = m_inputparticleList->getParticle(iLep);
    const ROOT::Math::XYZVector pLep = lepton->getMomentum();

    // look for all possible (radiative) gamma
    for (unsigned iGam = 0; iGam < nGam; iGam++) {
      Particle* gamma = m_gammaList->getParticle(iGam); // to write extraInfo
      const ROOT::Math::XYZVector pGam = gamma->getMomentum();

      const double angle = ROOT::Math::VectorUtil::Angle(pLep, pGam);

      if (angle < m_angleThres) {
        if (!m_usePhotonOnlyOnce) { // if multiple use is allowed, keep all gammas
          gamma->writeExtraInfo("angle_" + extraInfoSuffix, angle);
          gamma->writeExtraInfo("indexLep_" + extraInfoSuffix, iLep);
          selectedGammas.push_back(gamma);
        } else {
          if (gamma->hasExtraInfo("angle_" + extraInfoSuffix)) { // check if the gamma has already selected
            if (angle < gamma->getExtraInfo("angle_" + extraInfoSuffix)) {
              // this lepton is closer to the gamma than previous lepton
              int indexLep_prev = gamma->getExtraInfo("indexLep_" + extraInfoSuffix);
              selectedGammas_nLep[indexLep_prev].erase(std::remove(selectedGammas_nLep[indexLep_prev].begin(),
                                                                   selectedGammas_nLep[indexLep_prev].end(),
                                                                   gamma),
                                                       selectedGammas_nLep[indexLep_prev].end());
              gamma->writeExtraInfo("angle_" + extraInfoSuffix, angle);
              gamma->writeExtraInfo("indexLep_" + extraInfoSuffix, iLep);
              selectedGammas.push_back(gamma);
            }
          } else {
            gamma->writeExtraInfo("angle_" + extraInfoSuffix, angle);
            gamma->writeExtraInfo("indexLep_" + extraInfoSuffix, iLep);
            selectedGammas.push_back(gamma);
          }
        }
      }
    }

    // sorting the bremphotons in ascending order of the angle with the charged particle
    std::sort(selectedGammas.begin(), selectedGammas.end(),
    [extraInfoSuffix](const Particle * photon1, const Particle * photon2) {
      return photon1->getExtraInfo("angle_" + extraInfoSuffix) < photon2->getExtraInfo("angle_" + extraInfoSuffix);
    });

    if (!m_usePhotonOnlyOnce)
      correctLepton(lepton, selectedGammas);
    else
      selectedGammas_nLep[iLep] = selectedGammas;

  }

  if (m_usePhotonOnlyOnce) {
    for (unsigned iLep = 0; iLep < nLep; iLep++) {
      const Particle* lepton = m_inputparticleList->getParticle(iLep);
      correctLepton(lepton, selectedGammas_nLep[iLep]);
    }
  }

}

void BelleBremRecoveryModule::correctLepton(const Particle* lepton, std::vector<Particle*> selectedGammas)
{
  // Updates 4-vector and errorMatrix
  ROOT::Math::PxPyPzEVector new4Vec = lepton->get4Vector();
  const TMatrixFSym& lepErrorMatrix = lepton->getMomentumVertexErrorMatrix();
  TMatrixFSym corLepMatrix = lepErrorMatrix;

  // Sum of energy of correction gammas for the extraInfo
  double bremsGammaEnergySum = 0.0;

  // Create a correctedLepton. 4-vector will be updated
  Particle correctedLepton(new4Vec, lepton->getPDGCode(), Particle::EFlavorType::c_Flavored, Particle::c_Track,
                           lepton->getTrack()->getArrayIndex());

  correctedLepton.setVertex(lepton->getVertex());
  correctedLepton.setPValue(lepton->getPValue());

  correctedLepton.appendDaughter(lepton, false);

  for (auto gamma : selectedGammas) {

    new4Vec += gamma->get4Vector();
    bremsGammaEnergySum += Variable::eclClusterE(gamma);

    const TMatrixFSym& fsrErrorMatrix = gamma->getMomentumVertexErrorMatrix();
    for (int irow = 0; irow <= 3 ; irow++)
      for (int icol = irow; icol <= 3; icol++)
        corLepMatrix(irow, icol) += fsrErrorMatrix(irow, icol);

    correctedLepton.appendDaughter(gamma, false);

    B2DEBUG(19, "[BelleBremRecoveryModule] Found a radiative gamma and added its 4-vector to the charge particle");

    if (!m_isMultiPho) break;
  }

  correctedLepton.set4Vector(new4Vec);
  correctedLepton.updateMass(m_pdgCode);
  correctedLepton.setMomentumVertexErrorMatrix(corLepMatrix);
  correctedLepton.addExtraInfo("bremsCorrected", float(bremsGammaEnergySum > 0));
  correctedLepton.addExtraInfo("bremsCorrectedPhotonEnergy", bremsGammaEnergySum);

  // add the mc relation
  Particle* newLepton = m_particles.appendNew(correctedLepton);
  const MCParticle* mcLepton = lepton->getRelated<MCParticle>();
  const PIDLikelihood* pid = lepton->getPIDLikelihood();
  if (pid)
    newLepton->addRelationTo(pid);
  if (mcLepton)
    newLepton->addRelationTo(mcLepton);

  m_outputparticleList->addParticle(newLepton);

}
