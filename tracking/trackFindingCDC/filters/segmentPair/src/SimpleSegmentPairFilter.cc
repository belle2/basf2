/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/SimpleSegmentPairFilter.h>

#include <tracking/trackFindingCDC/fitting/CDCAxialStereoFusion.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight SimpleSegmentPairFilter::operator()(const CDCSegmentPair& segmentPair)
{
  const CDCSegment2D* ptrFromSegment = segmentPair.getFromSegment();
  const CDCSegment2D* ptrToSegment = segmentPair.getToSegment();

  assert(ptrFromSegment);
  assert(ptrToSegment);

  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;

  // Do fits
  const CDCTrajectory2D& fromFit = getFittedTrajectory2D(fromSegment);
  const CDCTrajectory2D& toFit = getFittedTrajectory2D(toSegment);

  // Check if segments are coaligned
  bool toSegmentIsCoaligned = fromFit.getTotalArcLength2D(toSegment) >= 0.0;
  bool fromSegmentIsCoaligned = toFit.getTotalArcLength2D(fromSegment) >= 0.0;

  if (not toSegmentIsCoaligned or not fromSegmentIsCoaligned) {
    return NAN;
  }

  // Check if there is a positive gap between from and to segment
  double fromFitGap = fromFit.getArcLength2DGap(fromSegment, toSegment);
  double toFitGap = toFit.getArcLength2DGap(fromSegment, toSegment);

  if (fromFitGap < -5 or fromFitGap > 50 or toFitGap < -5 or toFitGap > 50) {
    return NAN;
  }

  double fromFitFrontOffset = fromFit.getArcLength2DFrontOffset(fromSegment, toSegment);
  double toFitBackOffset = toFit.getArcLength2DBackOffset(fromSegment, toSegment);

  if (fromFitFrontOffset < 0 or
      fromFitFrontOffset > 50 or
      toFitBackOffset < 0 or
      toFitBackOffset > 50) {
    return NAN;
  }

  Vector2D fromBackRecoPos2D = fromSegment.back().getRecoPos2D();
  Vector2D toFrontRecoPos2D = toSegment.front().getRecoPos2D();

  // Momentum agreement cut
  Vector2D fromMom2DAtFromBack = fromFit.getFlightDirection2D(fromBackRecoPos2D);
  Vector2D toMom2DAtToFront = toFit.getFlightDirection2D(toFrontRecoPos2D);

  Vector2D fromMom2DAtToFront = fromFit.getFlightDirection2D(toFrontRecoPos2D);
  Vector2D toMom2DAtFromBack = toFit.getFlightDirection2D(fromBackRecoPos2D);

  double momAngleAtFromBack = fromMom2DAtFromBack.angleWith(toMom2DAtFromBack);
  double momAngleAtToFront = toMom2DAtToFront.angleWith(fromMom2DAtToFront);

  if (fabs(momAngleAtToFront) > 1.0 or fabs(momAngleAtFromBack) > 1.0) {
    return NAN;
  }

  // Proximity cut
  double fromFit_dist2DToFront_toSegment = fromFit.getDist2D(toSegment.front().getRecoPos2D());
  double toFit_dist2DToBack_fromSegment = toFit.getDist2D(fromSegment.back().getRecoPos2D());

  if (fromFit_dist2DToFront_toSegment < 10 and  toFit_dist2DToBack_fromSegment < 10) {
    getFittedTrajectory3D(segmentPair);
    return fromSegment.size() + toSegment.size();
  } else {
    return NAN;
  }
}

const CDCTrajectory2D& SimpleSegmentPairFilter::getFittedTrajectory2D(const CDCSegment2D& segment) const
{
  CDCTrajectory2D& trajectory2D = segment.getTrajectory2D();
  if (not trajectory2D.isFitted()) {
    getRiemannFitter().update(trajectory2D, segment);
  }
  return trajectory2D;
}

const CDCTrajectory3D& SimpleSegmentPairFilter::getFittedTrajectory3D(const CDCSegmentPair& segmentPair) const
{
  const CDCSegment2D* ptrFromSegment = segmentPair.getFromSegment();
  const CDCSegment2D* ptrToSegment = segmentPair.getToSegment();

  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;

  // Do fits if still necessary.
  getFittedTrajectory2D(fromSegment);
  getFittedTrajectory2D(toSegment);

  CDCAxialStereoFusion fusionFit;
  fusionFit.reconstructFuseTrajectories(segmentPair);
  return segmentPair.getTrajectory3D();
}
