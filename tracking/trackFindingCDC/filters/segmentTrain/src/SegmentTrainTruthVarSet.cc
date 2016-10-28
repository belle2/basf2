/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTrain/SegmentTrainTruthVarSet.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>
#include <tracking/trackFindingCDC/trackFinderOutputCombining/MatchingInformation.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/mclookup/ITrackType.h>

#include <cassert>

using namespace Belle2;
using namespace TrackFindingCDC;

bool SegmentTrainTruthVarSet::extract(const
                                      std::pair<std::vector<SegmentInformation*>, const CDCTrack*>* testPair)
{
  bool extracted = extractNested(testPair);
  if (not extracted or not testPair) return false;

  const std::vector<SegmentInformation*> segmentTrain = testPair->first;

  var<named("belongs_to_same_track_truth")>() = true;

  ITrackType firstMCID = INVALID_ITRACK;

  // Find the track with the highest number of hits in the segment
  for (const SegmentInformation* segment : segmentTrain) {
    const CDCMCSegmentLookUp& mcSegmentLookup = CDCMCSegmentLookUp::getInstance();
    ITrackType segmentMCMatch = mcSegmentLookup.getMCTrackId(segment->getSegment());
    if (segmentMCMatch == INVALID_ITRACK) {
      var<named("belongs_to_same_track_truth")>() = false;
      break;
    } else if (firstMCID == INVALID_ITRACK) {
      firstMCID = segmentMCMatch;
    } else if (firstMCID != segmentMCMatch) {
      var<named("belongs_to_same_track_truth")>() = false;
      break;
    }
  }

  var<named("truth")>() = var<named("belongs_to_same_track_truth")>();

  return true;
}
