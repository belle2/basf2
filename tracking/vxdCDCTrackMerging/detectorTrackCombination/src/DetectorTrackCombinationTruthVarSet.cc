/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Malwin Weiler, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/vxdCDCTrackMerging/detectorTrackCombination/DetectorTrackCombinationTruthVarSet.h>
#include <tracking/mcMatcher/TrackMatchLookUp.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool DetectorTrackCombinationTruthVarSet::extract(const BaseDetectorTrackCombinationFilter::Object* pair)
{
  const RecoTrack* cdcTrack = *(pair->first);
  const RecoTrack* vxdTrack = *(pair->second);

  if (not pair or not cdcTrack or not vxdTrack) return false;

  TrackMatchLookUp mcCDCMatchLookUp("MCRecoTracks", "CDCRecoTracks");
  TrackMatchLookUp mcVXDMatchLookUp("MCRecoTracks", "VXDRecoTracks");
  const RecoTrack* cdcMCTrack = mcCDCMatchLookUp.getMatchedMCRecoTrack(*cdcTrack);
  const RecoTrack* vxdMCTrack = mcCDCMatchLookUp.getMatchedMCRecoTrack(*vxdTrack);

  if (cdcMCTrack == vxdMCTrack && cdcMCTrack != nullptr) {
    var<named("truth")>() = true;
  } else {
    var<named("truth")>() = false;
  }

  return true;
}
