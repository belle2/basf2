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
#include <tracking/trackFindingCDC/mclookup/CDCMCSegment2DLookUp.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool StereoSegmentTruthVarSet::extract(const std::pair<const CDCRecoSegment3D*, const CDCTrack*>* testPair)
{
  if (not testPair) return false;
  const CDCRecoSegment3D& recoSegment3D = *testPair->first;
  const CDCTrack& track = *testPair->second;

  const CDCMCTrackLookUp& mcTrackLookup = CDCMCTrackLookUp::getInstance();
  const CDCMCSegment2DLookUp& segmentLookUp = CDCMCSegment2DLookUp::getInstance();

  const ITrackType trackMCMatch = mcTrackLookup.getMCTrackId(&track);

  // FIXME : Segment look up does not support the direct look up of CDCRecoSegment3D
  // Projecting down to CDCRecoSegment2D for now.
  const CDCRecoSegment2D recoSegment2D = recoSegment3D.stereoProjectToRef();
  const ITrackType segmentMCTrack = segmentLookUp.getMCTrackId(&recoSegment2D);

  if (trackMCMatch == INVALID_ITRACK) {
    var<named("track_is_fake_truth")>() = true;
    var<named("truth")>() = false;
  } else {
    var<named("track_is_fake_truth")>() = false;
    var<named("truth")>() = trackMCMatch == segmentMCTrack;
  }

  return true;
}
