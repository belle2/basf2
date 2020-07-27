/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>


namespace Belle2 {

  /**
   * Module to create the EventLevelTrackingInfo that is used to set
   * general tracking-related flags
   */
  class RegisterEventLevelTrackingInfoModule : public Module {

  public:

    /// Constructor of the module
    RegisterEventLevelTrackingInfoModule();

    /// Declare required StoreArray
    void initialize() override;

    /// Event processing, create store array
    void event() override;

  private:

    /// Name of the StoreObject to access the event level tracking information
    std::string m_eventLevelTrackingInfoName = "EventLevelTrackingInfo";
    /// StoreObject to access the event level tracking information
    StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo;
  };
}
