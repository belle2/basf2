/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/FitlessSegmentPairVarSet.h>
#include <assert.h>

#include <tracking/trackFindingCDC/fitting/CDCAxialStereoFusion.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>


using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  double nanmin(double x, double y)
  {
    if (std::isnan(x)) return y;
    if (std::isnan(y)) return x;
    return std::min(x, y);
  }

  double nanmax(double x, double y)
  {
    if (std::isnan(x)) return y;
    if (std::isnan(y)) return x;
    return std::max(x, y);
  }

}

FitlessSegmentPairVarSet::FitlessSegmentPairVarSet()
  : Super()
{
}

bool FitlessSegmentPairVarSet::extract(const CDCSegmentPair* ptrSegmentPair)
{
  bool extracted = extractNested(ptrSegmentPair);
  if (not extracted or not ptrSegmentPair) return false;

  const CDCSegmentPair segmentPair = *ptrSegmentPair;

  const CDCRecoSegment2D* ptrStartSegment = segmentPair.getFromSegment();
  const CDCRecoSegment2D* ptrEndSegment = segmentPair.getToSegment();

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
  // CDCTrajectory2D& stereoFit = stereoSegment.getTrajectory2D();

  ISuperLayer startFitISuperLayer = startFit.getStartISuperLayer();
  ISuperLayer endFitISuperLayer = endFit.getStartISuperLayer();

  var<named("start_fit_superlayer_id")>() = startFitISuperLayer;
  var<named("end_fit_superlayer_id")>() = endFitISuperLayer;

  // Cross check of fit against the superlayer of the segment
  var<named("start_fit_superlayer_id_difference")>() = startFitISuperLayer - startSegment.getISuperLayer();
  var<named("end_fit_superlayer_id_difference")>() = endFitISuperLayer - endSegment.getISuperLayer();

  // Super layer coarse extrapolation
  var<named("next_superlayer_id_difference")>() = startFit.getNextISuperLayer() - endFitISuperLayer;
  var<named("previous_superlayer_id_difference")>() = endFit.getPreviousISuperLayer() - startFitISuperLayer;

  // Hits
  //const CDCRecoHit2D& startFirstHit = startSegment.front();
  const CDCRecoHit2D& startLastHit = startSegment.back();

  const CDCRecoHit2D& endFirstHit = endSegment.front();
  //const CDCRecoHit2D& endLastHit = endSegment.back();

  const CDCRecoHit2D& stereoFirstHit = stereoSegment.front();
  const CDCRecoHit2D& stereoLastHit = stereoSegment.back();

  // Reconstructed position
  // const Vector2D startFirstRecoPos2D = startFirstHit.getRecoPos2D();
  const Vector2D startLastRecoPos2D = startLastHit.getRecoPos2D();

  const Vector2D endFirstRecoPos2D = endFirstHit.getRecoPos2D();
  // const Vector2D endLastRecoPos2D = endLastHit.getRecoPos2D();

  // Wires
  // const CDCWire& startFirstWire = startFirstHit.getWire();
  // const CDCWire& startLastWire = startLastHit.getWire();

  // const CDCWire& endFirstWire = endFirstHit.getWire();
  // const CDCWire& endLastWire = endLastHit.getWire();

  const CDCWire& stereoFirstWire = stereoFirstHit.getWire();
  const CDCWire& stereoLastWire = stereoLastHit.getWire();

  // Momentum vector
  const Vector2D startLastUnitMom2D = startFit.getFlightDirection2D(startLastRecoPos2D);
  const Vector2D endFirstUnitMom2D = endFit.getFlightDirection2D(endFirstRecoPos2D);

  const double endFirstToStartLastHitPosPhiDifference = endFirstRecoPos2D.angleWith(startLastRecoPos2D);
  const double endFirstToStartLastHitMomPhiDifference = endFirstUnitMom2D.angleWith(startLastUnitMom2D);

  var<named("end_first_to_start_last_hit_pos_phi_difference")>() = endFirstToStartLastHitPosPhiDifference;
  var<named("end_first_to_start_last_hit_mom_phi_difference")>() = endFirstToStartLastHitMomPhiDifference;
  const double endFirstToStartLastHitPhiDifference =
    endFirstToStartLastHitMomPhiDifference - endFirstToStartLastHitPosPhiDifference;

  var<named("end_first_to_start_last_hit_phi_difference")>() = endFirstToStartLastHitPhiDifference;

  const Vector3D stereoLastRecoPosition3D = stereoLastHit.reconstruct3D(axialFit);
  const Vector3D stereoFirstRecoPosition3D = stereoFirstHit.reconstruct3D(axialFit);

  const double stereoFirstHitDistZForwardWall = stereoFirstRecoPosition3D.z() - stereoFirstWire.getForwardZ();
  const double stereoFirstHitDistZBackwardWall = stereoFirstRecoPosition3D.z() - stereoFirstWire.getBackwardZ();

  const double stereoLastHitDistZForwardWall = stereoLastRecoPosition3D.z() - stereoLastWire.getForwardZ();
  const double stereoLastHitDistZBackwardWall = stereoLastRecoPosition3D.z() - stereoLastWire.getBackwardZ();

  var<named("stereo_first_hit_z")>() = stereoFirstRecoPosition3D.z();
  var<named("stereo_last_hit_z")>() = stereoLastRecoPosition3D.z();

  var<named("stereo_first_hit_dist_z_forward_wall")>() = stereoFirstHitDistZForwardWall;
  var<named("stereo_first_hit_dist_z_backward_wall")>() = stereoFirstHitDistZBackwardWall;

  var<named("stereo_last_hit_dist_z_forward_wall")>() = stereoLastHitDistZForwardWall;
  var<named("stereo_last_hit_dist_z_backward_wall")>() = stereoLastHitDistZBackwardWall;

  var<named("stereo_hits_max_dist_z_forward_wall")>() = nanmax(stereoFirstHitDistZForwardWall,
                                                        stereoLastHitDistZForwardWall);

  var<named("stereo_hits_min_dist_z_forward_wall")>() = nanmin(stereoFirstHitDistZForwardWall,
                                                        stereoLastHitDistZForwardWall);

  var<named("stereo_hits_max_dist_z_backward_wall")>() = nanmax(stereoFirstHitDistZBackwardWall,
                                                         stereoLastHitDistZBackwardWall);

  var<named("stereo_hits_min_dist_z_backward_wall")>() = nanmin(stereoFirstHitDistZBackwardWall,
                                                         stereoLastHitDistZBackwardWall);

  var<named("start_arc_length_front_offset")>() = startFit.getArcLength2DFrontOffset(startSegment, endSegment);
  var<named("end_arc_length_front_offset")>() = endFit.getArcLength2DFrontOffset(startSegment, endSegment);

  var<named("start_arc_length_back_offset")>() = startFit.getArcLength2DBackOffset(startSegment, endSegment);
  var<named("end_arc_length_back_offset")>() = endFit.getArcLength2DBackOffset(startSegment, endSegment);

  return true;
  /*
    // const double arcLength2DGap = axialFit.calcPerpSBetween(startLastRecoPosition3D.xy(),
    // endFirstRecoPosition3D.xy());

    // var<named("arc_length2d_gap")>() = arcLength2DGap;
    var<named("arc_length2d_gap")>() = 0;
    // if (arcLength2DGap < 0) return false;

    // Fit variance and chi2
    var<named("start_fit_chi2")>() = startFit.getChi2();
    var<named("end_fit_chi2")>() = endFit.getChi2();

    // Momentum agreement
    var<named("start_fit_curvature_xy")>() = startFit.getCurvature() == 0 ? NAN : startFit.getCurvature();
    var<named("end_fit_curvature_xy")>() = endFit.getCurvature() == 0 ? NAN :  endFit.getCurvature();


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


    // Proximity indicators
    var<named("startFit_dist2DToCenter_endSegment")>() = startFit.getDist2DToCenter(endSegment);
    var<named("endFit_dist2DToCenter_startSegment")>() = endFit.getDist2DToCenter(startSegment);

    var<named("startFit_dist2DToFront_endSegment")>() = startFit.getDist2DToFront(endSegment);
    var<named("endFit_dist2DToBack_startSegment")>() = endFit.getDist2DToBack(startSegment);

    // Momentum agreement
    var<named("startFit_absMom2D")>() = startFit.getAbsMom2D();
    var<named("endFit_absMom2D")>() = endFit.getAbsMom2D();

    Vector2D startMomAtCenter = startFit.getFlightDirection2DAtCenter(startSegment);
    Vector2D endMomAtCenter = endFit.getFlightDirection2DAtCenter(endSegment);

    Vector2D startMomAtExtrapolation = startFit.getFlightDirection2DAtCenter(endSegment);
    Vector2D endMomAtExtrapolation = endFit.getFlightDirection2DAtCenter(startSegment);

    var<named("momAngleAtCenter_startSegment")>() = startMomAtCenter.angleWith(endMomAtExtrapolation);
    var<named("momAngleAtCenter_endSegment")>() = endMomAtCenter.angleWith(startMomAtExtrapolation);


    var<named("axialFit_curvatureXY")>() = axialFit.getCurvature();
    var<named("axialFit_curvatureXY_variance")>() = axialFit.getLocalVariance(c_Curv);

    return true;
  */
}
