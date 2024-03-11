/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/kkmc/KKGenInterface.h>
#include <generators/modules/kkgeninput/KKGenInputModule.h>
#include <framework/utilities/ConditionalGaussGenerator.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <framework/utilities/FileSystem.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/IOIntercept.h>

#include <boost/filesystem.hpp>
#include <Math/Vector3D.h>

#include <stdio.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(KKGenInput);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

KKGenInputModule::KKGenInputModule() : GeneratorBaseModule(), m_initial(BeamParameters::c_smearVertex)
{
  //Set module properties
  setDescription("KKGenInput module. This an interface for KK2f Event Generator for basf2. The generated events are stored into MCParticles. You can find an expample of its decay file (tau_decaytable.dat) for tau-pair events at ${BELLE2_RELEASE_DIR}/data/generators/kkmc. On the other hand, when you like to generate mu-pair events, ${BELLE2_RELEASE_DIR}/data/generators/kkmc/mu.input.dat should be set to tauinputFile in your steering file.");
  setPropertyFlags(c_Input);

  //Parameter definition
  addParam("KKdefaultFile", m_KKdefaultFileName, "default KKMC setting filename",
           FileSystem::findFile("/data/generators/kkmc/KK2f_defaults.dat"));
  addParam("tauinputFile", m_tauinputFileName, "user-defined tau/mu/q-pairs generation setting",
           FileSystem::findFile("/data/generators/kkmc/KK2f_defaults.dat"));
  addParam("taudecaytableFile", m_taudecaytableFileName, "tau-decay-table file name",
           FileSystem::findFile("/data/generators/kkmc/tau.input.dat"));
  addParam("kkmcoutputfilename", m_KKMCOutputFileName, "KKMC output filename", string(""));

}


void KKGenInputModule::generatorInitialize()
{
  //Initialize MCParticle collection
  StoreArray<MCParticle> mcparticle;
  mcparticle.registerInDataStore();

  //Initialize initial particle for beam parameters.
  m_initial.initialize();

}

void KKGenInputModule::beginRun()
{

}

void KKGenInputModule::generatorEvent()
{

  // Check if the BeamParameters have changed (if they do, abort the job! otherwise cross section calculation will be a nightmare.)
  if (m_beamParams.hasChanged()) {
    if (!m_initialized) {
      initializeGenerator();
    } else {
      B2FATAL("KKGenInputModule::event(): BeamParameters have changed within a job, this is not supported for KKMC!");
    }
  }

  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);

  //generate an MCInitialEvent (for vertex smearing)
  ROOT::Math::XYZVector vertex = m_initial.getVertexConditional();

  const ConditionalGaussGenerator& lorentzGenerator = m_initial.getLorentzGenerator();

  mpg.clear();
  int nPart =  m_Ikkgen.simulateEvent(mpg, lorentzGenerator, vertex);

  // to check surely generated events are received or not
  for (int i = 0; i < nPart; ++i) {
    MCParticleGraph::GraphParticle* p = &mpg[i];
    int moID = 0;
    char buf[200];
    sprintf(buf, "IntC: %3d %4u %8d %4d %4d %4d %9.4f %9.4f %9.4f %9.4f",
            p->getIndex(), p->getStatus(), p->getPDG(), moID,
            p->getFirstDaughter(), p->getLastDaughter(),
            p->get4Vector().Px(), p->get4Vector().Py(),
            p->get4Vector().Pz(), p->get4Vector().E());
    B2DEBUG(100, buf);
  }

  B2DEBUG(150, "Generated event " << eventMetaDataPtr->getEvent() << " with " << nPart << " particles.");
}

void KKGenInputModule::terminate()
{
  m_Ikkgen.term();
}


void KKGenInputModule::initializeGenerator()
{

  FILE* fp;

  if (m_KKMCOutputFileName.empty()) {
    m_KKMCOutputFileName = boost::filesystem::unique_path("KKMC-%%%%%%%%%%.txt").native();
    B2DEBUG(150, "Using KKMC output file " << m_KKMCOutputFileName);
  }
  if (FileSystem::fileExists(m_KKMCOutputFileName)) {
    auto uniqueOutputFileName = boost::filesystem::unique_path(m_KKMCOutputFileName + "-%%%%%%%%%%").native();
    B2WARNING("The KKMC output file " << m_KKMCOutputFileName << " already exists. Using " << uniqueOutputFileName << " instead.");
    m_KKMCOutputFileName = uniqueOutputFileName;
  }
  fp = fopen(m_KKMCOutputFileName.c_str(), "w");
  if (fp) {
    fclose(fp);
    remove(m_KKMCOutputFileName.c_str());
  } else {
    B2FATAL("KKGenInputModule::initializeGenerator(): Failed to open KKMC output file!");
  }

  B2DEBUG(150, "m_KKdefaultFileName: " << m_KKdefaultFileName);
  B2DEBUG(150, "m_tauinputFileName: " << m_tauinputFileName);
  B2DEBUG(150, "m_taudecaytableFileName: " << m_taudecaytableFileName);
  B2DEBUG(150, "m_KKMCOutputFileName: " << m_KKMCOutputFileName);

  // Ensure that files provided by user exists
  if (!m_tauinputFileName.empty() && !FileSystem::fileExists(m_tauinputFileName)) {
    B2FATAL("KKGenInputModule::initializeGenerator(): " << m_tauinputFileName << " not found!");
  }
  if (!m_taudecaytableFileName.empty() && !FileSystem::fileExists(m_taudecaytableFileName)) {
    B2FATAL("KKGenInputModule::initializeGenerator(): " << m_taudecaytableFileName << " not found!");
  }



  //m_initial.initialize();
  m_initial.getVertexConditional();
  double E0cms       = m_initial.getNominalEcms();
  double E0cmsSpread = m_initial.getNominalEcmsSpread();



  IOIntercept::OutputToLogMessages initLogCapture("EvtGen", LogConfig::c_Debug, LogConfig::c_Info, 100, 100);
  initLogCapture.start();
  m_Ikkgen.setup(m_KKdefaultFileName, m_tauinputFileName,
                 m_taudecaytableFileName, m_KKMCOutputFileName);



  //set the beam parameters, ignoring beam energy spread for the moment
  m_Ikkgen.set_beam_info(E0cms, E0cmsSpread);
  initLogCapture.finish();

  m_initialized = true;

  B2DEBUG(150, "KKGenInputModule::initializeGenerator(): Finished initialising the KKGen Input Module. ");

}
