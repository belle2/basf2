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
      B2ERROR("FSRCorrectionModule::initialize Invalid output ParticleList name: " << m_outputListName);

    // output particle
    const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();
    m_pdgCode  = mother->getPDGCode();
    m_outputAntiListName = ParticleListName::antiParticleListName(m_outputListName);

    // get exiting particle lists
    if (m_inputListName == m_outputListName) {
      B2ERROR("FSRCorrectionModule::initialize Input and output list names " << m_inputListName << " are the same.");
    } else if (!m_decaydescriptor.init(m_inputListName)) {
      B2ERROR("FSRCorrectionModule::initialize Invalid input ParticleList name: " << m_inputListName);
    } else {
      StoreObjPtr<ParticleList>::required(m_inputListName);
    }

    if (!m_decaydescriptorGamma.init(m_gammaListName)) {
      B2ERROR("FSRCorrectionModule::initialize Invalid input ParticleList name: " << m_gammaListName);
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
    const StoreArray<Particle> particles;

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

    //fixme check if antiparticles are included

    // look for a possible fsr gamma
    for (unsigned i = 0; i < inputList->getListSize(); i++) {
      Particle* lepton = inputList->getParticle(i);
      TLorentzVector lepton4Vector = lepton->get4Vector();
      TLorentzVector gamma4Vector;
      bool foundGamma = false;
      int gammaMdstIndex = -999;
      std::vector<int> daughterIndices;

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

      // make new lepton
      TLorentzVector new4Vec;
      if (foundGamma) {
        B2INFO("FSRCorrectionModule::event Found a radiative gamma and added its 4-vector to the lepton");
        new4Vec = lepton4Vector + gamma4Vector;
        daughterIndices.push_back(lepton->getMdstArrayIndex());
        daughterIndices.push_back(gammaMdstIndex);
        m_usedGammas.push_back(gammaMdstIndex);

      } else {
        new4Vec = lepton4Vector;
        daughterIndices.push_back(lepton->getMdstArrayIndex());
      }

      // fixme not sure what else to set
      // fixme m_particleArray - his this correct?
      Particle newLepton = Particle(new4Vec, lepton->getPDGCode(), Particle::c_Flavored, daughterIndices, lepton->getArrayPointer());
      newLepton.setVertex(lepton->getVertex());
      newLepton.setMomentumVertexErrorMatrix(lepton->getMomentumVertexErrorMatrix());
      newLepton.setPValue(lepton->getPValue());
      newLepton.updateMass(lepton->getPDGCode()); //no sure about that one
      newLepton.addExtraInfo("fsrCor", float(foundGamma));
      // fixme not sure about the relation to the tracks (necessary?)

      // add new particle
      outputList->addParticle(&newLepton);

    }
  }


} // end Belle2 namespace

