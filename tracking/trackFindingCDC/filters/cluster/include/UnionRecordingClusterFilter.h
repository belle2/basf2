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

#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.dcl.h>

#include <tracking/trackFindingCDC/varsets/BaseVarSet.h>

#include <vector>
#include <string>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHitCluster;

    /// Filter to record multiple chooseable variable sets for wire hit clusters
    class UnionRecordingClusterFilter : public UnionRecordingFilter<ClusterFilterFactory> {

    private:
      /// Type of the base class
      using Super = UnionRecordingFilter<ClusterFilterFactory>;

    public:
      /// Get the valid names of variable sets for wire hit clusters.
      std::vector<std::string> getValidVarSetNames() const override;

      /// Create a concrete variables set for wire hit clusters from a name.
      std::unique_ptr<BaseVarSet<CDCWireHitCluster> >
      createVarSet(const std::string& name) const override;
    };
  }
}
