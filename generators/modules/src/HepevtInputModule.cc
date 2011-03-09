/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Susanne Koblitz                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/HepevtInputModule.h>
#include <generators/utilities/cm2LabBoost.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(HepevtInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

HepevtInputModule::HepevtInputModule() : Module()
{
  //Set module properties
  setDescription("Hepevt file input");
  setPropertyFlags(c_Input);

  //Parameter definition
  addParam("inputFileName", m_inputFileName, "Hepevt filename");
  addParam("makeMaster", m_makeMaster, "Boolean to indicate whether the event numbers from input file should be used.", false);
  addParam("runNum", m_runNum, "run number (should be set if makeMaster=true)", 0);
  addParam("expNum", m_expNum, "ExpNum (should be set if makeMaster=true)", 0);
  addParam("skipEvents", m_skipEventNumber, "Skip this number of events before starting.", 0);
  addParam("useWeights", m_useWeights, "Set to 'true' to if generator weights should be propagated.", false);
  addParam("nVirtualParticles", m_nVirtual, "Number of particles at the beginning of the events that should be made virtual.", 0);
  addParam("boost2lab", m_boost2Lab, "Boolean to indicate whether the particles should be boosted from CM frame to lab frame", false);
  addParam("wrongSignPz", m_wrongSignPz, "Boolean to signal that directions of HER and LER were switched", false);
}


void HepevtInputModule::initialize()
{
  try {
    m_hepevt.open(m_inputFileName);
    m_hepevt.skipEvents(m_skipEventNumber);
  } catch (runtime_error& e) {
    B2FATAL(e.what());
  }
  m_hepevt.m_nVirtual = m_nVirtual;
  m_hepevt.m_wrongSignPz = m_wrongSignPz;

  //Do we need to boost?
  if (m_boost2Lab) {
    //@TODO: get this from a central place instead of hard coded: framework issue
    // this is hard coded!!!!!! should be provided somewhere -> run meta data
    double Eher = 7.0 * Unit::GeV;
    double Eler = 4.0 * Unit::GeV;
    double cross_angle = 83 * Unit::mrad;
    double angle = 41.5 * Unit::mrad;
    m_hepevt.m_labboost = getBoost(Eher, Eler, cross_angle, angle);
  }

  //are we the master module? And do we have all infos?
  if (m_makeMaster) {
    B2INFO("HEPEVT reader acts as master module for data processing.");
    if (m_runNum == 0 && m_expNum == 0)
      B2WARNING("HEPEVT reader acts as master module, but no run and experiment number set. Using defaults.");
  }
}


void HepevtInputModule::event()
{
  try {
    mpg.clear();
    double weight = 1;
    int id = m_hepevt.getEvent(mpg, weight);

    StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
    if (m_makeMaster) {

      if (id > -1) {
        eventMetaDataPtr->setExperiment(m_expNum);
        eventMetaDataPtr->setRun(m_runNum);
        eventMetaDataPtr->setEvent(id);
      } else
        B2FATAL("The HepEvtReader is running in Master mode BUT he can't find the event number in the file!")

      }
    if (m_useWeights)
      eventMetaDataPtr->setGeneratedWeight(weight);
    mpg.generateList(DEFAULT_MCPARTICLES, MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);
  } catch (HepevtReader::HepEvtEmptyEventError) {
    StoreObjPtr <EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
    eventMetaDataPtr->setEndOfData();
    B2DEBUG(100, "Reached end of HepEvt file.")
  } catch (runtime_error& e) {
    B2ERROR(e.what());
  }
}


