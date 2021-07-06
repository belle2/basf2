/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
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

    /// Used to check if the object already exists (we need the object already during svd/pxd reconstruction)
    bool m_createNewObj = false;
  };
}
