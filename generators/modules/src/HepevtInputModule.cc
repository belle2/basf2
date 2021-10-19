/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/modules/HepevtInputModule.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

using namespace std;
using namespace Belle2;

REG_MODULE(HepevtInput)

HepevtInputModule::HepevtInputModule() : Module(), m_evtNum(-1)
{
  //Set module properties
  setDescription("HepEvt file input. This module loads an event record from HEPEVT format and store the content into the MCParticle collection. HEPEVT format is a standard event record format to contain an event record in a Monte Carlo-independent format.");
  setPropertyFlags(c_Input);

  //Parameter definition
  addParam("inputFileList", m_inputFileNames, "List of names of Hepevt files");
  addParam("makeMaster", m_makeMaster, "Boolean to indicate whether the event numbers from input file should be used.", false);
  addParam("runNum", m_runNum, "Run number", -1);
  addParam("expNum", m_expNum, "Experiment number", -1);
  addParam("skipEvents", m_skipEventNumber, "Skip this number of events before starting.", 0);
  addParam("useWeights", m_useWeights, "Set to 'true' to if generator weights should be propagated.", false);
  addParam("nVirtualParticles", m_nVirtual, "Number of particles at the beginning of the events that should be made virtual.", 0);
  addParam("wrongSignPz", m_wrongSignPz, "Boolean to signal that directions of HER and LER were switched", false);
}

void HepevtInputModule::initialize()
{
  if (m_expNum < 0 or m_runNum < 0)
    B2FATAL("The exp. and run numbers are not properly initialized: please set the 'expNum' and 'runNum' parameters of the HepevtInput module.");

  m_eventMetaData.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);
  B2INFO("HepevtInput acts as input module for this process. This means the exp., run and event numbers will be set by this module.");

  m_iFile = 0;
  if (m_inputFileNames.size() == 0) {
    //something is wrong with the file list.
    B2FATAL("Invalid list of input files, no entries found.");
  } else {
    //let's start with the first file:
    m_inputFileName = m_inputFileNames[m_iFile];
  }
  try {
    B2INFO("Opening first file: " << m_inputFileName);
    m_hepevt.open(m_inputFileName);
    m_hepevt.skipEvents(m_skipEventNumber);
  } catch (runtime_error& e) {
    B2FATAL(e.what());
  }
  m_hepevt.m_nVirtual = m_nVirtual;
  m_hepevt.m_wrongSignPz = m_wrongSignPz;

  //Initialize MCParticle collection
  StoreArray<MCParticle> mcparticle;
  mcparticle.registerInDataStore();
}


void HepevtInputModule::event()
{
  if (!m_eventMetaData)
    m_eventMetaData.create();
  try {
    mpg.clear();
    double weight = 1;
    int id = m_hepevt.getEvent(mpg, weight);
    if (m_makeMaster) {
      if (id > -1) {
        m_evtNum = id;
      } else {
        id = ++m_evtNum;
      }
      m_eventMetaData->setExperiment(m_expNum);
      m_eventMetaData->setRun(m_runNum);
      m_eventMetaData->setEvent(id);
    }
    if (m_useWeights)
      m_eventMetaData->setGeneratedWeight(weight);
    mpg.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);
  } catch (HepevtReader::HepEvtEmptyEventError&) {
    B2DEBUG(100, "Reached end of HepEvt file.");
    m_hepevt.closeCurrentInputFile();
    m_iFile++;
    if (m_iFile < m_inputFileNames.size()) {
      try {
        m_inputFileName = m_inputFileNames[m_iFile];
        B2INFO("Opening next file: " << m_inputFileName);
        m_hepevt.open(m_inputFileName);
      } catch (runtime_error& e) {
        B2FATAL(e.what());
      }
    } else {
      m_eventMetaData->setEndOfData();
      B2DEBUG(100, "Reached end of all HepEvt files.");
    }
  } catch (runtime_error& e) {
    B2ERROR(e.what());
  }
}
