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
  setPropertyFlags(c_TriggersNewRun | c_TriggersEndOfData);

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

      m_colIndex = -1;
      setProcessRecordType(prt_EndRun);
    } else B2ERROR("There are no events to be processed !")
    }
}


void EvtMetaGenModule::event()
{
  bool storeMetaData = true;

  switch (getProcessRecordType()) {
    case prt_EndRun : //If the process record type is endRun, a run was finished last time, therefore start a new run if possible.
      m_colIndex++;

      if (m_colIndex >= static_cast<int>(m_expList.size())) {
        setProcessRecordType(prt_EndOfData);
        storeMetaData = false;
      } else {
        m_evtNumber = m_evtStartList[m_colIndex];
        setProcessRecordType(prt_BeginRun);
      }
      break;
    case prt_BeginRun : //If the process record type is beginRun, a new run was started last time, therefore change to event mode.
      setProcessRecordType(prt_Event);
      break;
    case prt_Event : //Increase the event number. If the max. event number is reached stop the run.
      if (m_evtNumber + 1 > m_evtEndList[m_colIndex]) {
        setProcessRecordType(prt_EndRun);
        storeMetaData = false;
      } else m_evtNumber++;
      break;
    case prt_EndOfData :
      B2ERROR("EndOfData record type in event() method !")
      break;
  }

  //Store the event meta data information.
  if (storeMetaData) {
    StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
    eventMetaDataPtr->setExperiment(m_expList[m_colIndex]);
    eventMetaDataPtr->setRun(m_runList[m_colIndex]);
    eventMetaDataPtr->setEvent(m_evtNumber);
  }

}
