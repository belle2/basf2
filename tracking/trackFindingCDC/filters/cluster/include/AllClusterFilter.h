/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/cluster/BaseClusterFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHitCluster;

    /// Filter accepting all clusters
    class AllClusterFilter : public BaseClusterFilter {

    public:
      /// Basic filter method to override. All implementation rejects all clusters.
      Weight operator()(const CDCWireHitCluster& cluster) final;
    };
  }
}
