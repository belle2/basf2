/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: David Dossett                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/modules/core/EventLimiterModule.h>
#include <framework/core/Module.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EventLimiter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EventLimiterModule::EventLimiterModule() : Module()
{
  // Set module properties
  setDescription("Allows you to set limits on the number of events per run passing this module. "
                 "It returns True until the limit is reached, after which it returns False. "
                 "basf2 conditional paths can then be used to prevent events continuing onwards from this module.");

  // Parameter definitions
  addParam("maxEventsPerRun", m_maxEventsPerRun,
           "Maximum number of events that will have True returned on them per run. "
           "This module returns True until the limit in a particular run is reached, it then returns False. "
           "It will only start returning True again once a new run begins. "
           "The default value (-1) means that this module always returns True regardless of how many events "
           "are processed in a run.", int(-1));
}

void EventLimiterModule::initialize()
{
  m_eventMetaData.isRequired();
}

void EventLimiterModule::event()
{
  // Do we care about the number of events and are we past the limit?
  if (m_maxEventsPerRun > -1 and m_returnValue == true) {
    // Have we exceeded our maximum events in this run?
    if (m_runEvents >= m_maxEventsPerRun) {
      // If we have, we should skip collection until further notice
      B2INFO("Reached maximum number of events ("
             << m_maxEventsPerRun
             << ") for (Experiment, Run) = ("
             << m_eventMetaData->getExperiment() << ", "
             << m_eventMetaData->getRun() << ")");
      m_returnValue = false;
    } else {
      m_runEvents += 1;
    }
  }
  setReturnValue(m_returnValue);
}

void EventLimiterModule::beginRun()
{
  // Do we care about the number of events in each run?
  if (m_maxEventsPerRun > -1) {
    m_runEvents = 0;
    m_returnValue = true;
  }
}
