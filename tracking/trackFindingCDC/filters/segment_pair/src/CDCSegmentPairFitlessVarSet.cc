/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/CDCSegmentPairFitlessVarSet.h"
#include <assert.h>

#include <tracking/trackFindingCDC/fitting/CDCAxialStereoFusion.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>


using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCSegmentPairFitlessVarSet::CDCSegmentPairFitlessVarSet(const std::string& prefix) :
  VarSet<CDCSegmentPairFitlessVarNames>(prefix)
{
}

bool CDCSegmentPairFitlessVarSet::extract(const CDCSegmentPair* ptrSegmentPair)
{
  bool extracted = extractNested(ptrSegmentPair);
  if (not extracted or not ptrSegmentPair) return false;

  const CDCSegmentPair segmentPair = *ptrSegmentPair;

  const CDCRecoSegment2D* ptrStartSegment = segmentPair.getStartSegment();
  const CDCRecoSegment2D* ptrEndSegment = segmentPair.getEndSegment();

  assert(ptrStartSegment);
  assert(ptrEndSegment);

  const CDCRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCRecoSegment2D& endSegment = *ptrEndSegment;

  const CDCAxialRecoSegment2D* ptrAxialSegment = segmentPair.getAxialSegment();
  const CDCStereoRecoSegment2D* ptrStereoSegment = segmentPair.getStereoSegment();

  const CDCAxialRecoSegment2D& axialSegment = *ptrAxialSegment;
  const CDCStereoRecoSegment2D& stereoSegment = *ptrStereoSegment;

  // Segment fit should have been done at this point
  CDCTrajectory2D& startFit =  startSegment.getTrajectory2D();
  CDCTrajectory2D& endFit = endSegment.getTrajectory2D();
  CDCTrajectory2D& axialFit =  axialSegment.getTrajectory2D();
  CDCTrajectory2D& stereoFit = stereoSegment.getTrajectory2D();

  ISuperLayerType start_fit_superlayer_id = startFit.getStartISuperLayer();
  ISuperLayerType end_fit_superlayer_id = endFit.getStartISuperLayer();

  var<named("start_fit_superlayer_id")>() = start_fit_superlayer_id;
  var<named("end_fit_superlayer_id")>() = end_fit_superlayer_id;

  // Super layer coarse extrapolation
  var<named("next_superlayer_id_difference")>() = startFit.getNextISuperLayer() - end_fit_superlayer_id;
  var<named("previous_superlayer_id_difference")>() = endFit.getPreviousISuperLayer() - start_fit_superlayer_id;

  // Hits
  const CDCRecoHit2D& startFirstHit = startSegment.front();
  const CDCRecoHit2D& startLastHit = startSegment.back();

  const CDCRecoHit2D& endFirstHit = endSegment.front();
  const CDCRecoHit2D& endLastHit = endSegment.back();

  // Reconstructed position
  const Vector2D startFristRecoPos2D = startFirstHit.getRecoPos2D();
  const Vector2D startLastRecoPos2D = startLastHit.getRecoPos2D();

  const Vector2D endFirstRecoPos2D = endFirstHit.getRecoPos2D();
  const Vector2D endLastRecoPos2D = endLastHit.getRecoPos2D();

  // Wires
  const CDCWire& startLastWire = startLastHit.getWire();
  const CDCWire& startFirstWire = startFirstHit.getWire();

  const CDCWire& endFirstWire = endFirstHit.getWire();
  const CDCWire& endLastWire = endLastHit.getWire();


  const double endFirstToStartLastHitPhiDifference = endFirstRecoPos2D.angleWith(startLastRecoPos2D);
  if (fabs(endFirstToStartLastHitPhiDifference) > 1) return false;

  var<named("end_first_to_start_last_hit_phi_difference")>() = endFirstToStartLastHitPhiDifference;


  const Vector3D startLastRecoPosition3D = startLastHit.reconstruct3D(axialFit);
  const Vector3D endFirstRecoPosition3D = endFirstHit.reconstruct3D(axialFit);

  const bool startLastIsOutside = startLastWire.isOutsideCDC(startLastRecoPosition3D);
  const bool endFirstIsOutside = endFirstWire.isOutsideCDC(endFirstRecoPosition3D);

  var<named("end_first_hit_is_outside_cdc")>() = endFirstIsOutside;
  var<named("start_last_hit_is_outside_cdc")>() = startLastIsOutside;

  const double arcLength2DGap = axialFit.calcPerpSBetween(startLastRecoPosition3D.xy(),
                                                          endFirstRecoPosition3D.xy());

  var<named("arc_length2d_gap")>() = arcLength2DGap;
  if (arcLength2DGap < 0) return false;

  // Fit variance and chi2
  var<named("start_fit_chi2")>() = startFit.getChi2();
  var<named("end_fit_chi2")>() = endFit.getChi2();

  // Momentum agreement
  var<named("start_fit_curvature_xy")>() = startFit.getCurvature();
  var<named("end_fit_curvature_xy")>() = endFit.getCurvature();


  // Coalignment indicators
  var<named("startFit_totalPerpS_startSegment")>() = startFit.getTotalPerpS(startSegment);
  var<named("endFit_totalPerpS_startSegment")>() = endFit.getTotalPerpS(startSegment);

  var<named("startFit_totalPerpS_endSegment")>() = startFit.getTotalPerpS(endSegment);
  var<named("endFit_totalPerpS_endSegment")>() = endFit.getTotalPerpS(endSegment);

  var<named("startFit_isForwardOrBackwardTo_startSegment")>() = startFit.isForwardOrBackwardTo(startSegment);
  var<named("endFit_isForwardOrBackwardTo_startSegment")>() = endFit.isForwardOrBackwardTo(startSegment);

  var<named("startFit_isForwardOrBackwardTo_endSegment")>() = startFit.isForwardOrBackwardTo(endSegment);
  var<named("endFit_isForwardOrBackwardTo_endSegment")>() = endFit.isForwardOrBackwardTo(endSegment);

  var<named("startFit_perpSGap")>() = startFit.getPerpSGap(startSegment, endSegment);
  var<named("endFit_perpSGap")>() = endFit.getPerpSGap(startSegment, endSegment);

  var<named("startFit_perpSFrontOffset")>() = startFit.getPerpSFrontOffset(startSegment, endSegment);
  var<named("endFit_perpSFrontOffset")>() = endFit.getPerpSFrontOffset(startSegment, endSegment);

  var<named("startFit_perpSBackOffset")>() = startFit.getPerpSBackOffset(startSegment, endSegment);
  var<named("endFit_perpSBackOffset")>() = endFit.getPerpSBackOffset(startSegment, endSegment);

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


  var<named("axialFit_curvatureXY")>() = axialFit.getCurvature();
  var<named("axialFit_curvatureXY_variance")>() = axialFit.getLocalVariance(iCurv);

  return true;
}
