/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTrain/SimpleSegmentTrainFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/trackFinderOutputCombining/MatchingInformation.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight SimpleSegmentTrainFilter::operator()(const std::pair<std::vector<SegmentInformation*>, const CDCTrack*>& testPair)
{
  bool extracted = Super::operator()(testPair);
  if (not extracted) return false;

  const std::vector<SegmentInformation*>& trainOfSegments = testPair.first;
  const CDCTrack* track = testPair.second;
  const CDCTrajectory2D& trajectory = track->getStartTrajectory3D().getTrajectory2D();

  double lastPerpS;
  bool alreadySet = false;

  for (const SegmentInformation* segmentInformation : trainOfSegments) {
    double perpSFront = trajectory.calcArcLength2D(segmentInformation->getSegment()->front().getRecoPos2D());
    if (alreadySet and perpSFront < (1 - SegmentTrainVarSet::m_param_percentageForPerpSMeasurements) * lastPerpS) {
      // Means: no
      return NAN;
    }
    alreadySet = true;
    lastPerpS = trajectory.calcArcLength2D(segmentInformation->getSegment()->back().getRecoPos2D());
  }

  // Means: yes
  return 1.0;
}
