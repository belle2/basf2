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
  m_eventLevelTrackingInfo.registerInDataStore(m_eventLevelTrackingInfoName, DataStore::c_ErrorIfAlreadyRegistered);
}


void RegisterEventLevelTrackingInfoModule::event()
{
  m_eventLevelTrackingInfo.create();
}

