/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/core/EventInfoSetterModule.h>

#include <framework/core/Environment.h>
#include <framework/utilities/Utils.h>

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
  if (getenv("BELLE2_PRODUCTION"))
    m_production = stoi(getenv("BELLE2_PRODUCTION"));

  //Set module properties
  setDescription(
    "Sets the event meta data information (exp, run, evt). You must use this "
    "module to tell basf2 about the number of events you want to generate, "
    "unless you have an input module that already does so. Note that all "
    "experiment/run combinations specified must be unique."
  );

  //Parameter definition
  addParam("expList", m_expList, "List of experiment numbers. Can be overridden via --experiment argument to basf2.", m_expList);
  addParam("runList", m_runList, "List of run numbers. Can be overridden via --run argument to basf2.", m_runList);
  addParam("evtNumList", m_evtNumList, "List of the number of events which "
           "should be processed. Can be overridden via -n argument to basf2.",
           m_evtNumList);
  addParam("skipNEvents", m_eventsToSkip, "Skip this number of events before "
           "starting. Equivalent to running over this many events without performing "
           "any action, to allow starting at higher event numbers.", m_eventsToSkip);
  addParam("skipToEvent", m_skipToEvent, "Skip events until the event with "
           "the specified (experiment, run, event number) occurs. This parameter "
           "is useful for debugging to start with a specific event.", m_skipToEvent);
}


EventInfoSetterModule::~EventInfoSetterModule()
{

}


void EventInfoSetterModule::initialize()
{
  //Register the EventMetaData in the data store
  m_eventMetaDataPtr.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);

  //steering file content overwritten via command line arguments?
  unsigned int numEventsArgument = Environment::Instance().getNumberEventsOverride();
  int runOverride = Environment::Instance().getRunOverride();
  int expOverride = Environment::Instance().getExperimentOverride();
  if (numEventsArgument > 0 or runOverride >= 0 or expOverride >= 0) {
    if (m_evtNumList.size() > 1) {
      B2ERROR("The -n/--events, --run, and --experiment options cannot be used when multiple runs are specified for EventInfoSetter!");
    }
    m_evtNumList[0] = numEventsArgument;
    if (runOverride >= 0)
      m_runList[0] = runOverride;
    if (expOverride >= 0)
      m_expList[0] = expOverride;
  }

  unsigned int skipNEventsOverride = Environment::Instance().getSkipEventsOverride();
  if (skipNEventsOverride != 0)
    m_eventsToSkip = skipNEventsOverride;

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
      if (m_expList[i] < 0 or m_expList[i] > 1023)
        B2ERROR("Experiment " << m_expList[i] << " is out of range, should be in [0, 1023]!");
      if (m_runList[i] < 0)
        B2ERROR("Run " << m_runList[i] << " is out of range, should be >= 0!");
      unsigned int nevents = m_evtNumList[i];
      if (nevents == std::numeric_limits<unsigned int>::max()) {
        B2ERROR("Invalid number of events (valid range: 0.." << std::numeric_limits<unsigned int>::max() - 1 << ")!");
      }
    }
  }

  if (!m_skipToEvent.empty()) {
    // make sure the number of entries is exactly 3
    if (m_skipToEvent.size() != 3) {
      B2ERROR("skipToEvent must be a list of three values: experiment, run, event number");
      // ignore the value
      m_skipToEvent.clear();
    }
    if (m_eventsToSkip > 0) {
      B2ERROR("You cannot supply a number of events to skip (skipNEvents) and an "
              "event to skip to (skipToEvent) at the same time, ignoring skipNEvents");
      //force the number of skipped events to be zero
      m_eventsToSkip = 0;
    }
  }

  m_evtNumber = 1;
  m_colIndex = 0; //adjusted in event() if mismatched

  //determine number of events we will process
  unsigned int totalevents = 0;
  Belle2::EventInfoSetterModule copy(*this);
  while (copy.advanceEventCounter()) {
    copy.m_evtNumber++;
    totalevents++;
  }
  B2DEBUG(100, "EventInfoSetter: will process " << totalevents << " events in total.");

  Environment::Instance().setNumberOfMCEvents(totalevents);
}


bool EventInfoSetterModule::advanceEventCounter()
{
  // optimized for the assumed normal case of few runs with lots of events
  // and no skipping.
  // In the case of 1e9 events, the branch prediction hints give the following
  // improvements:
  //    clang:     -18% real time
  //    gcc (opt): -52% real time
  while (true) {
    if (branch_unlikely(m_evtNumber > m_evtNumList[m_colIndex])) {

      //Search for a column where the event number is greater than 0.
      do {
        m_colIndex++;
      } while ((m_colIndex < static_cast<int>(m_expList.size())) &&
               (m_evtNumList[m_colIndex] == 0));

      if (m_colIndex < static_cast<int>(m_expList.size())) {
        m_evtNumber = 1;
      } else { //no experiment/run with non-zero number of events found
        return false;
      }
    }

    // check if we want to skip to a specific event
    if (branch_unlikely(!m_skipToEvent.empty())) {
      // if current experiment and run number is different to what we're looking for
      if (m_skipToEvent[0] != m_expList[m_colIndex] || m_skipToEvent[1] != m_runList[m_colIndex]) {
        // then we set the m_evtNumber to the max+1
        m_evtNumber = m_evtNumList[m_colIndex] + 1;
      } else {
        // otherwise we start at the event number we want to skip to
        m_evtNumber = m_skipToEvent[2];
        // and reset the variable as skipping is done
        m_skipToEvent.clear();
      }
      // and check again if the event number is in the range we want to generate
    } else if (branch_unlikely(m_eventsToSkip != 0)) { //are we still skipping?
      unsigned int nskip = 1;
      const unsigned int eventsInList = m_evtNumList[m_colIndex];
      if (m_evtNumber <= eventsInList) //skip to end of current run?
        nskip = eventsInList - m_evtNumber + 1;
      if (nskip > m_eventsToSkip)
        nskip = m_eventsToSkip;

      m_eventsToSkip -= nskip;
      m_evtNumber += nskip;
    } else {
      break;
    }
  }
  return true;
}

void EventInfoSetterModule::event()
{
  if (!advanceEventCounter())
    return;

  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setProduction(m_production);
  m_eventMetaDataPtr->setExperiment(m_expList[m_colIndex]);
  m_eventMetaDataPtr->setRun(m_runList[m_colIndex]);
  m_eventMetaDataPtr->setEvent(m_evtNumber);
  auto time = std::chrono::high_resolution_clock::now().time_since_epoch();
  m_eventMetaDataPtr->setTime(std::chrono::duration_cast<std::chrono::nanoseconds>(time).count());

  m_evtNumber++;
}
