/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>
#include <tracking/trackingUtilities/utilities/WeightedRelation.h>
#include <tracking/ckf/vtx/filters/results/ChooseableVTXResultFilter.h>

namespace Belle2 {
  class ModuleParamList;
  class RecoTrack;
  class CKFToVTXResult;

  /**
   * The results of the CKF ar in the form (seed -> vector of hits). For the
   * following findlets, it is easier to have them in the form (seed -> related VTX track).
   * This is done in this findlet.
   *
   * Doing this, one can additionally apply a filter.
   */
  class VTXRecoTrackRelator : public TrackingUtilities::Findlet<const CKFToVTXResult,
    TrackingUtilities::WeightedRelation<const RecoTrack, const RecoTrack>> {
  public:
    /// The parent findlet
    using Super = TrackingUtilities::Findlet<CKFToVTXResult,
          TrackingUtilities::WeightedRelation<const RecoTrack, const RecoTrack>>;

    /// Add the sub findelts as listener
    VTXRecoTrackRelator();

    /// Expose the parameters of the sub findlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /// Create relations between tracks from the results (applying a filter)
    void apply(const std::vector<CKFToVTXResult>& results,
               std::vector<TrackingUtilities::WeightedRelation<const RecoTrack, const RecoTrack>>& relationsCDCToVTX) final;

  private:
    /// Filter for the results
    ChooseableVTXResultFilter m_overlapFilter;
  };
}
