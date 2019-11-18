/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/collectors/selectors/BestMatchSelector.h>
#include <tracking/trackFindingCDC/collectors/adders/RelationAdder.h>

#include <tracking/modules/vxdCDCTrackMerger/StoreArrayMerger.h>

#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  class RecoTrack;

  /// Findlet for merging VXD and CDC tracks with MC information.
  class MCVXDCDCTrackMergerFindlet : public TrackFindingCDC::Findlet<> {
  public:
    /// Constructor, for setting module description and parameters.
    MCVXDCDCTrackMergerFindlet();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Do the track merging.
    void apply() override;

  private:
    // Findlets
    /// Get and write back the relations to the store array.
    StoreArrayMerger m_storeArrayMerger;
    /// Make a best candidate selection
    TrackFindingCDC::BestMatchSelector<RecoTrack*, RecoTrack*> m_bestMatchSelector;
    /// Use the weighted relations to turn them into real DataStore relations.
    TrackFindingCDC::RelationAdder<RecoTrack*, RecoTrack*> m_relationAdder;

    // Parameters
    /// Only use fitted CDC tracks, as otherwise the comparison with the CKF is unfair.
    bool m_param_onlyFittedCDCTracks = true;
  };

  /// This module merges tracks which are reconstructed, separately, in the silicon (PXD+VXD) and in the CDC using MC
  class MCVXDCDCTrackMergerModule : public TrackFindingCDC::FindletModule<MCVXDCDCTrackMergerFindlet> {
  public:
    MCVXDCDCTrackMergerModule()
    {
      setDescription(
        "This module merges tracks which are reconstructed, separately, in the silicon (PXD+VXD) and in the CDC using MC");
    }
  };
}
