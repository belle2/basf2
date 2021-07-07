/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/modules/hepmcreader/HepmcInputModule.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/Environment.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(HepMCInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

HepMCInputModule::HepMCInputModule() : Module(), m_evtNum(0), m_minEvent(-1), m_maxEvent(INT_MAX), m_totalEvents(-1), m_initial(0)
{
  //Set module properties
  setDescription("HepMC file input. This module loads an event record from HEPMC2 format and store the content into the MCParticle collection. HEPMC format is used by for example pythia8.");
  setPropertyFlags(c_Input);

  //Parameter definition
  addParam("inputFileList", m_inputFileNames, "List of names of HepMC2 files");
  addParam("ignoreReadEventNr", m_ignorereadEventNr, "Parallel pythia can have dublicate event nrs.", false);
  addParam("runNum", m_runNum, "run number to start from", 0);
  addParam("expNum", m_expNum, "ExpNum to start from", 0);
  addParam("minEvt", m_minEvent, "Start converting at event number.", -1);
  addParam("maxEvt", m_maxEvent, "Stop converting at Event number.", INT_MAX);
  addParam("useWeights", m_useWeights, "Set to 'true' to if generator weights should be propagated.", false);
  addParam("nVirtualParticles", m_nVirtual, "Number of particles at the beginning of the events that should be made virtual.", 0);
  addParam("boost2Lab", m_boost2Lab, "Boolean to indicate whether the particles should be boosted from CM frame to lab frame", false);
  addParam("wrongSignPz", m_wrongSignPz, "Boolean to signal that directions of HER and LER were switched", false);
}


void HepMCInputModule::initialize()
{
  m_hepmcreader.reset(new HepMCReader(m_minEvent, m_maxEvent));
  TLorentzRotation m_labboost;     /**< Boost&rotation vector for boost from CM to LAB. */

  if (m_runNum != 0 || m_expNum != 0) {
    B2WARNING("Initialising the first read events with expNr or runNr != 0. This can lead to downloading the wrong payloads from the database if you are using MC!");
  }
  //Beam Parameters, initial particl
  m_initial.initialize();
  m_eventMetaDataPtr.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);
  m_iFile = 0;
  if (m_inputFileNames.size() == 0) {
    //something is wrong with the file list.
    B2FATAL("invalid list of input files. No entries found.");
  } else {
    //let's start with the first file:
    m_inputFileName = m_inputFileNames[m_iFile];
  }
  try {
    B2INFO("Opening first file: " << m_inputFileName);
    m_hepmcreader->open(m_inputFileName);
    //m_hepmcreader.skipEvents(m_skipEventNumber);
  } catch (runtime_error& e) {
    B2FATAL(e.what());
  }
  m_hepmcreader->m_nVirtual = m_nVirtual;
  m_hepmcreader->m_wrongSignPz = m_wrongSignPz;

  //Do we need to boost?
  if (m_boost2Lab) {
    const MCInitialParticles& initial = m_initial.generate();
    TLorentzRotation boost = initial.getCMSToLab();
    m_hepmcreader->m_labboost = boost;
  }

  //are we the master module? And do we have all infos?
  B2INFO("HEPMC reader acts as master module for data processing. This means the event numbers etc will be set by this module.");
  if (m_runNum == 0 && m_expNum == 0) {
    B2WARNING("HEPMC reader acts as master module, but no run and experiment number set. Using defaults.");
  }

  //Initialize MCParticle collection
  StoreArray<MCParticle> mcparticle;
  mcparticle.registerInDataStore();
  m_totalEvents = m_hepmcreader->countEvents(m_inputFileName);
  Environment::Instance().setNumberOfMCEvents(m_totalEvents);

}


void HepMCInputModule::event()
{
  // we get the next event until it is invalid which we catch with that exception
  B2DEBUG(20,
          "Event ________________________________________________________________________________________________________________________________________________________________________________________________");
  if (m_beamParams.hasChanged()) {
    if (m_boost2Lab) {
      const MCInitialParticles& initial = m_initial.generate();
      TLorentzRotation boost = initial.getCMSToLab();
      m_hepmcreader->m_labboost = boost;
    }
    B2WARNING("HepmcInputModule::event(): BeamParameters have changed within a job!");
  }

  if (!m_eventMetaDataPtr) { m_eventMetaDataPtr.create(); }
  try {
    m_mcParticleGraph.clear();
    double weight = 1;
    int id = m_hepmcreader->getEvent(m_mcParticleGraph, weight);
    if (id > -1 && !m_ignorereadEventNr) {
      m_evtNum = id;
    } else {
      id = ++m_evtNum;
    }
    B2DEBUG(20, "Setting exp " << m_expNum << " run " << m_runNum << " event " << id << ".");
    m_eventMetaDataPtr->setExperiment(m_expNum);
    m_eventMetaDataPtr->setRun(m_runNum);
    m_eventMetaDataPtr->setEvent(id);
    if (m_useWeights) { m_eventMetaDataPtr->setGeneratedWeight(weight); }
    m_mcParticleGraph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);
  } catch (HepMCReader::HepMCInvalidEventError&) {
    B2DEBUG(20, "Reached end of HepMC file.");
    m_hepmcreader->closeCurrentInputFile();
    m_iFile++;
    if (m_iFile < m_inputFileNames.size()) {
      try {
        m_inputFileName = m_inputFileNames[m_iFile];
        B2INFO("Opening next file: " << m_inputFileName);
        m_hepmcreader->open(m_inputFileName);
      } catch (runtime_error& e) {
        B2FATAL(e.what());
      }
    } else {
      m_eventMetaDataPtr->setEndOfData();
      B2DEBUG(20, "Reached end of all HepMC files.");
    }
  } catch (runtime_error& e) {
    B2ERROR(e.what());
  }
}

void HepMCInputModule::terminate()
{
  if (m_evtNum != m_totalEvents) { B2WARNING("Eventnumber mismatch. (ignore if more than one file was read.)");}
}

