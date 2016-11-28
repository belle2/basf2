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

#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCSegment3DLookUp.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool StereoSegmentTruthVarSet::extract(const std::pair<const CDCSegment3D*, const CDCTrack*>* testPair)
{
  if (not testPair) return false;
  const CDCSegment3D& segment3D = *testPair->first;
  const CDCTrack& track = *testPair->second;

  const CDCMCTrackLookUp& mcTrackLookup = CDCMCTrackLookUp::getInstance();
  const CDCMCSegment3DLookUp& segment3DLookUp = CDCMCSegment3DLookUp::getInstance();

  const ITrackType trackMCMatch = mcTrackLookup.getMCTrackId(&track);
  const ITrackType segment3DMCTrack = segment3DLookUp.getMCTrackId(&segment3D);

  if (trackMCMatch == INVALID_ITRACK) {
    var<named("track_is_fake_truth")>() = true;
    var<named("truth")>() = false;
  } else {
    var<named("track_is_fake_truth")>() = false;
    var<named("truth")>() = trackMCMatch == segment3DMCTrack;
  }

  return true;
}
