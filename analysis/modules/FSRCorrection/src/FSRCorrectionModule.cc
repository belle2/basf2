/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/FSRCorrection/FSRCorrectionModule.h>

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

using namespace std;

namespace Belle2 {

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

  REG_MODULE(FSRCorrection)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

  FSRCorrectionModule::FSRCorrectionModule() :
    Module()

  {
    // set module description (e.g. insert text)
    setDescription("Takes the particles from the given lepton list copies them to the output list and adds the 4-vector of the closest photon (considered as radiative) to the lepton, if the given criteria for maximum angle and energy are fulfilled.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("inputListName", m_inputListName, "The initial lepton list containing the leptons to correct, should already exists.");
    addParam("outputListName", m_outputListName, "The output lepton list containing the corrected leptons.");
    addParam("gammaListName", m_gammaListName, "The gammas list containing possibly radiative gammas, should already exist.");
    addParam("angleThreshold", m_angleThres,
             "The maximum angle (in degrees) between the lepton and the (radiative) gamma to be accepted.", 5.0);
    addParam("energyThreshold", m_energyThres, "The maximum energy of the (radiative) gamma to be accepted.", 1.0);
    addParam("writeOut", m_writeOut,
             "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);

    // initializing the rest of private members
    m_pdgCode   = 0;
  }

  void FSRCorrectionModule::initialize()
  {
    // check the validity of output ParticleList name
    bool valid = m_decaydescriptor.init(m_outputListName);
    if (!valid)
      B2ERROR("[FSRCorrectionModule] Invalid output ParticleList name: " << m_outputListName);

    // output particle
    const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();
    m_pdgCode  = mother->getPDGCode();
    m_outputAntiListName = ParticleListName::antiParticleListName(m_outputListName);

    // get exiting particle lists
    if (m_inputListName == m_outputListName) {
      B2ERROR("[FSRCorrectionModule] Input and output particle list names are the same: " << m_inputListName);
    } else if (!m_decaydescriptor.init(m_inputListName)) {
      B2ERROR("[FSRCorrectionModule] Invalid input particle list name: " << m_inputListName);
    } else {
      StoreObjPtr<ParticleList>().isRequired(m_inputListName);
    }

    if (!m_decaydescriptorGamma.init(m_gammaListName)) {
      B2ERROR("[FSRCorrectionModule] Invalid gamma particle list name: " << m_gammaListName);
    } else {
      StoreObjPtr<ParticleList>().isRequired(m_gammaListName);
    }

    // make output list
    StoreObjPtr<ParticleList> particleList(m_outputListName);
    DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
    particleList.registerInDataStore(flags);
    StoreObjPtr<ParticleList> antiParticleList(m_outputAntiListName);
    antiParticleList.registerInDataStore(flags);
    m_maxAngle = cos(m_angleThres * M_PI / 180.0);
  }


  void FSRCorrectionModule::event()
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

    // loop over leptons, correct them and add them to the output list
    const unsigned int nLep = inputList->getListSize();
    for (unsigned i = 0; i < nLep; i++) {
      const Particle* lepton = inputList->getParticle(i);
      Particle* fsrGamma = nullptr;

      TLorentzVector lepton4Vector = lepton->get4Vector();
      TLorentzVector new4Vec = lepton->get4Vector();

      double cos_max = -1.0;
      bool fsrGammaFound = false;

      // look for a possible fsr gamma
      const unsigned int nGam = gammaList->getListSize();
      for (unsigned j = 0; j < nGam; j++) {
        Particle* gamma = gammaList->getParticle(j);

        // check if gamma energy is below threshold
        if (gamma->getEnergy() > m_energyThres) continue;

        // get angle (in lab system)
        TVector3 pi = lepton->getMomentum();
        TVector3 pj = gamma->getMomentum();
        // todo use a more advanced angle
        double cos_angle = cos(pi.Angle(pj));

        if (cos_angle > cos_max) {
          cos_max = cos_angle;
          if (cos_max > m_maxAngle) {
            // check if gamma was already used
            bool gammaUsed = std::find(usedGammas.begin(), usedGammas.end(), gamma->getMdstArrayIndex()) != usedGammas.end();
            if (gammaUsed) continue;
            fsrGammaFound = true;
            fsrGamma = gamma;
          }
        }
      }

      if (fsrGammaFound) {
        new4Vec = lepton4Vector + fsrGamma->get4Vector();
        usedGammas.push_back(fsrGamma->getMdstArrayIndex());
        B2INFO("[FSRCorrectionModule] Found a radiative gamma and added its 4-vector to the lepton");
      }

      Particle correctedLepton(new4Vec, lepton->getPDGCode());
      correctedLepton.appendDaughter(lepton);
      if (fsrGammaFound) {
        correctedLepton.appendDaughter(fsrGamma);
        // update error matrix
        const TMatrixFSym& lepErrorMatrix = lepton->getMomentumVertexErrorMatrix();
        const TMatrixFSym& fsrErrorMatrix = fsrGamma->getMomentumVertexErrorMatrix();
        // todo add check
        TMatrixFSym corLepMatrix(c_DimMatrix);

        for (int irow = 0; irow < c_DimMatrix; irow++) {
          for (int icol = irow; icol < c_DimMatrix; icol++) {
            if (irow > 3 || icol > 3) {
              corLepMatrix(irow, icol) = lepErrorMatrix(irow, icol);
            } else {
              corLepMatrix(irow, icol) = lepErrorMatrix(irow, icol) + fsrErrorMatrix(irow, icol);
            }
          }
        }
        correctedLepton.setMomentumVertexErrorMatrix(corLepMatrix);
      } else {
        correctedLepton.setMomentumVertexErrorMatrix(lepton->getMomentumVertexErrorMatrix());
      }


      // add the info from original lepton to the new lepton
      correctedLepton.setVertex(lepton->getVertex());
      correctedLepton.setPValue(lepton->getPValue());

      correctedLepton.addExtraInfo("fsrCorrected", float(fsrGammaFound));

      // add the mc relation
      Particle* newLepton = particles.appendNew(correctedLepton);
      const MCParticle* mcLepton = lepton->getRelated<MCParticle>();
      if (mcLepton != nullptr) newLepton->addRelationTo(mcLepton);
      outputList->addParticle(newLepton);

    }
  }


} // end Belle2 namespace

