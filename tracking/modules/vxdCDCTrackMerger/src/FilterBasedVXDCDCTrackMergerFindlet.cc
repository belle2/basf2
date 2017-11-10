/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Malwin Weiler, Nils Braun                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/vxdCDCTrackMerger/FilterBasedVXDCDCTrackMergerFindlet.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;

FilterBasedVXDCDCTrackMergerFindlet::FilterBasedVXDCDCTrackMergerFindlet()
{
  addProcessingSignalListener(&m_storeArrayMerger);
  addProcessingSignalListener(&m_allMatcher);
  addProcessingSignalListener(&m_bestMatchSelector);
  addProcessingSignalListener(&m_cutFilter);
  addProcessingSignalListener(&m_relationAdder);
  addProcessingSignalListener(&m_extrapolationSelector);
  addProcessingSignalListener(&m_distanceFilterCut);
}

void FilterBasedVXDCDCTrackMergerFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_storeArrayMerger.exposeParameters(moduleParamList, prefix);
  m_allMatcher.exposeParameters(moduleParamList, prefix);
  m_bestMatchSelector.exposeParameters(moduleParamList, prefix);
  m_cutFilter.exposeParameters(moduleParamList, prefix);
  m_relationAdder.exposeParameters(moduleParamList, prefix);
  m_extrapolationSelector.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("extrapolation", prefix));
  m_distanceFilterCut.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("distance", prefix));

  moduleParamList->addParameter("extrapolate", m_param_extrapolate, "Call the extrapolation and fitting routine after the prefilter.",
                                m_param_extrapolate);
  moduleParamList->addParameter("mergeAndExtrapolate", m_param_mergeAndExtrapolate,
                                "Merge the prefiltered combinations immediately and extrapolate afterwards.", m_param_mergeAndExtrapolate);
}

void FilterBasedVXDCDCTrackMergerFindlet::apply()
{
  // Pick up items from the data store.
  std::vector<RecoTrack*> cdcRecoTrackVector;
  std::vector<RecoTrack*> vxdRecoTrackVector;

  m_storeArrayMerger.apply(cdcRecoTrackVector, vxdRecoTrackVector);

  // Prepare the weighted relations for later
  std::vector<TrackFindingCDC::WeightedRelation<RecoTrack*, RecoTrack* const>> weightedRelations;
  weightedRelations.reserve(cdcRecoTrackVector.size() * vxdRecoTrackVector.size());

  // Do all combinations between vxd and cdc tracks
  m_allMatcher.apply(cdcRecoTrackVector, vxdRecoTrackVector, weightedRelations);

  // Cut out only some of them based on a filter decision (probably a mva filter)
  m_cutFilter.apply(weightedRelations);

  if (m_param_extrapolate) {
    // In this case, immediately merge the found combinations of the filter and only go on with the remaining
    // tracks, that do not have a partner.
    if (m_param_mergeAndExtrapolate) {
      // Find the best matching elements
      m_bestMatchSelector.apply(weightedRelations);

      // Add the already found items from the filter-based decision
      m_relationAdder.apply(weightedRelations);

      // Clear all found relations and start from scratch - this time only using those items which were not used before
      m_storeArrayMerger.removeCDCRecoTracksWithPartner(cdcRecoTrackVector);
      m_storeArrayMerger.removeVXDRecoTracksWithPartner(vxdRecoTrackVector);

      // Refill the weighted relations vector
      weightedRelations.clear();
      m_allMatcher.apply(cdcRecoTrackVector, vxdRecoTrackVector, weightedRelations);
    }

    // Calculate a measure based on an extrapolation to the CDC inner wall
    m_extrapolationSelector.apply(weightedRelations);

    // Record what is in the weighted relations so far
    m_distanceFilterCut.apply(weightedRelations);
  }

  // Find the best matching elements
  m_bestMatchSelector.apply(weightedRelations);

  // Add the already found items from the filter-based decision
  m_relationAdder.apply(weightedRelations);
}
