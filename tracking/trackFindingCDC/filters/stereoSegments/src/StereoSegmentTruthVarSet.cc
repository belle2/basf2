/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/stereoSegments/StereoSegmentTruthVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool StereoSegmentTruthVarSet::extract(const std::pair<std::pair<const CDCRecoSegment2D*, const CDCRecoSegment3D>, const CDCTrack&>*
                                       testPair)
{
  bool extracted = extractNested(testPair);
  if (not extracted or not testPair) return false;

  const std::pair<const CDCRecoSegment2D*, const CDCRecoSegment3D&>& recoSegments = testPair->first;
  const CDCTrack& track = testPair->second;

  const CDCRecoSegment2D* recoSegment2D = recoSegments.first;

  const CDCMCTrackLookUp& mcTrackLookup = CDCMCTrackLookUp::getInstance();
  const CDCMCSegmentLookUp& segmentLookUp = CDCMCSegmentLookUp::getInstance();

  const ITrackType trackMCMatch = mcTrackLookup.getMCTrackId(&track);
  const ITrackType segmentMCTrack = segmentLookUp.getMCTrackId(recoSegment2D);

  if (trackMCMatch == INVALID_ITRACK) {
    var<named("track_is_fake_truth")>() = true;
    var<named("truth")>() = false;
  } else {
    var<named("track_is_fake_truth")>() = false;
    var<named("truth")>() = trackMCMatch == segmentMCTrack;
  }

  return true;
}
