/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segment_train/SegmentTrainVarSet.h>

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

#include <tracking/trackFindingCDC/trackFinderOutputCombining/MatchingInformation.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool SegmentTrainVarSet::extract(const std::pair<std::vector<SegmentInformation*>, const CDCTrack*>* testPair)
{
  extractNested(testPair);
  const std::vector<SegmentInformation*> segmentTrain = testPair->first;
  const CDCTrack* track = testPair->second;

  const CDCTrajectory2D& trajectory = track->getStartTrajectory3D().getTrajectory2D();

  bool is_stereo = segmentTrain.front()->getSegment()->getStereoType() != StereoType_c::Axial;

  var<named("is_stereo")>() = is_stereo;
  var<named("size")>() = segmentTrain.size();

  double maximumPerpSOverlap = -999;
  double sumPerpSOverlap = 0;

  double lastPerpS;
  bool alreadySet = false;

  const CDCTrajectory2D& trajectory2D = track->getStartTrajectory3D().getTrajectory2D();

  for (const SegmentInformation* segmentInformation : segmentTrain) {
    double perpSFront = 0;
    if (is_stereo) {
      CDCRLWireHit rlWireHit(segmentInformation->getSegment()->front().getWireHit(),
                             segmentInformation->getSegment()->front().getRLInfo());
      CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trajectory);
      perpSFront = recoHit3D.getPerpS();
    } else {
      perpSFront = trajectory2D.calcPerpS(segmentInformation->getSegment()->front().getRecoPos2D());
    }
    if (std::isnan(perpSFront)) {
      var<named("maximum_perpS_overlap")>() = 0;
      var<named("perpS_overlap_mean")>() = 0;
      var<named("calculation_failed")>() = true;
      return true;
    }
    if (alreadySet) {
      double currentOverlap = (1 - m_param_percentageForPerpSMeasurements) * lastPerpS - perpSFront;
      sumPerpSOverlap += currentOverlap;
      if (currentOverlap > maximumPerpSOverlap) {
        maximumPerpSOverlap = currentOverlap;
      }
    }
    alreadySet = true;
    if (is_stereo) {
      CDCRLWireHit rlWireHit(segmentInformation->getSegment()->back().getWireHit(), segmentInformation->getSegment()->back().getRLInfo());
      CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trajectory);
      lastPerpS = recoHit3D.getPerpS();
    } else {
      lastPerpS = trajectory2D.calcPerpS(segmentInformation->getSegment()->back().getRecoPos2D());
    }

    if (std::isnan(lastPerpS)) {
      var<named("maximum_perpS_overlap")>() = 0;
      var<named("perpS_overlap_mean")>() = 0;
      var<named("calculation_failed")>() = true;
      return true;
    }
  }

  var<named("calculation_failed")>() = false;
  var<named("maximum_perpS_overlap")>() = maximumPerpSOverlap;
  if (segmentTrain.size() > 1)
    var<named("perpS_overlap_mean")>() = sumPerpSOverlap / (segmentTrain.size() - 1);
  else
    var<named("perpS_overlap_mean")>() = 0;

  return true;
}
