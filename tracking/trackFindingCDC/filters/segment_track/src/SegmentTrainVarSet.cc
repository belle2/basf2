/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segment_track/SegmentTrainVarSet.h>

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

#include <tracking/trackFinderOutputCombining/MatchingInformation.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackFinderOutputCombining;

bool SegmentTrainVarSet::extract(const std::pair<std::vector<SegmentInformation*>, const CDCTrack*>* testPair)
{
  extractNested(testPair);
  const std::vector<SegmentInformation*> segmentTrain = testPair->first;
  const CDCTrack* track = testPair->second;

  bool is_stereo = segmentTrain.front()->getSegment()->getStereoType() != AXIAL;

  double maximumPerpSOverlap = 0;
  double sumPerpSOverlap = 0;

  double lastPerpS;
  bool alreadySet = false;

  const CDCTrajectory2D& trajectory2D = track->getStartTrajectory3D().getTrajectory2D();

  for (const SegmentInformation* segmentInformation : segmentTrain) {
    double perpSFront = trajectory2D.calcPerpS(segmentInformation->getSegment()->front().getRecoPos2D());
    if (alreadySet) {
      double currentOverlap = lastPerpS - perpSFront;
      sumPerpSOverlap += currentOverlap;
      if (currentOverlap > maximumPerpSOverlap) {
        maximumPerpSOverlap = currentOverlap;
      }
    }
    alreadySet = true;
    lastPerpS = trajectory2D.calcPerpS(segmentInformation->getSegment()->back().getRecoPos2D());
  }

  // TODO
  var<named("fit_prob")>() = 0;
  var<named("is_stereo")>() = is_stereo;
  var<named("size")>() = segmentTrain.size();
  var<named("maximum_perpS_overlap")>() = maximumPerpSOverlap;
  if (segmentTrain.size() > 1)
    var<named("perpS_overlap_mean")>() = sumPerpSOverlap / (segmentTrain.size() - 1);
  else
    var<named("perpS_overlap_mean")>() = 0;

  return true;
}
