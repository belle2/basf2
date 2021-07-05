/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTrack/SegmentTrackTruthVarSet.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCSegment2DLookUp.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/filters/base/Filter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool SegmentTrackTruthVarSet::extract(const BaseSegmentTrackFilter::Object* testPair)
{
  if (not testPair) return false;

  const CDCTrack* track = testPair->getFrom();
  const CDCSegment2D* segment = testPair->getTo();

  var<named("belongs_to_same_track_truth")>() = 0.0;

  const CDCMCHitLookUp& mcHitLookup =  CDCMCHitLookUp::getInstance();

  // Find the track with the highest number of hits in the segment
  const CDCMCSegment2DLookUp& mcSegmentLookup = CDCMCSegment2DLookUp::getInstance();
  ITrackType segmentMCMatch = mcSegmentLookup.getMCTrackId(segment);
  if (segmentMCMatch == INVALID_ITRACK) {
    var<named("segment_is_fake_truth")>() = 1.0;
    var<named("segment_purity_truth")>() = 0.0;
    var<named("track_purity_truth")>() = 0.0;

  } else {
    var<named("segment_is_fake_truth")>() = 0.0;

    double numberOfCorrectHits = 0;
    for (const CDCRecoHit3D& recoHit : *track) {
      if (mcHitLookup.getMCTrackId(recoHit.getWireHit().getHit()) == segmentMCMatch) {
        numberOfCorrectHits++;
      }
    }

    var<named("track_purity_truth")>() = numberOfCorrectHits / track->size();

    if (numberOfCorrectHits / track->size() > 0.5) {
      var<named("belongs_to_same_track_truth")>() = 1.0;
    } else {
      var<named("belongs_to_same_track_truth")>() = 0.0;
    }
  }


  var<named("truth")>() = var<named("belongs_to_same_track_truth")>();

  return true;
}
