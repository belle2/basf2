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

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCSegment2DLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void TruthSegmentVarSet::initialize()
{
  CDCMCManager::getInstance().requireTruthInformation();
  Super::initialize();
}

void TruthSegmentVarSet::beginEvent()
{
  CDCMCManager::getInstance().fill();
  Super::beginEvent();
}

bool TruthSegmentVarSet::extract(const CDCSegment2D* segment)
{
  if (not segment) return false;

  // Find the track with the highest number of hits in the segment
  const CDCMCSegment2DLookUp& mcSegmentLookup = CDCMCSegment2DLookUp::getInstance();
  const CDCMCHitLookUp& hitLookup = CDCMCHitLookUp::getInstance();

  ITrackType segmentMCMatch = mcSegmentLookup.getMCTrackId(segment);
  bool segmentIsFake = false;
  bool trackIsAlreadyFound = false;


  if (segmentMCMatch == INVALID_ITRACK) {
    segmentIsFake = true;

  } else {
    unsigned int numberOfCorrectHits = 0;
    for (const CDCRecoHit2D& recoHit : *segment) {
      if (hitLookup.getMCTrackId(recoHit.getWireHit().getHit()) == segmentMCMatch) {
        numberOfCorrectHits++;
      }
    }
    if (numberOfCorrectHits < 0.8 * segment->size()) {
      segmentIsFake = true;
    } else {
      segmentIsFake = false;
    }

  }

  if (not segmentIsFake) {
    // It is a bit suspicuous that this low level objects accesses the DataStore.
    // Maybe this code is the answer to the wrong question.?
    StoreWrappedObjPtr<std::vector<CDCWireHit> > storedWireHits("CDCWireHitVector");
    const std::vector<CDCWireHit>& wireHits = *storedWireHits;
    unsigned int numberOfTakenHitsInThisTrack = 0;
    unsigned int numberOfHitsInThisTrack = 0;

    for (const CDCWireHit& wireHit : wireHits) {
      if (hitLookup.getMCTrackId(wireHit.getHit()) == segmentMCMatch) {
        numberOfHitsInThisTrack++;
        if (wireHit.getAutomatonCell().hasTakenFlag()) {
          numberOfTakenHitsInThisTrack++;
        }
      }
    }

    if (numberOfTakenHitsInThisTrack > 0.5 * numberOfHitsInThisTrack) {
      trackIsAlreadyFound = true;
    }
  }

  var<named("segment_is_fake_truth")>() = segmentIsFake;
  var<named("track_is_already_found_truth")>() = trackIsAlreadyFound;
  var<named("segment_is_new_track_truth")>() = not segmentIsFake and not trackIsAlreadyFound;
  var<named("truth")>() = false; // override in derived classes
  return true;
}
