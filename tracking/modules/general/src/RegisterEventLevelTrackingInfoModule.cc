/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/general/RegisterEventLevelTrackingInfoModule.h>

using namespace Belle2;

REG_MODULE(RegisterEventLevelTrackingInfo)

RegisterEventLevelTrackingInfoModule::RegisterEventLevelTrackingInfoModule() : Module()
{
  setDescription("Simple module that registers the EventLevelTrackingInfo that is used to set general tracking-related flags");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("EventLevelTrackingInfoName", m_eventLevelTrackingInfoName,
           "Name of the EventLevelTrackingInfo StoreObject", m_eventLevelTrackingInfoName);
}


void RegisterEventLevelTrackingInfoModule::initialize()
{
  // If m_eventLevelTrackingInfo already exists we'd like to keep it (typically from svd/pxd reconstruction)
  if (!m_eventLevelTrackingInfo.isOptional()) {
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

