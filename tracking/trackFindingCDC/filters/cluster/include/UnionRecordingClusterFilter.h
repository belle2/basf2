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

#include <tracking/trackFindingCDC/filters/cluster/ClusterFilterFactory.h>
#include <tracking/trackFindingCDC/filters/cluster/BaseClusterFilter.h>
#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.h>

#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Records the encountered CDCWireHitClusters.
    class UnionRecordingClusterFilter :
      public UnionRecordingFilter<ClusterFilterFactory> {

    private:
      /// Type of the base class
      using Super = UnionRecordingFilter<ClusterFilterFactory>;

    public:
      /// Constructor initialising the RecordingFilter with standard root file name for this filter.
      UnionRecordingClusterFilter();

      /// Valid names of variable sets for clusters.
      std::vector<std::string> getValidVarSetNames() const override;

      /// Create a concrete variables set for clusters from a name.

      std::unique_ptr<BaseVarSet<CDCWireHitCluster> >
      createVarSet(const std::string& name) const override;

    };
  }
}
