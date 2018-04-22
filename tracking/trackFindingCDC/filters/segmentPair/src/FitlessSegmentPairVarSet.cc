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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/numerics/Angle.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool FitlessSegmentPairVarSet::extract(const CDCSegmentPair* ptrSegmentPair)
{
  if (not ptrSegmentPair) return false;

  const CDCSegmentPair& segmentPair = *ptrSegmentPair;

  const CDCSegment2D* ptrFromSegment = segmentPair.getFromSegment();
  const CDCSegment2D* ptrToSegment = segmentPair.getToSegment();

  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;

  const CDCSegment2D* ptrAxialSegment = segmentPair.getAxialSegment();
  const CDCSegment2D& axialSegment = *ptrAxialSegment;

  const CDCSegment2D* ptrStereoSegment = segmentPair.getStereoSegment();
  const CDCSegment2D& stereoSegment = *ptrStereoSegment;

  // Segment fit should have been done at this point
  const CDCTrajectory2D& fromFit = fromSegment.getTrajectory2D();
  const CDCTrajectory2D& toFit = toSegment.getTrajectory2D();
  const CDCTrajectory2D& axialFit = axialSegment.getTrajectory2D();
  // const CDCTrajectory2D& stereoFit = stereoSegment.getTrajectory2D();

  finitevar<named("from_ndf")>() = fromFit.getNDF();
  finitevar<named("to_ndf")>() = toFit.getNDF();

  finitevar<named("from_chi2_over_ndf")>() = fabs(fromFit.getChi2() / fromFit.getNDF());
  finitevar<named("to_chi2_over_ndf")>() = fabs(toFit.getChi2() / toFit.getNDF());

  finitevar<named("from_p_value")>() = fromFit.getPValue();
  finitevar<named("to_p_value")>() = toFit.getPValue();

  // Direction agreement
  using namespace NPerigeeParameterIndices;
  const double fromCurv = fromFit.isFitted() ? fromFit.getCurvature() : NAN;
  const double fromCurvVar = fromFit.getLocalVariance(c_Curv);

  const double toCurv = toFit.isFitted() ? toFit.getCurvature() : NAN;
  const double toCurvVar = toFit.getLocalVariance(c_Curv);

  const double deltaCurvVar = fromCurvVar + toCurvVar;
  const double avgPrecision = 1 / fromCurvVar + 1 / toCurvVar;
  const double deltaCurvSigma = std::sqrt(deltaCurvVar);

  finitevar<named("abs_avg_curv")>() = std::fabs(toCurv / toCurvVar  + fromCurv / fromCurvVar) / avgPrecision;
  finitevar<named("delta_curv_var")>() = deltaCurvVar;
  finitevar<named("delta_curv")>() = toCurv - fromCurv;
  finitevar<named("delta_curv_pull")>() = (toCurv - fromCurv) / deltaCurvSigma;

  // Hits
  const CDCRecoHit2D& fromFirstHit = fromSegment.front();
  const CDCRecoHit2D& fromLastHit = fromSegment.back();
  const CDCRecoHit2D& toFirstHit = toSegment.front();
  const CDCRecoHit2D& toLastHit = toSegment.back();

  const Vector2D fromHitPos = fromLastHit.getRecoPos2D();
  const Vector2D toHitPos = toFirstHit.getRecoPos2D();

  // Fit
  const Vector2D fromFitPos = fromFit.getClosest(fromHitPos);
  const Vector2D toFitPos = toFit.getClosest(toHitPos);
  const Vector2D fromFitMom = fromFit.getFlightDirection2D(fromHitPos);
  const Vector2D toFitMom = toFit.getFlightDirection2D(toHitPos);

  const Vector2D fromOtherFitMom = toFit.getFlightDirection2D(fromHitPos);
  const Vector2D toOtherFitMom = fromFit.getFlightDirection2D(toHitPos);

  const double deltaPosPhi = fromFitPos.angleWith(toFitPos);
  const double deltaMomPhi = fromFitMom.angleWith(toFitMom);
  const double deltaAlpha = AngleUtil::normalised(deltaMomPhi - deltaPosPhi);

  finitevar<named("delta_pos_phi")>() = deltaPosPhi;
  finitevar<named("delta_mom_phi")>() = deltaMomPhi;

  finitevar<named("from_delta_mom_phi")>() = fromFitMom.angleWith(fromOtherFitMom);
  finitevar<named("to_delta_mom_phi")>() = toFitMom.angleWith(toOtherFitMom);
  finitevar<named("delta_alpha")>() = deltaAlpha;

  // Reconstructed quantities
  // One of the fitted positions corresponds to the axial hit and one to the stereo hit.
  const CDCRecoHit2D& nearAxialHit = toFirstHit.isAxial() ? toFirstHit : fromLastHit;
  const CDCRecoHit2D& farStereoHit = not fromFirstHit.isAxial() ? fromFirstHit : toLastHit;
  const CDCRecoHit2D& nearStereoHit = not toFirstHit.isAxial() ? toFirstHit : fromLastHit;

  const CDCWire& farStereoWire = farStereoHit.getWire();
  const WireLine& farWireLine = farStereoWire.getWireLine();

  const CDCWire& nearStereoWire = nearStereoHit.getWire();
  const WireLine& nearWireLine = nearStereoWire.getWireLine();

  const Vector3D nearAxialRecoPos = nearAxialHit.reconstruct3D(axialFit);
  const Vector3D farStereoRecoPos = farStereoHit.reconstruct3D(axialFit);
  const double farZ = farStereoRecoPos.z();

  const Vector3D nearStereoRecoPos = nearStereoHit.reconstruct3D(axialFit);
  const double nearZ = nearStereoRecoPos.z();

  const double stereoArcLength2D =
    axialFit.calcArcLength2DBetween(nearStereoRecoPos.xy(),
                                    farStereoRecoPos.xy());

  const double arcLength2DGap =
    axialFit.calcArcLength2DBetween(nearAxialRecoPos.xy(),
                                    nearStereoRecoPos.xy());

  finitevar<named("reco_arc_length_gap")>() = fabs(arcLength2DGap);
  finitevar<named("stereo_arc_length")>() = fabs(stereoArcLength2D);

  finitevar<named("near_reco_z")>() = nearZ;
  finitevar<named("near_z_bound_factor")>() = nearWireLine.outOfZBoundsFactor(nearZ);

  finitevar<named("far_reco_z")>() = farZ;
  finitevar<named("far_z_bound_factor")>() = farWireLine.outOfZBoundsFactor(farZ);

  finitevar<named("coarse_tanl")>() = (farZ - nearZ) / stereoArcLength2D;

  finitevar<named("stereo_rel_size")>() = fabs(stereoSegment.size() / stereoArcLength2D);

  finitevar<named("arc_length_front_offset")>() =
    (fromFit.getArcLength2DFrontOffset(fromSegment, toSegment)
     + toFit.getArcLength2DFrontOffset(fromSegment, toSegment)) / 2;

  finitevar<named("arc_length_back_offset")>() =
    (fromFit.getArcLength2DBackOffset(fromSegment, toSegment)
     + toFit.getArcLength2DBackOffset(fromSegment, toSegment)) / 2;

  finitevar<named("from_arc_length_total")>() = toFit.getTotalArcLength2D(fromSegment);
  finitevar<named("to_arc_length_total")>() = fromFit.getTotalArcLength2D(toSegment);

  finitevar<named("arc_length_gap")>() =
    (fromFit.getArcLength2DGap(fromSegment, toSegment)
     + toFit.getArcLength2DGap(fromSegment, toSegment)) / 2;

  return true;
}
