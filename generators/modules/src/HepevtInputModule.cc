/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/HepevtInputModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;
using namespace Generators;

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
  addParam("skipEvents", m_skipEventNumber, "Skip this number of events before starting.", 0);
  addParam("useWeights", m_useWeights, "Set to 'true' to if generator weights should be propagated.", false);

  m_inputMode = c_NotSet;
}


void HepevtInputModule::initialize()
{
  try {
    m_hepevt.open(m_inputFileName);
    m_hepevt.skipEvents(m_skipEventNumber);
  } catch (runtime_error &e) {
    B2FATAL(e.what());
  }
}


void HepevtInputModule::event()
{
  try {
    mpg.clear();
    double weight = 1;
    int id = m_hepevt.getEvent(mpg, weight);

    if (id > -1) {
      if (m_inputMode == c_NotSet) {
        m_inputMode = c_EvtNumFile;
      }
      if (m_inputMode != c_EvtNumFile) B2FATAL("The event number is taken from the HepEvt file, but was taken from an external source previously !")

        StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
      eventMetaDataPtr->setExperiment(0);
      eventMetaDataPtr->setRun(0);
      eventMetaDataPtr->setEvent(id);

      if (m_useWeights)
        eventMetaDataPtr->setGeneratedWeight(weight);
    } else {
      if (m_inputMode == c_NotSet) {
        m_inputMode = c_EvtNumExternal;
      }
      if (m_inputMode != c_EvtNumExternal) B2FATAL("The event number is not available for this event, but was available for previous events !")
      }
    mpg.generateList(DEFAULT_MCPARTICLES, MCParticleGraph::set_decay_info | MCParticleGraph::check_cyclic);
  } catch (HepevtReader::HepEvtEmptyEventError) {
    B2DEBUG(100, "Reached end of HepEvt file.")
  } catch (runtime_error &e) {
    B2ERROR(e.what());
  }
}

