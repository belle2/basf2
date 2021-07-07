/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
