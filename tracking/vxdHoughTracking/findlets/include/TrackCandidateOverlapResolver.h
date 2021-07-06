/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDCluster.h>

#include <string>
#include <vector>

namespace Belle2 {
  class ModuleParamList;
  class SpacePointTrackCand;

  namespace vxdHoughTracking {

    /// Findlet for rejecting wrong SpacePointTrackCands and for removing bad hits.
    class TrackCandidateOverlapResolver : public TrackFindingCDC::Findlet<SpacePointTrackCand> {
      /// Parent class
      using Super = TrackFindingCDC::Findlet<SpacePointTrackCand>;

    public:
      /// Find intercepts in the 2D Hough space
      TrackCandidateOverlapResolver();

      /// Default destructor
      ~TrackCandidateOverlapResolver();

      /// Expose the parameters of the sub findlets.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Create the store arrays
      void initialize() override;

      /// Reject bad SpacePointTrackCands and bad hits inside the remaining
      void apply(std::vector<SpacePointTrackCand>& spacePointTrackCandsToResolve) override;

    private:
      /// Stay connected to the DataStore for timing improvement.
      StoreArray<SVDCluster> m_svdClusters;
      /// Name of array of SVDCluster.
      std::string m_param_nameSVDClusters = "SVDClusters";

      /// Strategy used to resolve overlaps.
      std::string m_param_resolveMethod = "greedy";

      /// Minimum of activityState of candidate required to be accepted by the algorithm.
      double m_param_minActivityState = 0.7;
    };

  }
}
