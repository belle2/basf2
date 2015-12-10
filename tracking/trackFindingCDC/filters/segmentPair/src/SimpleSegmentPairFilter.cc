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

#include <framework/logging/Logger.h>
#include <tracking/trackFindingCDC/fitting/CDCAxialStereoFusion.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

SimpleSegmentPairFilter::SimpleSegmentPairFilter() : m_riemannFitter()
{
  m_riemannFitter.useOnlyOrientation();
}

CellWeight SimpleSegmentPairFilter::operator()(const CDCSegmentPair& segmentPair)
{
  const CDCAxialRecoSegment2D* ptrStartSegment = segmentPair.getStartSegment();
  const CDCAxialRecoSegment2D* ptrEndSegment = segmentPair.getEndSegment();

  assert(ptrStartSegment);
  assert(ptrEndSegment);

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  // Do fits
  const CDCTrajectory2D& startFit = getFittedTrajectory2D(startSegment);
  const CDCTrajectory2D& endFit = getFittedTrajectory2D(endSegment);

  // Check if segments are coaligned
  bool endSegmentIsCoaligned = startFit.getTotalArcLength2D(endSegment) >= 0.0;
  bool startSegmentIsCoaligned = endFit.getTotalArcLength2D(startSegment) >= 0.0;

  if (not endSegmentIsCoaligned or not startSegmentIsCoaligned) {
    return NOT_A_CELL;
  }

  // Check if there is a positive gap between start and end segment
  double startFitGap = startFit.getArcLength2DGap(startSegment, endSegment);
  double endFitGap = endFit.getArcLength2DGap(startSegment, endSegment);

  if (startFitGap < -5 or startFitGap > 50 or endFitGap < -5 or endFitGap > 50) {
    return NOT_A_CELL;
  }

  double startFitFrontOffset = startFit.getArcLength2DFrontOffset(startSegment, endSegment);
  double endFitBackOffset = endFit.getArcLength2DBackOffset(startSegment, endSegment);

  if (startFitFrontOffset < 0 or
      startFitFrontOffset > 50 or
      endFitBackOffset < 0 or
      endFitBackOffset > 50) {
    return NOT_A_CELL;
  }

  Vector2D startBackRecoPos2D = startSegment.back().getRecoPos2D();
  Vector2D endFrontRecoPos2D = endSegment.front().getRecoPos2D();

  // Momentum agreement cut
  Vector2D startMom2DAtStartBack = startFit.getUnitMom2D(startBackRecoPos2D);
  Vector2D endMom2DAtEndFront = endFit.getUnitMom2D(endFrontRecoPos2D);

  Vector2D startMom2DAtEndFront = startFit.getUnitMom2D(endFrontRecoPos2D);
  Vector2D endMom2DAtStartBack = endFit.getUnitMom2D(startBackRecoPos2D);

  double momAngleAtStartBack = startMom2DAtStartBack.angleWith(endMom2DAtStartBack);
  double momAngleAtEndFront = endMom2DAtEndFront.angleWith(startMom2DAtEndFront);

  if (fabs(momAngleAtEndFront) > 1.0 or fabs(momAngleAtStartBack) > 1.0) {
    return NOT_A_CELL;
  }

  // Proximity cut
  double startFit_dist2DToFront_endSegment = startFit.getDist2D(endSegment.front().getRecoPos2D());
  double endFit_dist2DToBack_startSegment = endFit.getDist2D(startSegment.back().getRecoPos2D());

  if (startFit_dist2DToFront_endSegment < 10 and  endFit_dist2DToBack_startSegment < 10) {
    getFittedTrajectory3D(segmentPair);
    return startSegment.size() + endSegment.size();
  } else {
    return NOT_A_CELL;
  }

}


const CDCTrajectory2D& SimpleSegmentPairFilter::getFittedTrajectory2D(const CDCAxialRecoSegment2D& segment) const
{

  CDCTrajectory2D& trajectory2D = segment.getTrajectory2D();
  if (not trajectory2D.isFitted()) {
    getRiemannFitter().update(trajectory2D, segment);
  }
  return trajectory2D;

}


const CDCTrajectory3D& SimpleSegmentPairFilter::getFittedTrajectory3D(const CDCSegmentPair&
    segmentPair) const
{
  const CDCAxialRecoSegment2D* ptrStartSegment = segmentPair.getStartSegment();
  const CDCAxialRecoSegment2D* ptrEndSegment = segmentPair.getEndSegment();

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  //Do fits if still necessary.
  getFittedTrajectory2D(startSegment);
  getFittedTrajectory2D(endSegment);

  CDCAxialStereoFusion::reconstructFuseTrajectories(segmentPair);
  return segmentPair.getTrajectory3D();

}
