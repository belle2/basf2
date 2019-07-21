/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
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
    addParam("minimum_energy", m_energy_min, "The minimum energy of the (radiative) gamma to be accepted.", 0.05);
    addParam("multiplePhotons", isMultiPho, "If only the nearest photon to add then make it False otherwise true", true);
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
      StoreObjPtr<ParticleList>().isRequired(m_inputListName);
    }

    if (!m_decaydescriptorGamma.init(m_gammaListName)) {
      B2ERROR("[BelleBremRecoveryModule] Invalid gamma particle list name: " << m_gammaListName);
    } else {
      StoreObjPtr<ParticleList>().isRequired(m_gammaListName);
    }

    // make output list
    StoreObjPtr<ParticleList> particleList(m_outputListName);
    DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
    particleList.registerInDataStore(flags);
    StoreObjPtr<ParticleList> antiParticleList(m_outputAntiListName);
    antiParticleList.registerInDataStore(flags);
  }


  void BelleBremRecoveryModule::event()
  {
    StoreArray<Particle> particles;
    StoreArray<MCParticle> mcParticles;

    RelationArray particlesToMCParticles(particles, mcParticles);

    const StoreObjPtr<ParticleList> inputList(m_inputListName);
    const StoreObjPtr<ParticleList> gammaList(m_gammaListName);

    // new output particle list
    StoreObjPtr<ParticleList> outputList(m_outputListName);
    outputList.create();
    outputList->initialize(m_pdgCode, m_outputListName);

    StoreObjPtr<ParticleList> outputAntiList(m_outputAntiListName);
    outputAntiList.create();
    outputAntiList->initialize(-1 * m_pdgCode, m_outputAntiListName);
    outputAntiList->bindAntiParticleList(*(outputList));
    std::vector<int> usedGammas;
    // loop over charged particles, correct them and add them to the output list
    const unsigned int nLep = inputList->getListSize();
    for (unsigned i = 0; i < nLep; i++) {
      const Particle* lepton = inputList->getParticle(i);
      TLorentzVector lepton4Vector = lepton->get4Vector();
      TLorentzVector new4Vec = lepton->get4Vector();
      std::map<Particle*, double> map;
      // look for all possible (radiative) gamma
      const unsigned int nGam = gammaList->getListSize();
      for (unsigned j = 0; j < nGam; j++) {
        Particle* gamma = gammaList->getParticle(j);

        // check if gamma energy is within allowed energy range
        if (gamma->getEnergy() < m_energy_min) continue;
        bool gammaUsed = std::find(usedGammas.begin(), usedGammas.end(), gamma->getMdstArrayIndex()) != usedGammas.end();
        if (gammaUsed) continue;
        // get angle (in lab system)
        TVector3 pi = lepton->getMomentum();
        TVector3 pj = gamma->getMomentum();
        double angle = (pi.Angle(pj));
        if (m_angleThres > angle) {
          B2INFO("[BelleBremRecoveryModule] Found a radiative gamma and added its 4-vector to the charge particle");
          map.insert(std::pair<Particle*, double>(gamma, angle));
        }
      }
      std::vector<pair> vec;
      std::copy(map.begin(), map.end(), std::back_inserter<std::vector<pair>>(vec));
      std::sort(vec.begin(), vec.end(), [](const pair & l, const pair & r) {
        if (l.second != r.second)
          return l.second < r.second;
      });
      if (!isMultiPho && vec.size() > 1)
        vec.erase(vec.begin() + 1, vec.end());
      for (auto const& x : vec)
        new4Vec += x.first->get4Vector();
      Particle correctedLepton(new4Vec, lepton->getPDGCode());
      correctedLepton.appendDaughter(lepton);
      const TMatrixFSym& lepErrorMatrix = lepton->getMomentumVertexErrorMatrix();
      TMatrixFSym corLepMatrix(c_DimMatrix);
      for (auto const& x : vec) {
        Particle* fsrgamma = x.first;
        usedGammas.push_back(fsrgamma->getMdstArrayIndex());
        fsrgamma->addExtraInfo("theta_e_gamma", x.second);
        const TMatrixFSym& fsrErrorMatrix = fsrgamma->getMomentumVertexErrorMatrix();
        for (int irow = 0; irow < c_DimMatrix; irow++)
          for (int icol = irow; icol < c_DimMatrix; icol++) {
            if (irow > 3 || icol > 3)
              corLepMatrix(irow, icol) = lepErrorMatrix(irow, icol);
            else
              corLepMatrix(irow, icol) = lepErrorMatrix(irow, icol) + fsrErrorMatrix(irow, icol);
          }
        correctedLepton.appendDaughter(fsrgamma);
        correctedLepton.setMomentumVertexErrorMatrix(corLepMatrix);
      }
      if (int(vec.size()) == 0)
        correctedLepton.setMomentumVertexErrorMatrix(lepton->getMomentumVertexErrorMatrix());
      correctedLepton.setVertex(lepton->getVertex());
      correctedLepton.setPValue(lepton->getPValue());
      correctedLepton.addExtraInfo("nGamma", float(vec.size()));

      // add the mc relation
      Particle* newLepton = particles.appendNew(correctedLepton);
      const MCParticle* mcLepton = lepton->getRelated<MCParticle>();
      if (mcLepton != nullptr) newLepton->addRelationTo(mcLepton);
      outputList->addParticle(newLepton);
    }
  }

} // end Belle2 namespace

