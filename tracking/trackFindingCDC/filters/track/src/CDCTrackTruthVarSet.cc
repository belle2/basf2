/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/track/CDCTrackTruthVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool CDCTrackTruthVarSet::extract(const CDCTrack* track)
{
  if (not track) return false;

  // Find the track with the highest number of hits in the segment
  const CDCMCTrackLookUp& mcTrackLookup = CDCMCTrackLookUp::getInstance();
  const CDCMCHitLookUp& hitLookup = CDCMCHitLookUp::getInstance();

  ITrackType trackMCMatch = mcTrackLookup.getMCTrackId(track);
  bool trackIsFake = false;

  if (trackMCMatch == INVALID_ITRACK) {
    trackIsFake = true;
  } else {

    unsigned int numberOfCorrectHits = 0;
    for (const CDCRecoHit3D& recoHit : *track) {
      if (hitLookup.getMCTrackId(recoHit->getWireHit().getHit()) == trackMCMatch) {
        numberOfCorrectHits++;
      }
    }
    if ((double)numberOfCorrectHits / track->size() < 0.8) {
      trackIsFake = 1.0;
    } else {
      trackIsFake = 0.0;
    }
  }

  var<named("track_is_fake_truth")>() = trackIsFake;
  var<named("truth")>() = not trackIsFake;
  return true;
}
