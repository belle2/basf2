/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/CDCAxialStereoSegmentPairVarSet.h"
#include <assert.h>

#include <tracking/trackFindingCDC/fitting/CDCAxialStereoFusion.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>


using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCAxialStereoSegmentPairVarSet::CDCAxialStereoSegmentPairVarSet(const std::string& prefix) :
  VarSet<CDCAxialStereoSegmentPairVarNames>(prefix)
{
}

bool CDCAxialStereoSegmentPairVarSet::extract(const CDCAxialStereoSegmentPair* ptrSegmentPair)
{
  bool extracted = extractNested(ptrSegmentPair);
  if (not extracted or not ptrSegmentPair) return false;

  const CDCAxialStereoSegmentPair segmentPair = *ptrSegmentPair;

  const CDCRecoSegment2D* ptrStartSegment = segmentPair.getStartSegment();
  const CDCRecoSegment2D* ptrEndSegment = segmentPair.getEndSegment();

  assert(ptrStartSegment);
  assert(ptrEndSegment);

  const CDCRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCRecoSegment2D& endSegment = *ptrEndSegment;

  CDCTrajectory2D& startFit =  startSegment.getTrajectory2D();
  CDCTrajectory2D& endFit = endSegment.getTrajectory2D();

  if (not startFit.isFitted()) {
    getRiemannFitter().update(startFit, startSegment);
  }

  if (not endFit.isFitted()) {
    getRiemannFitter().update(endFit, endSegment);
  }

  CDCAxialStereoFusion::reconstructFuseTrajectories(segmentPair);
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
  var<named("startFit_totalPerpS_startSegment")>() = startFit.getTotalPerpS(startSegment);
  var<named("endFit_totalPerpS_startSegment")>() = endFit.getTotalPerpS(startSegment);
  var<named("commonFit_totalPerpS_startSegment")>() = commonFit.getTotalPerpS(startSegment);

  var<named("startFit_totalPerpS_endSegment")>() = startFit.getTotalPerpS(endSegment);
  var<named("endFit_totalPerpS_endSegment")>() = endFit.getTotalPerpS(endSegment);
  var<named("commonFit_totalPerpS_endSegment")>() = commonFit.getTotalPerpS(endSegment);

  var<named("startFit_isForwardOrBackwardTo_startSegment")>() = startFit.isForwardOrBackwardTo(startSegment);
  var<named("endFit_isForwardOrBackwardTo_startSegment")>() = endFit.isForwardOrBackwardTo(startSegment);
  var<named("commonFit_isForwardOrBackwardTo_startSegment")>() = commonFit.isForwardOrBackwardTo(startSegment);

  var<named("startFit_isForwardOrBackwardTo_endSegment")>() = startFit.isForwardOrBackwardTo(endSegment);
  var<named("endFit_isForwardOrBackwardTo_endSegment")>() = endFit.isForwardOrBackwardTo(endSegment);
  var<named("commonFit_isForwardOrBackwardTo_endSegment")>() = commonFit.isForwardOrBackwardTo(endSegment);


  var<named("startFit_perpSGap")>() = startFit.getPerpSGap(startSegment, endSegment);
  var<named("endFit_perpSGap")>() = endFit.getPerpSGap(startSegment, endSegment);
  var<named("commonFit_perpSGap")>() = commonFit.getPerpSGap(startSegment, endSegment);

  var<named("startFit_perpSFrontOffset")>() = startFit.getPerpSFrontOffset(startSegment, endSegment);
  var<named("endFit_perpSFrontOffset")>() = endFit.getPerpSFrontOffset(startSegment, endSegment);
  var<named("commonFit_perpSFrontOffset")>() = commonFit.getPerpSFrontOffset(startSegment, endSegment);

  var<named("startFit_perpSBackOffset")>() = startFit.getPerpSBackOffset(startSegment, endSegment);
  var<named("endFit_perpSBackOffset")>() = endFit.getPerpSBackOffset(startSegment, endSegment);
  var<named("commonFit_perpSBackOffset")>() = commonFit.getPerpSBackOffset(startSegment, endSegment);

  // Proximity indicators
  var<named("startFit_dist2DToCenter_endSegment")>() = startFit.getDist2DToCenter(endSegment);
  var<named("endFit_dist2DToCenter_startSegment")>() = endFit.getDist2DToCenter(startSegment);

  var<named("startFit_dist2DToFront_endSegment")>() = startFit.getDist2DToFront(endSegment);
  var<named("endFit_dist2DToBack_startSegment")>() = endFit.getDist2DToBack(startSegment);

  // Momentum agreement
  var<named("startFit_absMom2D")>() = startFit.getAbsMom2D();
  var<named("endFit_absMom2D")>() = endFit.getAbsMom2D();

  Vector2D startMomAtCenter = startFit.getUnitMom2DAtCenter(startSegment);
  Vector2D endMomAtCenter = endFit.getUnitMom2DAtCenter(endSegment);

  Vector2D startMomAtExtrapolation = startFit.getUnitMom2DAtCenter(endSegment);
  Vector2D endMomAtExtrapolation = endFit.getUnitMom2DAtCenter(startSegment);

  var<named("momAngleAtCenter_startSegment")>() = startMomAtCenter.angleWith(endMomAtExtrapolation);
  var<named("momAngleAtCenter_endSegment")>() = endMomAtCenter.angleWith(startMomAtExtrapolation);

  // Fit variance and chi2
  var<named("startFit_chi2")>() = startFit.getChi2();
  var<named("endFit_chi2")>() = endFit.getChi2();
  var<named("commonFit_chi2")>() = commonFit3D.getChi2();

  var<named("commonFit_szSlope")>() = commonFit3D.getSZSlope();
  var<named("commonFit_szSlope_variance")>() = commonFit3D.getLocalVariance(iSZ);
  var<named("szFit_szSlope")>() = szFit.getSZSlope();

  var<named("axialFit_curvatureXY")>() = axialFit.getCurvature();
  var<named("axialFit_curvatureXY_variance")>() = axialFit.getLocalVariance(iCurv);

  var<named("commonFit_curvatureXY")>() = commonFit.getCurvature();
  var<named("commonFit_curvatureXY_variance")>() = commonFit.getLocalVariance(iCurv);

  return true;
}
