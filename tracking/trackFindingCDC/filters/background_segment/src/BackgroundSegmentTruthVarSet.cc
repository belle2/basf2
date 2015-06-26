/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/background_segment/BackgroundSegmentTruthVarSet.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>

#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool BackgroundSegmentTruthVarSet::extract(const CDCRecoSegment2D* segment)
{
  bool extracted = extractNested(segment);
  if (not extracted or not segment) return false;

  // Find the track with the highest number of hits in the segment
  const CDCMCSegmentLookUp& mcSegmentLookup = CDCMCSegmentLookUp::getInstance();
  ITrackType segmentMCMatch = mcSegmentLookup.getMCTrackId(segment);
  if (segmentMCMatch == INVALID_ITRACK) {
    var<named("segment_is_fake_truth")>() = 1.0;
  } else {
    const CDCMCHitLookUp& hitLookup = CDCMCHitLookUp::getInstance();

    unsigned int numberOfCorrectHits = 0;
    for (const CDCRecoHit2D& recoHit : segment->items()) {
      if (hitLookup.getMCTrackId(recoHit->getWireHit().getHit()) == segmentMCMatch) {
        numberOfCorrectHits ++;
      }
    }
    if ((double)numberOfCorrectHits / segment->size() < 0.8) {
      var<named("segment_is_fake_truth")>() = 1.0;
    } else {
      var<named("segment_is_fake_truth")>() = 0.0;
    }
  }

  var<named("truth")>() = 1 - var<named("segment_is_fake_truth")>();
  return true;
}
