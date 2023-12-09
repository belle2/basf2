/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <generators/modules/evtgeninput/EvtGenInputModule.h>

/* Generators headers. */
#include <generators/evtgen/EvtGenInterface.h>
#include <generators/evtgen/EvtGenUtilities.h>

/* Basf2 headers. */
#include <framework/utilities/FileSystem.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticleGraph.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EvtGenInput);

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
           FileSystem::findFile("decfiles/dec/DECAY_BELLE2.DEC", true));
  addParam("ParentParticle", m_parentParticle, "Parent Particle Name", string("Upsilon(4S)"));
  addParam("InclusiveType", m_inclusiveType, "inclusive decay type (0: generic, 1: inclusive, 2: inclusive (charge conjugate)", 0);
  addParam("CoherentMixing", m_coherentMixing, "decay the neutral B meson pairs coherently or non-coherently", true);
  addParam("InclusiveParticle", m_inclusiveParticle, "Inclusive Particle Name", string(""));
  addParam("maxTries", m_maxTries, "Number of tries to generate a parent "
           "particle from the beam energies which fits inside the mass window "
           "before giving up", 100000);
}


void EvtGenInputModule::initialize()
{
  StoreArray<MCParticle> mcparticle;
  mcparticle.registerInDataStore();
  generators::checkEvtGenDecayFile(m_DECFileName);
  // Initial particle for beam parameters.
  m_initial.initialize();
}


void EvtGenInputModule::beginRun()
{

}

MCInitialParticles  EvtGenInputModule::createBeamParticle(double minMass, double maxMass)
{
  // try to generate the 4 momentum a m_maxTries amount of times before we give up
  for (int i = 0; i < m_maxTries; ++i) {
    const MCInitialParticles& initial = m_initial.generate();

    // check if we fullfill the mass window
    if (initial.getMass() >= minMass && initial.getMass() < maxMass) {
      return initial;
    }
  }

  //Apparently the beam energies don't match the particle mass we want to generate
  B2FATAL("Could not create parent particle within mass window: "
          << "minMass=" << minMass << " GeV, "
          << "maxMass=" << maxMass << " GeV");

  //This will never be reached, only used to avoid warning
  return m_initial.generate();
}


// Add colliding electron/positron to the event graph
static void addInitialParticle(MCParticleGraph& mpg, int pdg, ROOT::Math::PxPyPzEVector p4)
{
  MCParticleGraph::GraphParticle& part = mpg.addParticle();

  part.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator | MCParticle::c_Initial);
  part.setMass(Const::electronMass);
  part.setPDG(pdg);

  part.set4Vector(p4);

  part.setProductionVertex(0, 0, 0);
  part.setProductionTime(0);
  part.setValidVertex(false);
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

  MCInitialParticles initial;

  //Initialize the beam energy for each event separatly
  if (EvtPDL::getStdHep(m_parentId) == 10022) {
    //virtual photon (vpho), no mass window, we accept everything
    initial = createBeamParticle();
  } else {
    //everything else needs to be in the mass window
    initial = createBeamParticle(EvtPDL::getMinMass(m_parentId),
                                 EvtPDL::getMaxMass(m_parentId));
  }

  ROOT::Math::PxPyPzEVector pParentParticle = initial.getLER() + initial.getHER();
  ROOT::Math::XYZVector primaryVertex = initial.getVertex();

  //end initialization

  //clear existing MCParticles
  mpg.clear();

  // add colliding electron & positron to the event list
  addInitialParticle(mpg, 11, initial.getHER());
  addInitialParticle(mpg, -11, initial.getLER());

  //generate event.
  int nPart =  m_Ievtgen.simulateEvent(mpg, pParentParticle, primaryVertex,
                                       m_inclusiveType, m_inclusiveParticle);

  B2DEBUG(10, "EvtGen: generated event with " << nPart << " particles.");
}

void EvtGenInputModule::initializeGenerator()
{

  //setup the DECAY files:
  m_Ievtgen.setup(m_DECFileName, m_parentParticle, m_userDECFileName, m_coherentMixing);

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
