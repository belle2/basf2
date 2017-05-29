/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcTrackSpacePointCombination/CDCVXDTrackCombinationTruthVarSet.h>
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

  // Count the number of times the CDC-related MC-track is also related to the SVD cluster.
  unsigned int numberOfCorrectHits = 0;

  for (const SpacePoint* spacePoint : result->second) {
    for (const SVDCluster& relatedCluster : spacePoint->getRelationsWith<SVDCluster>()) {
      const auto& relatedMCRecoTracksToOneCluster = relatedCluster.getRelationsWith<RecoTrack>("MCRecoTracks");

      bool cdcTrackMatchedToCluster = std::any_of(relatedMCRecoTracksToOneCluster.begin(),
      relatedMCRecoTracksToOneCluster.end(), [cdcMCTrack](const RecoTrack & mcRecoTrack) {
        return &mcRecoTrack == cdcMCTrack;
      });

      if (cdcTrackMatchedToCluster) {
        numberOfCorrectHits++;
      }
    }
  }

  var<named("truth_number_of_correct_hits")>() = numberOfCorrectHits;
  var<named("truth_number_of_mc_hits")>() = cdcMCTrack->getNumberOfSVDHits();
  var<named("truth")>() = numberOfCorrectHits == 2 * result->second.size() and numberOfCorrectHits > 0;

  return true;
}
