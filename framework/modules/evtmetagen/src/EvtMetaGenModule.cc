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
#include <framework/dataobjects/EventMetaData.h>

#include <vector>

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
  std::vector<int> defaultExpRunList;
  defaultExpRunList.push_back(0);
  std::vector<int> defaultEvtNum;
  defaultEvtNum.push_back(1);

  addParam("ExpList",      m_expList,      "The list for the experiment numbers.", defaultExpRunList);
  addParam("RunList",      m_runList,      "The list for the run numbers.",        defaultExpRunList);
  addParam("EvtNumList",   m_evtNumList,   "The list for the number of events which should be processed.", defaultEvtNum);
}


EvtMetaGenModule::~EvtMetaGenModule()
{

}


void EvtMetaGenModule::initialize()
{
  //Make sure all lists have the same size
  unsigned int defListSize = m_expList.size();
  if ((m_runList.size() != defListSize) || (m_evtNumList.size() != defListSize)) {
    B2ERROR("Parameters are inconsistent. The lists must have the same number of entries.")
  } else {

    if (m_expList.size() == 0) B2ERROR("There are no events to be processed !")
    }

  m_evtNumber = 0;
  m_colIndex = 0;
}


void EvtMetaGenModule::event()
{
  if (m_evtNumber >= static_cast<unsigned long>(m_evtNumList[m_colIndex])) {

    //Search for a column where the event number is greater than 0.
    do {
      m_colIndex++;
    } while ((m_colIndex < static_cast<int>(m_expList.size())) &&
             (m_evtNumList[m_colIndex] <= 0));

    if (m_colIndex < static_cast<int>(m_expList.size())) {
      m_evtNumber = 0;
    } else {
      return;
    }
  }

  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  eventMetaDataPtr->setExperiment(m_expList[m_colIndex]);
  eventMetaDataPtr->setRun(m_runList[m_colIndex]);
  eventMetaDataPtr->setEvent(m_evtNumber);

  m_evtNumber++;
}
