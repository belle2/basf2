/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Malwin Weiler, Nils Braun                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/collectors/matchers/MatcherInterface.h>
#include <tracking/trackFindingCDC/collectors/selectors/FilterSelector.h>
#include <tracking/trackFindingCDC/collectors/selectors/BestMatchSelector.h>
#include <tracking/trackFindingCDC/collectors/selectors/CutSelector.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>

#include <tracking/modules/vxdCDCTrackMerger/DetectorTrackCombinationFilterFactory.h>
#include <tracking/modules/vxdCDCTrackMerger/ExtrapolationDetectorTrackCombinationSelector.h>
#include <tracking/modules/vxdCDCTrackMerger/StoreArrayMerger.h>
#include <tracking/modules/vxdCDCTrackMerger/DetectorTrackCombinationVarSet.h>
#include <tracking/modules/vxdCDCTrackMerger/DetectorTrackCombinationTruthVarSet.h>
#include <tracking/trackFindingCDC/collectors/adders/RelationAdder.h>

namespace Belle2 {
  class ModuleParamList;

  /**
   * Findlet for combining VXD and CDC tracks.
   *
   * The workflow has three steps:
   *
   * 1) All possible combinations between cdc and vxd tracks are fed into a filter (e.g. a MVA filter).
   *    A best candidate selection is performed.
   * 2) If (extrapolate is true and mergeAndExtrapolate is true) or (extrapolate is false),
   *    those combinations that survive the filter are immediately merged. In the following, only the
   *    remaining tracks without a partner are used.
   * 3) If extrapolate is true, the remaining combinations (either those that survived the filter if
   *    mergeAndExtrapolate is false ot those that do not have a partner if mergeAndExtrapolate is true)
   *    are extrapolated onto the CDC innter wall and their distance is compared.
   *    Again, a best candidate selection is performed.
   *
   */
  class FilterBasedVXDCDCTrackMergerFindlet : public TrackFindingCDC::Findlet<> {
  public:
    /// Constructor, for setting module description and parameters.
    FilterBasedVXDCDCTrackMergerFindlet();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Do the track merging.
    void apply() override;

  private:
    /// Operation mode: true, if the mva filter decision should be taken as correct.
    bool m_param_mergeAndExtrapolate = true;

    /// Operation mode: true, if an extrapolation should happen.
    bool m_param_extrapolate = true;

    // Findlets
    /// Get and write back the relations to the store array.
    StoreArrayMerger m_storeArrayMerger;
    /// Create all possible matches between CDC and VXD tracks
    TrackFindingCDC::MatcherInterface<RecoTrack*, RecoTrack*> m_allMatcher;
    /// Make a best candidate selection
    TrackFindingCDC::BestMatchSelector<RecoTrack*, RecoTrack*> m_bestMatchSelector;
    /// Filter the relations based on a (MVA) filter
    TrackFindingCDC::FilterSelector<RecoTrack*, RecoTrack*,
                    TrackFindingCDC::ChooseableFilter<DetectorTrackCombinationFilterFactory>> m_cutFilter;
    /// Use the weighted relations to turn them into real DataStore relations.
    TrackFindingCDC::RelationAdder<RecoTrack*, RecoTrack*> m_relationAdder;
    /// A filter based on an extrapolation to a common plane.
    ExtrapolationDetectorTrackCombinationSelector m_extrapolationSelector;
    /// A filter based on the extrapolation result and some additional variables
    TrackFindingCDC::FilterSelector<RecoTrack*, RecoTrack*,
                    TrackFindingCDC::ChooseableFilter<DetectorTrackCombinationFilterFactory>> m_distanceFilterCut;
  };
}
