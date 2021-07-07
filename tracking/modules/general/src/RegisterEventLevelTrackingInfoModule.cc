/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/general/RegisterEventLevelTrackingInfoModule.h>

using namespace Belle2;

REG_MODULE(RegisterEventLevelTrackingInfo)

RegisterEventLevelTrackingInfoModule::RegisterEventLevelTrackingInfoModule() : Module()
{
  setDescription("Simple module that registers the EventLevelTrackingInfo that is used to set general tracking-related flags");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("EventLevelTrackingInfoName",
           m_eventLevelTrackingInfoName,
           "Name of the EventLevelTrackingInfo that should be used (different one for ROI-finding).",
           m_eventLevelTrackingInfoName);
}


void RegisterEventLevelTrackingInfoModule::initialize()
{
  // If m_eventLevelTrackingInfo already exists we'd like to keep it (typically from svd/pxd reconstruction)
  if (!m_eventLevelTrackingInfo.isOptional(m_eventLevelTrackingInfoName)) {
    m_createNewObj = true;
    m_eventLevelTrackingInfo.registerInDataStore(m_eventLevelTrackingInfoName, DataStore::c_ErrorIfAlreadyRegistered);
  }
}


void RegisterEventLevelTrackingInfoModule::event()
{
  if (m_createNewObj) {
    m_eventLevelTrackingInfo.create();
  }
}

