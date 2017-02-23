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
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/utility/ReferenceFrame.h>


// utilities
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <math.h>
#include <algorithm>

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
    setDescription("Adds radiative photons to lepton");
    // fixme: no clue here
    // setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("inputListName", m_inputListName, "The initial lepton list containing the leptons to correct, should already exists.");
    addParam("outputListName", m_outputListName, "The output lepton list containing the corrected leptons.");
    addParam("gammaListName", m_gammaListName, "The gammas list containing possibly radiative gammas, should already exist.");
    addParam("angleThreshold", m_angleThres,
             "The maximal accepted angle (in degrees) between the lepton and the (radiative) gamma to be accepted", 5.0);
    addParam("energyThreshold", m_energyThres, "The maximum energy of the (radiative) gamma to be accepted.", 1.0);
    addParam("writeOut", m_writeOut,
             "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);

    // initializing the rest of private members
    m_pdgCode   = 0;
  }

  void FSRCorrectionModule::initialize()
  {
    m_pdgCode = 0;

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
      StoreObjPtr<ParticleList>::required(m_inputListName);
    }

    if (!m_decaydescriptorGamma.init(m_gammaListName)) {
      B2ERROR("[FSRCorrectionModule] Invalid gamma particle list name: " << m_gammaListName);
    } else {
      StoreObjPtr<ParticleList>::required(m_gammaListName);
    }

    // make output list
    StoreObjPtr<ParticleList> particleList(m_outputListName);
    DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
    particleList.registerInDataStore(flags);
    StoreObjPtr<ParticleList> antiParticleList(m_outputAntiListName);
    antiParticleList.registerInDataStore(flags);
  }


  void FSRCorrectionModule::event()
  {

    // clear the list
    m_usedGammas.clear();

    double cos_max = -1.0;
    double cos_limit = cos(m_angleThres * M_PI / 180.0);
    StoreArray<Particle> particles;

    const StoreObjPtr<ParticleList> inputList(m_inputListName);
    const StoreObjPtr<ParticleList> gammaList(m_gammaListName);

    // new particle list
    StoreObjPtr<ParticleList> outputList(m_outputListName);
    outputList.create();
    outputList->initialize(m_pdgCode, m_outputListName);

    StoreObjPtr<ParticleList> outputAntiList(m_outputAntiListName);
    outputAntiList.create();
    outputAntiList->initialize(-1 * m_pdgCode, m_outputAntiListName);
    outputAntiList->bindAntiParticleList(*(outputList));

    TLorentzVector new4Vec;

    B2INFO("Entries in input list" << inputList->getListSize());
    for (unsigned i = 0; i < inputList->getListSize(); i++) {
      Particle* lepton = inputList->getParticle(i);
      TLorentzVector lepton4Vector = lepton->get4Vector();
      TLorentzVector gamma4Vector;
      bool foundGamma = false;
      int gammaMdstIndex = -999;
      std::vector<int> daughterIndices;

      // look for a possible fsr gamma
      for (unsigned j = 0; j < gammaList->getListSize(); j++) {
        Particle* gamma = gammaList->getParticle(j);

        // check if gamma energy is below threshold
        if (gamma->getEnergy() > m_energyThres) continue;

        // check if gamma was already used
        int currentGamma = gamma->getMdstArrayIndex();
        bool gammaUsed = std::find(m_usedGammas.begin(), m_usedGammas.end(), currentGamma) != m_usedGammas.end();
        if (gammaUsed) continue;

        // get angle
        // todo use a more advanced angle
        // fixme do I use the correct frame
        const auto& frame = ReferenceFrame::GetCurrent();
        TVector3 pi = frame.getMomentum(lepton).Vect();
        TVector3 pj = frame.getMomentum(gamma).Vect();
        auto cos_angle = cos(pi.Angle(pj));

        if (cos_angle > cos_max) {
          cos_max = cos_angle;
          if (cos_max > cos_limit) {
            foundGamma = true;
            gammaMdstIndex = gamma->getMdstArrayIndex();
            gamma4Vector = gamma->get4Vector();
          }
        }
      }


      if (foundGamma) {
        B2INFO("[FSRCorrectionModule] Found a radiative gamma and added its 4-vector to the lepton");
        new4Vec = lepton4Vector + gamma4Vector;
        daughterIndices.push_back(lepton->getMdstArrayIndex());
        daughterIndices.push_back(gammaMdstIndex);
      }

      Particle* newParticle = nullptr;
      B2INFO("Add a particle, entries in StoreArray before adding " << particles.getEntries());
      if (foundGamma)
        newParticle = particles.appendNew(new4Vec, lepton->getPDGCode(), Particle::c_Flavored, daughterIndices);
      else
        newParticle = particles.appendNew(lepton4Vector, lepton->getPDGCode(), Particle::c_Flavored, lepton->getParticleType(),
                                          lepton->getMdstArrayIndex());

      newParticle->updateMass(lepton->getPDGCode());
      newParticle->setMomentumVertexErrorMatrix(lepton->getMomentumVertexErrorMatrix());
      newParticle->setVertex(lepton->getVertex());
      newParticle->setPValue(lepton->getPValue());
      newParticle->addExtraInfo("fsrCorrected", float(foundGamma));

      int iparticle = particles.getEntries() - 1;
      B2INFO("Add a particle, entries in outputlist before adding" << outputList->getListSize());
      outputList->addParticle(iparticle, newParticle->getPDGCode(), newParticle->getFlavorType());
      B2INFO("Add a particle, entries in outputlist after adding " << outputList->getListSize());
      B2INFO(" Entries in StoreArray after adding " <<  particles.getEntries());
    }
  }


} // end Belle2 namespace

