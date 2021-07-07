/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/modules/SimulateEventLevelTriggerTimeInfoModule.h>

using namespace Belle2;

REG_MODULE(SimulateEventLevelTriggerTimeInfo)

SimulateEventLevelTriggerTimeInfoModule::SimulateEventLevelTriggerTimeInfoModule() : Module()
{
  setDescription("Simple module that registers the EventLevelTriggerTimeInfo if not yet present");
  setPropertyFlags(c_ParallelProcessingCertified);
}


void SimulateEventLevelTriggerTimeInfoModule::initialize()
{
  // If EventLevelTriggerTimeInfo already exists we'd like to keep it (typically from BG Overlay)
  if (!m_eventLevelTriggerTimeInfo.isOptional()) {
    m_createNewObj = true;
    m_eventLevelTriggerTimeInfo.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);
  }
}


void SimulateEventLevelTriggerTimeInfoModule::event()
{
  // Simply create a new object
  // (default constructor without arguments sets flag that stored data is invalid)
  if (m_createNewObj) {
    m_eventLevelTriggerTimeInfo.create();
  }
}

