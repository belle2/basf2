/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/FitSegmentPairVarSet.h>
#include <cassert>

#include <tracking/trackFindingCDC/fitting/CDCAxialStereoFusion.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>


using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool FitSegmentPairVarSet::extract(const CDCSegmentPair* ptrSegmentPair)
{
  bool extracted = extractNested(ptrSegmentPair);
  if (not extracted or not ptrSegmentPair) return false;

  const CDCSegmentPair segmentPair = *ptrSegmentPair;

  const CDCRecoSegment2D* ptrStartSegment = segmentPair.getFromSegment();
  const CDCRecoSegment2D* ptrEndSegment = segmentPair.getToSegment();

  assert(ptrStartSegment);
  assert(ptrEndSegment);

  const CDCRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCRecoSegment2D& endSegment = *ptrEndSegment;

  Vector2D startBackRecoPos2D = startSegment.back().getRecoPos2D();
  Vector2D endFrontRecoPos2D = endSegment.front().getRecoPos2D();

  CDCTrajectory2D& startFit =  startSegment.getTrajectory2D();
  CDCTrajectory2D& endFit = endSegment.getTrajectory2D();

  if (not startFit.isFitted()) {
    getRiemannFitter().update(startFit, startSegment);
  }

  if (not endFit.isFitted()) {
    getRiemannFitter().update(endFit, endSegment);
  }

  CDCAxialStereoFusion fusionFit;
  fusionFit.reconstructFuseTrajectories(segmentPair);
  const CDCTrajectory3D& commonFit3D = segmentPair.getTrajectory3D();
  const CDCTrajectory2D commonFit = commonFit3D.getTrajectory2D();

  const CDCAxialRecoSegment2D* ptrAxialSegment = segmentPair.getAxialSegment();
  const CDCStereoRecoSegment2D* ptrStereoSegment = segmentPair.getStereoSegment();

  const CDCAxialRecoSegment2D& axialSegment = *ptrAxialSegment;
  const CDCStereoRecoSegment2D& stereoSegment = *ptrStereoSegment;

  const CDCTrajectory2D& axialFit = axialSegment.getTrajectory2D();
  const CDCSZFitter& szFitter = CDCSZFitter::getFitter();

  const CDCTrajectorySZ szFit = szFitter.fit(stereoSegment, axialFit);

  // Collect decision criteria

  // Super layers numbers of fits
  var<named("startFit_startISuperLayer")>() = startFit.getStartISuperLayer();
  var<named("endFit_startISuperLayer")>() = endFit.getStartISuperLayer();

  // Super layer coarse extrapolation
  var<named("startFit_nextISuperLayer")>() = startFit.getNextISuperLayer();
  var<named("startFit_nextAxialISuperLayer")>() = startFit.getNextAxialISuperLayer();

  var<named("endFit_previousISuperLayer")>() = endFit.getPreviousISuperLayer();
  var<named("endFit_previousAxialISuperLayer")>() = endFit.getPreviousAxialISuperLayer();


  // Coalignment indicators
  var<named("startFit_totalPerpS_startSegment")>() = startFit.getTotalArcLength2D(startSegment);
  var<named("endFit_totalPerpS_startSegment")>() = endFit.getTotalArcLength2D(startSegment);
  var<named("commonFit_totalPerpS_startSegment")>() = commonFit.getTotalArcLength2D(startSegment);

  var<named("startFit_totalPerpS_endSegment")>() = startFit.getTotalArcLength2D(endSegment);
  var<named("endFit_totalPerpS_endSegment")>() = endFit.getTotalArcLength2D(endSegment);
  var<named("commonFit_totalPerpS_endSegment")>() = commonFit.getTotalArcLength2D(endSegment);

  var<named("startFit_isForwardOrBackwardTo_startSegment")>() = startFit.isForwardOrBackwardTo(startSegment);
  var<named("endFit_isForwardOrBackwardTo_startSegment")>() = endFit.isForwardOrBackwardTo(startSegment);
  var<named("commonFit_isForwardOrBackwardTo_startSegment")>() = commonFit.isForwardOrBackwardTo(startSegment);

  var<named("startFit_isForwardOrBackwardTo_endSegment")>() = startFit.isForwardOrBackwardTo(endSegment);
  var<named("endFit_isForwardOrBackwardTo_endSegment")>() = endFit.isForwardOrBackwardTo(endSegment);
  var<named("commonFit_isForwardOrBackwardTo_endSegment")>() = commonFit.isForwardOrBackwardTo(endSegment);


  var<named("startFit_perpSGap")>() = startFit.getArcLength2DGap(startSegment, endSegment);
  var<named("endFit_perpSGap")>() = endFit.getArcLength2DGap(startSegment, endSegment);
  var<named("commonFit_perpSGap")>() = commonFit.getArcLength2DGap(startSegment, endSegment);

  var<named("startFit_perpSFrontOffset")>() = startFit.getArcLength2DFrontOffset(startSegment, endSegment);
  var<named("endFit_perpSFrontOffset")>() = endFit.getArcLength2DFrontOffset(startSegment, endSegment);
  var<named("commonFit_perpSFrontOffset")>() = commonFit.getArcLength2DFrontOffset(startSegment, endSegment);

  var<named("startFit_perpSBackOffset")>() = startFit.getArcLength2DBackOffset(startSegment, endSegment);
  var<named("endFit_perpSBackOffset")>() = endFit.getArcLength2DBackOffset(startSegment, endSegment);
  var<named("commonFit_perpSBackOffset")>() = commonFit.getArcLength2DBackOffset(startSegment, endSegment);

  // Proximity indicators
  var<named("startFit_dist2DToFront_endSegment")>() = startFit.getDist2D(endFrontRecoPos2D);
  var<named("endFit_dist2DToBack_startSegment")>() = endFit.getDist2D(startBackRecoPos2D);

  // Momentum agreement
  var<named("startFit_absMom2D")>() = startFit.getAbsMom2D();
  var<named("endFit_absMom2D")>() = endFit.getAbsMom2D();

  Vector2D startMom2DAtStartBack = startFit.getFlightDirection2D(startBackRecoPos2D);
  Vector2D endMom2DAtEndFront = endFit.getFlightDirection2D(endFrontRecoPos2D);

  Vector2D startMom2DAtEndFront = startFit.getFlightDirection2D(endFrontRecoPos2D);
  Vector2D endMom2DAtStartBack = endFit.getFlightDirection2D(startBackRecoPos2D);

  var<named("momAngleAtStartBack")>() = startMom2DAtStartBack.angleWith(endMom2DAtStartBack);
  var<named("momAngleAtEndFront")>() = endMom2DAtEndFront.angleWith(startMom2DAtEndFront);

  // Fit variance and chi2
  var<named("startFit_chi2")>() = startFit.getChi2();
  var<named("endFit_chi2")>() = endFit.getChi2();
  var<named("commonFit_chi2")>() = commonFit3D.getChi2();

  var<named("commonFit_tanLambda")>() = commonFit3D.getTanLambda();
  var<named("commonFit_tanLambda_variance")>() = commonFit3D.getLocalVariance(EHelixParameter::c_TanL);
  var<named("szFit_tanLambda")>() = szFit.getTanLambda();

  var<named("axialFit_curvatureXY")>() = axialFit.getCurvature();
  var<named("axialFit_curvatureXY_variance")>() = axialFit.getLocalVariance(EPerigeeParameter::c_Curv);

  var<named("commonFit_curvatureXY")>() = commonFit.getCurvature();
  var<named("commonFit_curvatureXY_variance")>() = commonFit.getLocalVariance(EPerigeeParameter::c_Curv);

  return true;
}
