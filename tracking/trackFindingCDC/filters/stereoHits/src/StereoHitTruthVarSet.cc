/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitTruthVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool StereoHitTruthVarSet::extract(const std::pair<const CDCRecoHit3D*, const CDCTrack*>* testPair)
{
  const CDCRecoHit3D* recoHit3D = testPair->first;
  const CDCTrack* track = testPair->second;

  if (not testPair or not recoHit3D or not track) return false;

  const CDCMCTrackLookUp& mcTrackLookup = CDCMCTrackLookUp::getInstance();
  const CDCMCHitLookUp& hitLookup = CDCMCHitLookUp::getInstance();

  const Belle2::CDCHit* hit = recoHit3D->getWireHit().getHit();

  ITrackType trackMCMatch = mcTrackLookup.getMCTrackId(track);
  ITrackType hitMCMatch = hitLookup.getMCTrackId(hit);
  ERightLeft hitMCRLInfo = hitLookup.getRLInfo(recoHit3D->getWireHit().getHit());

  if (trackMCMatch == INVALID_ITRACK) {
    var<named("track_is_fake_truth")>() = true;
    var<named("truth_may_reversed")>() = false;
    var<named("truth")>() = false;
  } else {
    var<named("track_is_fake_truth")>() = false;
    var<named("truth_may_reversed")>() = trackMCMatch == hitMCMatch;
    var<named("truth")>() = trackMCMatch == hitMCMatch and hitMCRLInfo == recoHit3D->getRLInfo();
  }

  return true;
}
