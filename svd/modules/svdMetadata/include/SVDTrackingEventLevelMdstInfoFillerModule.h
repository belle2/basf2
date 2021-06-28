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
#include <framework/datastore/StoreObjPtr.h>

#include <svd/dataobjects/SVDCluster.h>

#include <mdst/dataobjects/EventLevelTrackingInfo.h>


namespace Belle2 {

  /**
   * This module adds additional global event level information about SVD track finding
   * to the MDST object 'EventLevelTrackingInfo'
   */
  class SVDTrackingEventLevelMdstInfoFillerModule : public Module {

  public:

    /// Constructor of the module
    SVDTrackingEventLevelMdstInfoFillerModule();

    /// Declare required StoreArray
    void initialize() override;

    /// Event processing, create store array
    void event() override;

  private:

    /// Name of the StoreObject to access the event level tracking information
    std::string m_eventLevelTrackingInfoName = "";

    /// StoreObject to access the event level tracking information
    StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo;

    /// StoreArray of SVDClusters
    std::string m_svdClustersName = "";

    /// StoreArray of SVDClusters
    StoreArray<SVDCluster> m_svdClusters;

  };
}
