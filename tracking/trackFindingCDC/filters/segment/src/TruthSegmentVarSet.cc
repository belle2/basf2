/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segment/TruthSegmentVarSet.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool TruthSegmentVarSet::extract(const CDCRecoSegment2D* segment)
{
  bool extracted = extractNested(segment);
  if (not extracted or not segment) return false;

  // Find the track with the highest number of hits in the segment
  const CDCMCSegmentLookUp& mcSegmentLookup = CDCMCSegmentLookUp::getInstance();
  const CDCMCHitLookUp& hitLookup = CDCMCHitLookUp::getInstance();

  ITrackType segmentMCMatch = mcSegmentLookup.getMCTrackId(segment);
  bool segmentIsFake = false;
  bool trackIsAlreadyFound = false;


  if (segmentMCMatch == INVALID_ITRACK) {
    segmentIsFake = true;
  } else {


    unsigned int numberOfCorrectHits = 0;
    for (const CDCRecoHit2D& recoHit : *segment) {
      if (hitLookup.getMCTrackId(recoHit->getWireHit().getHit()) == segmentMCMatch) {
        numberOfCorrectHits++;
      }
    }
    if ((double)numberOfCorrectHits / segment->size() < 0.8) {
      segmentIsFake = 1.0;
    } else {
      segmentIsFake = 0.0;
    }
  }

  if (not segmentIsFake) {
    const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
    unsigned int numberOfTakenHitsInThisTrack = 0;
    unsigned int numberOfHitsInThisTrack = 0;

    for (const CDCWireHit& wireHit : wireHitTopology.getWireHits()) {
      if (hitLookup.getMCTrackId(wireHit.getHit()) == segmentMCMatch) {
        numberOfHitsInThisTrack++;
        if (wireHit->getAutomatonCell().hasTakenFlag()) {
          numberOfTakenHitsInThisTrack++;
        }
      }
    }

    if ((double)numberOfTakenHitsInThisTrack / numberOfHitsInThisTrack > 0.5) {
      trackIsAlreadyFound = true;
    }
  }

  var<named("track_is_already_found_truth")>() = trackIsAlreadyFound;
  var<named("segment_is_fake_truth")>() = segmentIsFake;
  var<named("segment_is_new_track_truth")>() = not segmentIsFake and not trackIsAlreadyFound;
  var<named("truth")>() = false; // override in children class
  return true;
}
