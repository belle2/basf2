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
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/ckf/svd/filters/results/ChooseableSVDResultFilter.h>

namespace Belle2 {
  class ModuleParamList;
  class RecoTrack;
  class CKFToSVDResult;

  /**
   * The results of the CKF ar in the form (seed -> vector of hits). For the
   * following findlets, it is easier to have them in the form (seed -> related SVD track).
   * This is done in this findlet.
   *
   * Doing this, one can additionally apply a filter.
   */
  class RecoTrackRelator : public TrackFindingCDC::Findlet<const CKFToSVDResult,
    TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>> {
  public:
    /// The parent findlet
    using Super = TrackFindingCDC::Findlet<CKFToSVDResult,
          TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>>;

    /// Add the sub findelts as listener
    RecoTrackRelator();

    /// Expose the parameters of the sub findlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /// Create relations between tracks from the results (applying a filter)
    void apply(const std::vector<CKFToSVDResult>& results,
               std::vector<TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>>& relationsCDCToSVD) final;

  private:
    /// Filter for the results
    ChooseableSVDResultFilter m_overlapFilter;
  };
}