/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/eventinfo/EventInfoSetterModule.h>

#include <framework/core/Environment.h>

#include <vector>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EventInfoSetter)
REG_MODULE(EvtMetaGen)
REG_MODULE(EventNumbers)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EventInfoSetterModule::EventInfoSetterModule() : Module()
{
  //Set module properties
  setDescription("Sets the event meta data information (exp, run, evt). You must use this module to tell basf2 about the number of events you want to generate, unless you have an input module that already does so.");

  //Parameter definition
  std::vector<int> defaultExpRunList;
  defaultExpRunList.push_back(0);
  std::vector<int> defaultEvtNum;
  defaultEvtNum.push_back(1);

  addParam("expList",      m_expList,      "List of experiment numbers.", defaultExpRunList);
  addParam("runList",      m_runList,      "List of run numbers.",        defaultExpRunList);
  addParam("evtNumList",   m_evtNumList,   "List of the number of events which should be processed.", defaultEvtNum);
}


EventInfoSetterModule::~EventInfoSetterModule()
{

}


void EventInfoSetterModule::initialize()
{
  //Register the EventMetaData in the data store
  m_eventMetaDataPtr.registerAsPersistent();

  //steering file content overwritten via command line arguments?
  int numEventsArgument = Environment::Instance().getNumberEventsOverride();
  if (numEventsArgument > 0) {
    if (m_evtNumList.size() > 1) {
      B2ERROR("The -n/--events option cannot be used when multiple runs are specified for EventInfoSetter!");
    }
    m_evtNumList[0] = numEventsArgument;
  }

  //Make sure all lists have the same size
  unsigned int defListSize = m_expList.size();
  if ((m_runList.size() != defListSize) || (m_evtNumList.size() != defListSize)) {
    B2ERROR("Parameters are inconsistent. The exp, run and evt lists must have the same number of entries.")
  } else if (defListSize == 0) {
    B2ERROR("There are no events to be processed!")
  }

  m_evtNumber = 0;
  m_colIndex = 0;
}


void EventInfoSetterModule::event()
{
  if (m_evtNumber >= static_cast<unsigned long>(m_evtNumList[m_colIndex])) {

    //Search for a column where the event number is greater than 0.
    do {
      m_colIndex++;
    } while ((m_colIndex < static_cast<int>(m_expList.size())) &&
             (m_evtNumList[m_colIndex] <= 0));

    if (m_colIndex < static_cast<int>(m_expList.size())) {
      m_evtNumber = 0;
    } else { //no experiment/run with non-zero number of events found
      return;
    }
  }

  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(m_expList[m_colIndex]);
  m_eventMetaDataPtr->setRun(m_runList[m_colIndex]);
  m_eventMetaDataPtr->setEvent(m_evtNumber);

  m_evtNumber++;
}
