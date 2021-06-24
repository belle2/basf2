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
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>

#include <pxd/dataobjects/PXDCluster.h>

#include <mdst/dataobjects/EventLevelTrackingInfo.h>


namespace Belle2 {

  /**
   * This module adds additional global event level information about PXD track finding
   * to the MDST object 'EventLevelTrackingInfo'
   */
  class PXDTrackingEventLevelMdstInfoFillerModule : public Module {

  public:

    /// Constructor of the module
    PXDTrackingEventLevelMdstInfoFillerModule();

    /// Declare required StoreArray
    void initialize() override;

    /// Event processing, create store array
    void event() override;

  private:

    /// StoreObject to access the event level tracking information
    StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo;

    /// StoreArray of PXDClusters
    StoreArray<PXDCluster> m_pxdClusters;

  };
}
