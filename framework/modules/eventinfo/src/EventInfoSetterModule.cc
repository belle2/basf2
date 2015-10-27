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
#include <framework/core/InputController.h>
#include <framework/dataobjects/FileMetaData.h>

#include <chrono>
#include <set>
#include <algorithm>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EventInfoSetter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EventInfoSetterModule::EventInfoSetterModule() : Module()
{
  if (getenv("BELLE2_PRODUCTION")) m_production = stoi(getenv("BELLE2_PRODUCTION"));

  //Set module properties
  setDescription(
    "Sets the event meta data information (exp, run, evt). You must use this "
    "module to tell basf2 about the number of events you want to generate, "
    "unless you have an input module that already does so. Note that all "
    "experiment/run combinations specified must be unique"
  );

  //Parameter definition
  addParam("expList", m_expList, "List of experiment numbers.", m_expList);
  addParam("runList", m_runList, "List of run numbers.", m_runList);
  addParam("evtNumList", m_evtNumList, "List of the number of events which "
           "should be processed. Can be overridden via -n argument to basf2.",
           m_evtNumList);
  addParam("skipNEvents", m_eventsToSkip, "Skip this number of events before "
           "starting. Equivalent to running over this many events without performing "
           "any action, to allow starting at higher event numbers.", m_eventsToSkip);
  addParam("skipTillEvent", m_skipTillEvent, "Skip events until the event with "
           "the specified (experiment, run, event number) occurs. This parameter "
           "is useful for debugging to start with a specific event.", m_skipTillEvent);
}


EventInfoSetterModule::~EventInfoSetterModule()
{

}


void EventInfoSetterModule::initialize()
{
  //Register the EventMetaData in the data store
  m_eventMetaDataPtr.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);

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
    B2ERROR("Parameters are inconsistent. The exp, run and evt lists must have the same number of entries.");
  } else if (defListSize == 0) {
    B2ERROR("There are no events to be processed!");
  } else {
    set<pair<int, int>> expRunSet;
    for (unsigned int i = 0; i < defListSize; i++) {
      auto ret = expRunSet.insert(make_pair(m_expList[i], m_runList[i]));
      if (!ret.second) {
        B2ERROR("Exp " << ret.first->first << ", run " << ret.first->second <<
                " used more than once! Please make sure all experiment/run combinations are unique.");
      }
    }
  }

  if (!m_skipTillEvent.empty()) {
    // make sure the number of entries is exactly 3
    if (m_skipTillEvent.size() != 3) {
      B2ERROR("skipTillEvent must be a list of three values: experiment, run, event number");
      // ignore the value
      m_skipTillEvent.clear();
    }
    if (m_eventsToSkip > 0) {
      B2ERROR("You cannot supply a number of events to skip (skipNEvents) and an "
              "event to skip to (skipTillEvent) at the same time, ignoring skipNEvents");
      //force the number of skipped events to be zero
      m_eventsToSkip = 0;
    }
  }

  m_evtNumber = 0;
  m_colIndex = 0; //adjusted in event() if mismatched
}


void EventInfoSetterModule::event()
{
  while (true) {
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

    // check if we want to skip to a specific event
    if (!m_skipTillEvent.empty()) {
      // if current experiment and run number is different to what we're looking for
      if (m_skipTillEvent[0] != m_expList[m_colIndex] || m_skipTillEvent[1] != m_runList[m_colIndex]) {
        // then we set the m_evtNumber to the max
        m_evtNumber = m_evtNumList[m_colIndex];
      } else {
        // otherwise we start at the event number we want to skip to
        m_evtNumber = m_skipTillEvent[2];
        // and reset the variable as skipping is done
        m_skipTillEvent.clear();
      }
      // and check again if the event number is in the range we want to generate
      continue;
    }

    if (m_eventsToSkip != 0) { //are we still skipping?
      unsigned long nskip = 1;
      const unsigned long eventsInList = m_evtNumList[m_colIndex];
      if (m_evtNumber < eventsInList) //skip to end of current run?
        nskip = eventsInList - m_evtNumber;
      if (nskip > m_eventsToSkip)
        nskip = m_eventsToSkip;

      m_eventsToSkip -= nskip;
      m_evtNumber += nskip;
    } else { //no? then stop.
      break;
    }
  }

  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setProduction(m_production);
  m_eventMetaDataPtr->setExperiment(m_expList[m_colIndex]);
  m_eventMetaDataPtr->setRun(m_runList[m_colIndex]);
  m_eventMetaDataPtr->setEvent(m_evtNumber);
  auto time = std::chrono::high_resolution_clock::now().time_since_epoch();
  m_eventMetaDataPtr->setTime(std::chrono::duration_cast<std::chrono::nanoseconds>(time).count());

  InputController::mcEvents()++;
  m_evtNumber++;
}
