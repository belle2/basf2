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
#include <tracking/dataobjects/RecoTrack.h>

#include <tracking/trackFindingCDC/collectors/matchers/MatcherInterface.h>
#include <tracking/trackFindingCDC/collectors/selectors/FilterSelector.h>
#include <tracking/trackFindingCDC/collectors/selectors/BestMatchSelector.h>
#include <tracking/trackFindingCDC/collectors/selectors/CutSelector.h>
#include <tracking/vxdCDCTrackMerging/detectorTrackCombination/DetectorTrackCombinationFilterFactory.h>
#include <tracking/vxdCDCTrackMerging/detectorTrackCombination/ExtrapolationDetectorTrackCombinationSelector.h>
#include <tracking/vxdCDCTrackMerging/detectorTrackCombination/StoreArrayMerger.h>
#include <tracking/trackFindingCDC/collectors/adders/RelationAdder.h>

namespace Belle2 {
  // TODO
  class FilterBasedVXDCDCTrackMergerFindlet : public TrackFindingCDC::Findlet<> {
  public:
    /** Constructor, for setting module description and parameters. */
    FilterBasedVXDCDCTrackMergerFindlet()
    {
      addProcessingSignalListener(&m_storeArrayMerger);
      addProcessingSignalListener(&m_allMatcher);
      addProcessingSignalListener(&m_bestMatchSelector);
      addProcessingSignalListener(&m_cutFilter);
      addProcessingSignalListener(&m_relationAdder);
      addProcessingSignalListener(&m_extrapolationSelector);
      addProcessingSignalListener(&m_distanceCutSelector);
    }

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      m_storeArrayMerger.exposeParameters(moduleParamList, prefix);
      m_allMatcher.exposeParameters(moduleParamList, prefix);
      m_bestMatchSelector.exposeParameters(moduleParamList, prefix);
      m_cutFilter.exposeParameters(moduleParamList, prefix);
      m_relationAdder.exposeParameters(moduleParamList, prefix);
      m_extrapolationSelector.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("extrapolation", prefix));
      m_distanceCutSelector.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("distanceWeight", prefix));

      moduleParamList->addParameter("extrapolate", m_param_extrapolate, "TODO", m_param_extrapolate);
      moduleParamList->addParameter("mergeAndExtrapolate", m_param_mergeAndExtrapolate, "TODO", m_param_mergeAndExtrapolate);
    }

    /**
     */
    void apply() override
    {
      // Pick up items from the data store.
      std::vector<RecoTrack*> cdcRecoTrackVector;
      std::vector<RecoTrack*> vxdRecoTrackVector;

      m_storeArrayMerger.fetch(cdcRecoTrackVector, vxdRecoTrackVector);

      std::vector<TrackFindingCDC::WeightedRelation<RecoTrack*, RecoTrack* const>> weightedRelations;
      weightedRelations.reserve(cdcRecoTrackVector.size() * vxdRecoTrackVector.size());

      // Do all combinations between vxd and cdc tracks
      m_allMatcher.apply(cdcRecoTrackVector, vxdRecoTrackVector, weightedRelations);

      // Cut out only some of them based on a filter decision (probably a mva filter)
      m_cutFilter.apply(weightedRelations);

      if (m_param_extrapolate) {
        if (m_param_mergeAndExtrapolate) {
          // Find the best matching elements
          m_bestMatchSelector.apply(weightedRelations);

          // Add the already found items from the filter-based decision
          m_relationAdder.apply(weightedRelations);

          // Clear all found relations and start from scratch - this time only using those items which vere not used before

          m_storeArrayMerger.removeCDCRecoTracksWithPartner(cdcRecoTrackVector);
          m_storeArrayMerger.removeVXDRecoTracksWithPartner(vxdRecoTrackVector);

          // Refill the weighted relations vector
          weightedRelations.clear();
          m_allMatcher.apply(cdcRecoTrackVector, vxdRecoTrackVector, weightedRelations);
        }
        // Calculate a measure based on an extrapolation
        m_extrapolationSelector.apply(weightedRelations);

        // Cut on this measure, so that to large distances are dismissed
        m_distanceCutSelector.apply(weightedRelations);
      }

      // Find the best matching elements
      m_bestMatchSelector.apply(weightedRelations);

      // Add the already found items from the filter-based decision
      m_relationAdder.apply(weightedRelations);

      // Use the relations to merge the tracks and fill them into a new store array entry
      m_storeArrayMerger.apply();
    }

  private:
    /** Operation mode: true, if the mva filter decision should be taken as correct */
    bool m_param_mergeAndExtrapolate = true;
    bool m_param_extrapolate = true;

    // Findlets
    StoreArrayMerger m_storeArrayMerger;
    TrackFindingCDC::MatcherInterface<RecoTrack*, RecoTrack*> m_allMatcher;
    TrackFindingCDC::BestMatchSelector<RecoTrack*, RecoTrack*> m_bestMatchSelector;
    TrackFindingCDC::FilterSelector<RecoTrack*, RecoTrack*,
                    TrackFindingCDC::ChooseableFilter<DetectorTrackCombinationFilterFactory>> m_cutFilter;
    TrackFindingCDC::RelationAdder<RecoTrack*, RecoTrack*> m_relationAdder;
    ExtrapolationDetectorTrackCombinationSelector m_extrapolationSelector;
    TrackFindingCDC::CutSelector<RecoTrack*, RecoTrack*> m_distanceCutSelector;
  };
}