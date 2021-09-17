/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/modules/LHEInputModule.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <TF1.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(LHEInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

LHEInputModule::LHEInputModule() : Module(),
  m_nInitial(0),
  m_nVirtual(0),
  m_evtNum(-1),
  m_initial(0)
{
  //Set module properties
  setDescription("LHE file input. This module loads an event record from LHE format and store the content into the MCParticle collection. LHE format is a standard event record format to contain an event record in a Monte Carlo-independent format.");
  setPropertyFlags(c_Input);

  //Parameter definition
  addParam("inputFileList", m_inputFileNames, "List of names of LHE files");
  addParam("makeMaster", m_makeMaster, "Boolean to indicate whether the event numbers from input file should be used.", false);
  addParam("runNum", m_runNum, "Run number (should be set if makeMaster=true)", 0);
  addParam("expNum", m_expNum, "Experiment number (should be set if makeMaster=true)", 0);
  addParam("skipEvents", m_skipEventNumber, "Skip this number of events before starting.", 0);
  addParam("useWeights", m_useWeights, "Set to 'true' to if generator weights should be propagated (not implemented yet).", false);
  addParam("nInitialParticles", m_nInitial, "Number of MCParticles at the beginning of the events that should be flagged c_Initial.",
           0);
  addParam("nVirtualParticles", m_nVirtual,
           "Number of MCParticles at the beginning of the events that should be flagged c_IsVirtual.", 0);
  addParam("boost2Lab", m_boost2Lab, "Boolean to indicate whether the particles should be boosted from CM frame to lab frame", false);
  addParam("wrongSignPz", m_wrongSignPz, "Boolean to signal that directions of HER and LER were switched", true);
  addParam("meanDecayLength", m_meanDecayLength,
           "Mean decay length(mean lifetime * c) between displaced vertex to IP, default to be zero, unit in cm", 0.);
  addParam("Rmin", m_Rmin, "Minimum of distance between displaced vertex to IP", 0.);
  addParam("Rmax", m_Rmax, "Maximum of distance between displaced vertex to IP", 1000000.);
  addParam("pdg_displaced", m_pdg_displaced, "PDG code of the displaced particle being studied", 9000008);
}


void LHEInputModule::initialize()
{
  //Beam Parameters, initial particles
  m_initial.initialize();

  m_iFile = 0;
  if (m_inputFileNames.size() == 0) {
    //something is wrong with the file list.
    B2FATAL("Invalid list of input files. No entries found.");
  } else {
    //let's start with the first file:
    m_inputFileName = m_inputFileNames[m_iFile];
  }
  try {
    B2INFO("Opening first file: " << m_inputFileName);
    m_lhe.open(m_inputFileName);
    m_lhe.skipEvents(m_skipEventNumber);
  } catch (runtime_error& e) {
    B2FATAL(e.what());
  }
  m_lhe.setInitialIndex(m_nInitial);
  m_lhe.setVirtualIndex(m_nInitial + m_nVirtual);
  m_lhe.m_wrongSignPz = m_wrongSignPz;

  //boost
  if (m_boost2Lab) {
    const MCInitialParticles& initial = m_initial.generate();
    ROOT::Math::LorentzRotation boost = initial.getCMSToLab();
    m_lhe.m_labboost = boost;
  }

  //pass displaced vertex to LHEReader
  m_lhe.m_meanDecayLength = m_meanDecayLength;
  m_lhe.m_Rmin = m_Rmin;
  m_lhe.m_Rmax = m_Rmax;
  m_lhe.m_pdgDisplaced = m_pdg_displaced;
  //print out warning information if default R range is change
  if (m_Rmin != 0 || m_Rmax != 1000000) {
    TF1 fr("fr", "exp(-x/[0])", 0, 1000000);
    double factor;
    factor = fr.Integral(m_Rmin, m_Rmax) / fr.Integral(0, 1000000);
    B2WARNING("Default range of R is changed, new range is from " << m_Rmin << "cm to " << m_Rmax <<
              " cm. This will change the cross section by a factor of " << factor);
  }

  //are we the master module? And do we have all infos?
  if (m_makeMaster) {
    B2INFO("LHE reader acts as master module for data processing.");
    if (m_runNum == 0 && m_expNum == 0)
      B2WARNING("LHE reader acts as master module, but no run and experiment number set. Using defaults.");
    //register EventMetaData object in data store
    StoreObjPtr<EventMetaData> eventmetadata;
    eventmetadata.registerInDataStore();
  }

  //Initialize MCParticle collection
  StoreArray<MCParticle> mcparticle;
  mcparticle.registerInDataStore();

}


void LHEInputModule::event()
{

  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  if (!eventMetaDataPtr) eventMetaDataPtr.create();
  B2DEBUG(100, "LHE processes event nbr " << eventMetaDataPtr->getEvent());

  try {
    mpg.clear();
    double weight = 1;
    int id = m_lhe.getEvent(mpg, weight);

    if (m_makeMaster) {
      if (id > -1) {
        m_evtNum = id;
      } else {
        id = ++m_evtNum;
      }

      eventMetaDataPtr->setExperiment(m_expNum);
      eventMetaDataPtr->setRun(m_runNum);
      eventMetaDataPtr->setEvent(id);
    }
    if (m_useWeights)
      eventMetaDataPtr->setGeneratedWeight(weight);
    mpg.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);
  } catch (LHEReader::LHEEmptyEventError&) {
    B2DEBUG(100, "Reached end of LHE file.");
    m_lhe.closeCurrentInputFile();
    m_iFile++;
    if (m_iFile < m_inputFileNames.size()) {
      try {
        m_inputFileName = m_inputFileNames[m_iFile];
        B2INFO("Opening next file: " << m_inputFileName);
        m_lhe.open(m_inputFileName);
      } catch (runtime_error& e) {
        B2FATAL(e.what());
      }
    } else {
      eventMetaDataPtr->setEndOfData();
      B2DEBUG(100, "Reached end of all LHE files.");
    }
  } catch (runtime_error& e) {
    B2ERROR(e.what());
  }

}
