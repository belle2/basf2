/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <set>
#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;
  class SpacePoint;

  /// Findlet for tagging all space points in the results vector as used
  template <class AResult, class ACluster>
  class SpacePointTagger : public TrackFindingCDC::Findlet<const AResult, const SpacePoint* const> {
    /// The parent class
    using Super = TrackFindingCDC::Findlet<const AResult, const SpacePoint* const>;

  public:
    /// Clear the used clusters
    void beginEvent() override;

    /// Expose the parameters of the findlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Mark all space points as used, that they share clusters if the given kind with the results.
    void apply(const std::vector<AResult>& results,
               const std::vector<const SpacePoint*>& spacePoints) override;

  private:
    // Object Pool
    /// Store the used clusters in the results
    std::set<const ACluster*> m_usedClusters;
    /// Store the used space points in the results
    std::set<const SpacePoint*> m_usedSpacePoints;

    // Parameters
    /// Parameter: Mark SP as used, if the share a single cluster with the results, or if they share a whole SP.
    bool m_param_singleClusterLevel = true;
    /// Parameter: Mark used space points as assigned
    bool m_param_markUsedSpacePoints = true;
  };
}
