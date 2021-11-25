/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <generators/modules/koralwinput/KoralWInputModule.h>

/* Belle 2 headers. */
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

using namespace std;
using namespace Belle2;


REG_MODULE(KoralWInput)


KoralWInputModule::KoralWInputModule() : Module(), m_initial(BeamParameters::c_smearVertex)
{
  //Set module properties
  setDescription("Generates four fermion final state events with KoralW.");

  //Parameter definition
  addParam("DataPath",  m_dataPath, "The path to the KoralW input data files.",
           FileSystem::findFile("/data/generators/koralw"));
  addParam("UserDataFile",  m_userDataFile, "The filename of the user KoralW input data file.",
           FileSystem::findFile("/data/generators/koralw/KoralW_eeee.data"));
}

KoralWInputModule::~KoralWInputModule()
{
}

void KoralWInputModule::initialize()
{
  StoreArray<MCParticle> mcparticle;
  mcparticle.registerInDataStore();
  // Initialize the InitialParticleGeneration utility
  m_initial.initialize();
  const BeamParameters& beams = m_initial.getBeamParameters();
  // Initialize the KoralW generator
  m_generator.setCMSEnergy(beams.getMass());
  m_generator.init(m_dataPath, m_userDataFile);
  m_initialized = true;
}

void KoralWInputModule::event()
{
  // Check if KoralW is properly initialized.
  if (not m_initialized)
    B2FATAL("KorlalW is not properly initialized.");
  // Check if the BeamParameters have changed: if they do, abort the job, otherwise cross section calculation is a nightmare.
  if (m_beamParams.hasChanged())
    B2FATAL("BeamParameters have changed within a job, this is not supported for KoralW.");
  const MCInitialParticles& initial = m_initial.generate();
  TLorentzRotation boost = initial.getCMSToLab();
  TVector3 vertex = initial.getVertex();
  m_mcGraph.clear();
  m_generator.generateEvent(m_mcGraph, vertex, boost);
  m_mcGraph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);
}

void KoralWInputModule::terminate()
{
  if (m_initialized) {
    m_generator.term();
    B2RESULT("Total cross section: " << m_generator.getCrossSection() << " pb +- " << m_generator.getCrossSectionError() << " pb");
  }
}
