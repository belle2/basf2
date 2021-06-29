/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>


namespace Belle2 {

  /**
   * Module to create the EventLevelTriggerTimeInfo that stores information from the Trigger Timing Distribution (TTD)
   * Will not be registered if it already exists as we want to use the EventLevelTriggerTimeInfo from the BG Overlay
   * For now this module creates just an empty object (flag is set to indicate that it is invalid)
   */
  class SimulateEventLevelTriggerTimeInfoModule : public Module {

  public:

    /// Constructor of the module
    SimulateEventLevelTriggerTimeInfoModule();

    /// Declare required StoreObjPtr (if not existing)
    void initialize() override;

    /// Event processing, create obj (if not existing)
    void event() override;

  private:

    /// StoreObject to access the EventLevelTriggerTimeInfo
    StoreObjPtr<EventLevelTriggerTimeInfo> m_eventLevelTriggerTimeInfo;
    /// Used to check if the object already exists (typically from BG Overlay)
    bool m_createNewObj = false;
  };
}
