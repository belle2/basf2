/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz, Martin Ritter, Torben Ferber            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/evtgeninput/EvtGenInputModule.h>
#include <generators/evtgen/EvtGenInterface.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <framework/utilities/FileSystem.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EvtGenInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EvtGenInputModule::EvtGenInputModule() : Module(),
  m_initial(BeamParameters::c_smearALL)
{
  //Set module properties
  setDescription("EvtGenInput module. The module is served as an interface for EvtGen Event Generator so that the EvtGen generator can store the generated particles into MCParticles. The users need to provide their own decay mode based on the standard DECAY.DEC.");
  setPropertyFlags(c_Input);

  //Parameter definition
  addParam("userDECFile", m_userDECFileName, "user DECfile name", string(""));
  addParam("DECFile", m_DECFileName, "global DECfile to be used",
           FileSystem::findFile("generators/evtgen/decayfiles/DECAY_BELLE2.DEC", true));
  addParam("ParentParticle", m_parentParticle, "Parent Particle Name", string("Upsilon(4S)"));
  addParam("InclusiveType", m_inclusiveType, "inclusive decay type (0: generic, 1: inclusive, 2: inclusive (charge conjugate)", 0);
  addParam("InclusiveParticle", m_inclusiveParticle, "Inclusive Particle Name", string(""));
  addParam("maxTries", m_maxTries, "Number of tries to generate a parent "
           "particle from the beam energies which fits inside the mass window "
           "before giving up", 100000);

  m_PrimaryVertex = TVector3(0., 0., 0.);

}


void EvtGenInputModule::initialize()
{
  const std::string defaultDecFile = FileSystem::findFile("generators/evtgen/decayfiles/DECAY_BELLE2.DEC", true);
  if (m_DECFileName.empty()) {
    B2ERROR("No global decay file defined, please make sure the parameter 'DECFile' is set correctly");
    return;
  }
  if (defaultDecFile.empty()) {
    B2WARNING("Cannot find default decay file");
  } else if (defaultDecFile != m_DECFileName) {
    B2INFO("Using non-standard DECAY file \"" << m_DECFileName << "\"");
  }
  //Initialize MCParticle collection
  StoreArray<MCParticle> mcparticle;
  mcparticle.registerInDataStore();

  //initial particle for beam parameters
  m_initial.initialize();

}


void EvtGenInputModule::beginRun()
{

}

TLorentzVector EvtGenInputModule::createBeamParticle(double minMass, double maxMass)
{
  // try to generate the 4 momentum a m_maxTries amount of times before we give up
  for (int i = 0; i < m_maxTries; ++i) {
    MCInitialParticles& initial = m_initial.generate();

    // check if we fullfill the mass window
    if (initial.getMass() >= minMass && initial.getMass() < maxMass) {

      TLorentzVector beam = initial.getLER() + initial.getHER();
      m_PrimaryVertex = initial.getVertex();
      return beam;
    }
  }

  //Apparently the beam energies don't match the particle mass we want to generate
  B2FATAL("Could not create parent particle within mass window: "
          << "minMass=" << minMass << " GeV, "
          << "maxMass=" << maxMass << " GeV");

  //This will never be reached so return empty to avoid warning
  return TLorentzVector(0, 0, 0, 0);
}

void EvtGenInputModule::event()
{
  B2DEBUG(10, "Starting event generation");

  // Check if the BeamParameters have changed (if they do, abort the job! otherwise cross section calculation will be a nightmare.)
  if (m_beamParams.hasChanged()) {
    if (!m_initialized) {
      initializeGenerator();
    } else {
      B2FATAL("EvtGenInputModule::event(): BeamParameters have changed within a job, this is not supported for EvtGen!");
    }
  }

  TLorentzVector pParentParticle;

  //Initialize the beam energy for each event separatly
  if (m_parentId.getId() == 93) {
    //virtual photon (vpho), no mass window, we accept everything
    pParentParticle = createBeamParticle();
  } else {
    //everything else needs to be in the mass window
    pParentParticle = createBeamParticle(EvtPDL::getMinMass(m_parentId),
                                         EvtPDL::getMaxMass(m_parentId));
  }
  //end initialization

  //clear existing MCParticles
  mpg.clear();

  //generate event.
  int nPart =  m_Ievtgen.simulateEvent(mpg, pParentParticle, m_PrimaryVertex,
                                       m_inclusiveType, m_inclusiveParticle);

  B2DEBUG(10, "EvtGen: generated event with " << nPart << " particles.");
}

void EvtGenInputModule::initializeGenerator()
{

  //setup the DECAY files:
  m_Ievtgen.setup(m_DECFileName, m_parentParticle, m_userDECFileName);

  if (m_inclusiveType == 0) m_inclusiveParticle = "";
  if (m_inclusiveType != 0 && EvtPDL::getId(m_inclusiveParticle).getId() == -1) {
    B2ERROR("User Specified Inclusive Particle '" << m_inclusiveParticle
            << "' does not exist");
  }
  m_parentId = EvtPDL::getId(m_parentParticle);
  if (m_parentId.getId() == -1) {
    B2ERROR("User specified parent particle '" << m_parentParticle
            << "' does not exist");
  }

  m_initialized = true;

}
