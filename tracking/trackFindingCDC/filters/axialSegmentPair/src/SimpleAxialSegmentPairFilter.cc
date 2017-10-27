/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/axialSegmentPair/SimpleAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

SimpleAxialSegmentPairFilter::SimpleAxialSegmentPairFilter() : m_riemannFitter()
{
  m_riemannFitter.useOnlyOrientation();
}

Weight SimpleAxialSegmentPairFilter::operator()(const CDCAxialSegmentPair& axialSegmentPair)
{
  const CDCAxialSegment2D* ptrStartSegment = axialSegmentPair.getStartSegment();
  const CDCAxialSegment2D* ptrEndSegment = axialSegmentPair.getEndSegment();

  assert(ptrStartSegment);
  assert(ptrEndSegment);

  const CDCAxialSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialSegment2D& endSegment = *ptrEndSegment;

  // Do fits
  const CDCTrajectory2D& startFit = getFittedTrajectory2D(startSegment);
  const CDCTrajectory2D& endFit = getFittedTrajectory2D(endSegment);

  // Check if segments are coaligned
  bool endSegmentIsCoaligned = startFit.getTotalArcLength2D(endSegment) >= 0.0;
  bool startSegmentIsCoaligned = endFit.getTotalArcLength2D(startSegment) >= 0.0;

  if (not endSegmentIsCoaligned or not startSegmentIsCoaligned) {
    return NAN;
  }

  // Check if there is a positive gap between start and end segment
  double startFitGap = startFit.getArcLength2DGap(startSegment, endSegment);
  double endFitGap = endFit.getArcLength2DGap(startSegment, endSegment);

  if (startFitGap < 0 or startFitGap > 100 or endFitGap < 0 or endFitGap > 100) {
    return NAN;
  }

  double startFitFrontOffset = startFit.getArcLength2DFrontOffset(startSegment, endSegment);
  double endFitBackOffset = endFit.getArcLength2DBackOffset(startSegment, endSegment);

  if (startFitFrontOffset < 0 or endFitBackOffset < 0) {
    return NAN;
  }

  Vector2D startBackRecoPos2D = startSegment.back().getRecoPos2D();
  Vector2D endFrontRecoPos2D = endSegment.front().getRecoPos2D();

  // Momentum agreement cut
  Vector2D startMom2DAtStartBack = startFit.getFlightDirection2D(startBackRecoPos2D);
  Vector2D endMom2DAtEndFront = endFit.getFlightDirection2D(endFrontRecoPos2D);

  Vector2D startMom2DAtEndFront = startFit.getFlightDirection2D(endFrontRecoPos2D);
  Vector2D endMom2DAtStartBack = endFit.getFlightDirection2D(startBackRecoPos2D);

  double momAngleAtStartBack = startMom2DAtStartBack.angleWith(endMom2DAtStartBack);
  double momAngleAtEndFront = endMom2DAtEndFront.angleWith(startMom2DAtEndFront);

  if (fabs(momAngleAtEndFront) > 2.0 or fabs(momAngleAtStartBack) > 2.0) {
    return NAN;
  }

  // Proximity cut
  double startFit_dist2DToFront_endSegment = startFit.getDist2D(endFrontRecoPos2D);
  double endFit_dist2DToBack_startSegment = endFit.getDist2D(startBackRecoPos2D);

  if (startFit_dist2DToFront_endSegment < 6 and endFit_dist2DToBack_startSegment < 6) {
    return startSegment.size() + endSegment.size();
  } else {
    return NAN;
  }

}



const CDCTrajectory2D& SimpleAxialSegmentPairFilter::getFittedTrajectory2D(const CDCAxialSegment2D& segment) const
{

  CDCTrajectory2D& trajectory2D = segment.getTrajectory2D();
  if (not trajectory2D.isFitted()) {
    getRiemannFitter().update(trajectory2D, segment);
  }
  return trajectory2D;

}



const CDCTrajectory2D& SimpleAxialSegmentPairFilter::getFittedTrajectory2D(const CDCAxialSegmentPair&
    axialSegmentPair) const
{
  CDCTrajectory2D& trajectory2D = axialSegmentPair.getTrajectory2D();
  if (not trajectory2D.isFitted()) {
    getRiemannFitter().update(trajectory2D, axialSegmentPair);
  }
  return trajectory2D;
}
