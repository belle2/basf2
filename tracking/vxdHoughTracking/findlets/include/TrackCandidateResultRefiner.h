/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>
#include <tracking/vxdHoughTracking/findlets/TrackCandidateOverlapResolver.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <string>
#include <vector>

namespace Belle2 {
  class SpacePointTrackCand;
  class ModuleParamList;

  namespace vxdHoughTracking {

    /// Findlet for rejecting wrong SpacePointTrackCands and for removing bad hits.
    class TrackCandidateResultRefiner : public TrackFindingCDC::Findlet<SpacePointTrackCand, SpacePointTrackCand> {
      /// Parent class
      using Super = TrackFindingCDC::Findlet<SpacePointTrackCand, SpacePointTrackCand>;

    public:
      /// Find intercepts in the 2D Hough space
      TrackCandidateResultRefiner();

      /// Default destructor
      ~TrackCandidateResultRefiner();

      /// Expose the parameters of the sub findlets.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Create the store arrays
      void initialize() override;

      /// End run and write Root file
      void beginRun() override;

      /// Reject bad SpacePointTrackCands and bad hits inside the remaining
      void apply(std::vector<SpacePointTrackCand>& unrefinedResults, std::vector<SpacePointTrackCand>& refinedResults) override;

    private:
      /// Identifier which estimation method to use. Valid identifiers are:
      /// mcInfo, circleFit, tripletFit, helixFit
      std::string m_param_EstimationMethod = "tripletFit";
      /// sets the name of the expected StoreArray containing MCRecoTracks. Only required for MCInfo method
      std::string m_param_MCRecoTracksStoreArrayName = "MCRecoTracks";
      /// Only required for MCInfo method
      bool m_param_MCStrictQualityEstimator = true;
      /// pointer to the selected QualityEstimator
      std::unique_ptr<QualityEstimatorBase> m_estimator;

      /// Resolve hit overlaps in track candidates
      TrackCandidateOverlapResolver m_overlapResolver;

      /// Cut on the quality estimator and only further propagate SPTCs with three hits that are above this value
      double m_param_minQualitiyIndicatorSize3 = 0.001;
      /// Cut on the quality estimator and only further propagate SPTCs with four hits that are above this value
      double m_param_minQualitiyIndicatorSize4 = 0.001;
      /// Cut on the quality estimator and only further propagate SPTCs with five hits that are above this value
      double m_param_minQualitiyIndicatorSize5 = 0.001;

      /// Accept nHits for each size at maximum
      uint m_param_maxNumberOfEachPathLength = 10;
    };

  }
}
