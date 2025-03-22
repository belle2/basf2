/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/cluster/ClusterFilterFactory.h>

#include <tracking/trackingUtilities/filters/base/UnionRecordingFilter.dcl.h>

#include <tracking/trackingUtilities/varsets/BaseVarSet.h>

#include <vector>
#include <string>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHitCluster;

    /// Filter to record multiple chooseable variable sets for wire hit clusters
    class UnionRecordingClusterFilter : public TrackingUtilities::UnionRecordingFilter<ClusterFilterFactory> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::UnionRecordingFilter<ClusterFilterFactory>;

    public:
      /// Get the valid names of variable sets for wire hit clusters.
      std::vector<std::string> getValidVarSetNames() const override;

      /// Create a concrete variables set for wire hit clusters from a name.
      std::unique_ptr<TrackingUtilities::BaseVarSet<CDCWireHitCluster> >
      createVarSet(const std::string& name) const override;
    };
  }
}
