/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTriple/SimpleSegmentTripleFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight SimpleSegmentTripleFilter::operator()(const CDCSegmentTriple& segmentTriple)
{
  const CDCAxialSegment2D* ptrStartSegment = segmentTriple.getStartSegment();
  const CDCStereoSegment2D* ptrMiddleSegment = segmentTriple.getMiddleSegment();
  const CDCAxialSegment2D* ptrEndSegment = segmentTriple.getEndSegment();

  assert(ptrStartSegment);
  assert(ptrMiddleSegment);
  assert(ptrEndSegment);

  const CDCAxialSegment2D& startSegment = *ptrStartSegment;
  const CDCStereoSegment2D& middleSegment = *ptrMiddleSegment;
  const CDCAxialSegment2D& endSegment = *ptrEndSegment;

  const double toleranceFraction = 0.0;

  // Check if the middle segment lies within the acceptable bounds in angular deviation
  {
    //get the remembered fits
    const CDCTrajectory2D& startFit = startSegment.getTrajectory2D();
    const CDCTrajectory2D& endFit = endSegment.getTrajectory2D();

    //use only the first and last hit for this check
    const CDCRecoHit2D& firstHit = middleSegment.front();
    const CDCRecoHit2D& lastHit = middleSegment.back();

    Vector3D firstRecoPos = firstHit.reconstruct3D(startFit);
    Vector3D lastRecoPos = lastHit.reconstruct3D(endFit);

    const CDCWire& firstWire = firstHit.getWire();
    const CDCWire& lastWire = lastHit.getWire();

    const bool agrees =
      firstWire.isInCellZBounds(firstRecoPos, toleranceFraction) and
      lastWire.isInCellZBounds(lastRecoPos, toleranceFraction);

    if (not agrees) return NAN;
  }

  // make more complex judgement on fitness

  // Get the combined fit of start and end axial segment
  CDCTrajectory2D trajectory2D =  getFitter2D().fit(*(segmentTriple.getStartSegment()),
                                                    *(segmentTriple.getEndSegment()));

  // Check if the middle segment is actually coaligned with the trajectory
  EForwardBackward fbInfo = trajectory2D.isForwardOrBackwardTo(middleSegment);
  if (fbInfo != EForwardBackward::c_Forward) return NAN;

  // Reconstruct the middle stereo segment
  CDCSegment3D reconstructedMiddle;
  for (const CDCRecoHit2D& recoHit2D : middleSegment) {
    reconstructedMiddle.push_back(CDCRecoHit3D::reconstruct(recoHit2D, trajectory2D));
    if (not reconstructedMiddle.back().isInCellZBounds(toleranceFraction)) {
      B2DEBUG(100, "  RecoHit out of CDC");
      return NAN;
    }
  }

  // Fit the sz slope and intercept
  CDCTrajectorySZ trajectorySZ;
  getSZFitter().update(trajectorySZ, middleSegment, trajectory2D);
  segmentTriple.setTrajectory3D(CDCTrajectory3D(trajectory2D, trajectorySZ));

  Weight result = startSegment.size() + middleSegment.size() + endSegment.size();

  return result;
}
