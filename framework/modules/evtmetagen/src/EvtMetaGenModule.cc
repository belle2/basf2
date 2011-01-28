/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "framework/modules/evtmetagen/EvtMetaGenModule.h"

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/EventMetaData.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EvtMetaGen)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EvtMetaGenModule::EvtMetaGenModule() : Module()
{
  //Set module properties
  setDescription("Sets the event meta data information (exp, run, evt).");

  //Parameter definition
  addParam("ExpList",      m_expList,      "The list for the experiment numbers.");
  addParam("RunList",      m_runList,      "The list for the run numbers.");
  addParam("EvtStartList", m_evtStartList, "The list for the event start numbers.");
  addParam("EvtEndList",   m_evtEndList,   "The list for the event end numbers.");
}


EvtMetaGenModule::~EvtMetaGenModule()
{

}


void EvtMetaGenModule::initialize()
{
  //Make sure all lists have the same size
  unsigned int defListSize = m_expList.size();
  if ((m_runList.size() != defListSize) || (m_evtStartList.size() != defListSize) ||
      (m_evtEndList.size() != defListSize)) {
    B2ERROR("Parameters are inconsistent. The lists must have the same number of entries.")
  } else {

    if (m_expList.size() != 0) {

      //Make sure the event start number is smaller than the event end number
      for (unsigned int iEvt = 0; iEvt < defListSize; ++iEvt) {
        if (m_evtStartList[iEvt] > m_evtEndList[iEvt]) {
          B2ERROR("Exp " << m_expList[iEvt] << ", Run " << m_runList[iEvt] <<  ": The start event number (" << m_evtStartList[iEvt]  << ") is greater than the end event number (" << m_evtEndList[iEvt] << ")")
          break;
        }
      }

      m_evtNumber = m_evtStartList[0] - 1;
    } else B2ERROR("There are no events to be processed !")
    }
  m_colIndex = 0;
}


void EvtMetaGenModule::event()
{
  if (m_colIndex >= static_cast<int>(m_expList.size())) return;

  m_evtNumber++;
  if (m_evtNumber > m_evtEndList[m_colIndex]) {
    m_colIndex++;
    if (m_colIndex >= static_cast<int>(m_expList.size())) {
      return;
    } else {
      m_evtNumber = m_evtStartList[m_colIndex];
    }
  }

  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  eventMetaDataPtr->setExperiment(m_expList[m_colIndex]);
  eventMetaDataPtr->setRun(m_runList[m_colIndex]);
  eventMetaDataPtr->setEvent(m_evtNumber);
  return;
}
