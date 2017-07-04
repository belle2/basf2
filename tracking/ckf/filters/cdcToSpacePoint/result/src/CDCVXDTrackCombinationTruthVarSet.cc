/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcToSpacePoint/result/CDCVXDTrackCombinationTruthVarSet.h>
#include <tracking/ckf/utilities/CKFMCUtils.h>
#include <tracking/mcMatcher/TrackMatchLookUp.h>
#include <framework/dataobjects/EventMetaData.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool CDCVXDTrackCombinationTruthVarSet::extract(const BaseCDCVXDTrackCombinationFilter::Object* result)
{
  RecoTrack* cdcTrack = result->first;
  if (not cdcTrack) return false;

  StoreObjPtr<EventMetaData> eventMetaData;
  var<named("truth_event_number")>() = eventMetaData->getEvent();
  var<named("truth_cdc_number")>() = cdcTrack->getArrayIndex();

  const std::string& cdcTrackStoreArrayName = cdcTrack->getArrayName();

  TrackMatchLookUp mcCDCMatchLookUp("MCRecoTracks", cdcTrackStoreArrayName);
  const RecoTrack* cdcMCTrack = mcCDCMatchLookUp.getMatchedMCRecoTrack(*cdcTrack);

  // Default to false
  var<named("truth_number_of_correct_hits")>() = 0;
  var<named("truth")>() = 0;
  var<named("truth_number_of_mc_hits")>() = 0;

  if (not cdcMCTrack) {
    // CDC track is a fake.
    return true;
  }

  // Count the number of times the CDC-related MC-track is also related to the clusters.
  const unsigned int numberOfCorrectHits = getNumberOfCorrectHits(cdcMCTrack, result->second);

  var<named("truth_number_of_correct_hits")>() = numberOfCorrectHits;
  var<named("truth_number_of_mc_hits")>() = cdcMCTrack->getNumberOfSVDHits() + cdcMCTrack->getNumberOfPXDHits();
  var<named("truth")>() = numberOfCorrectHits == 2 * result->second.size() and numberOfCorrectHits > 0;

  return true;
}
