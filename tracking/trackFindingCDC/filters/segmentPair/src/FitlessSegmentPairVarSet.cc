/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/FitlessSegmentPairVarSet.h>

#include <tracking/trackingUtilities/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackingUtilities/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackingUtilities/numerics/Angle.h>

#include <cdc/topology/CDCWire.h>

#include <framework/geometry/VectorUtil.h>

#include <Math/Vector3D.h>
#include <Math/Vector2D.h>
#include <Math/VectorUtil.h>

using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

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

  const ROOT::Math::XYVector fromHitPos = fromLastHit.getRecoPos2D();
  const ROOT::Math::XYVector toHitPos = toFirstHit.getRecoPos2D();

  // Fit
  const ROOT::Math::XYVector fromFitPos = fromFit.getClosest(fromHitPos);
  const ROOT::Math::XYVector toFitPos = toFit.getClosest(toHitPos);
  const ROOT::Math::XYVector fromFitMom = fromFit.getFlightDirection2D(fromHitPos);
  const ROOT::Math::XYVector toFitMom = toFit.getFlightDirection2D(toHitPos);

  const ROOT::Math::XYVector fromOtherFitMom = toFit.getFlightDirection2D(fromHitPos);
  const ROOT::Math::XYVector toOtherFitMom = fromFit.getFlightDirection2D(toHitPos);

  const double deltaPosPhi = ROOT::Math::VectorUtil::DeltaPhi(fromFitPos, toFitPos);
  const double deltaMomPhi = ROOT::Math::VectorUtil::DeltaPhi(fromFitMom, toFitMom);
  const double deltaAlpha = AngleUtil::normalised(deltaMomPhi - deltaPosPhi);

  finitevar<named("delta_pos_phi")>() = deltaPosPhi;
  finitevar<named("delta_mom_phi")>() = deltaMomPhi;

  finitevar<named("from_delta_mom_phi")>() = ROOT::Math::VectorUtil::DeltaPhi(fromFitMom, fromOtherFitMom);
  finitevar<named("to_delta_mom_phi")>() = ROOT::Math::VectorUtil::DeltaPhi(toFitMom, toOtherFitMom);
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

  const ROOT::Math::XYZVector nearAxialRecoPos = nearAxialHit.reconstruct3D(axialFit);
  const ROOT::Math::XYZVector farStereoRecoPos = farStereoHit.reconstruct3D(axialFit);
  const double farZ = farStereoRecoPos.z();

  const ROOT::Math::XYZVector nearStereoRecoPos = nearStereoHit.reconstruct3D(axialFit);
  const double nearZ = nearStereoRecoPos.z();

  const double stereoArcLength2D =
    axialFit.calcArcLength2DBetween(VectorUtil::getXYVector(nearStereoRecoPos),
                                    VectorUtil::getXYVector(farStereoRecoPos));

  const double arcLength2DGap =
    axialFit.calcArcLength2DBetween(VectorUtil::getXYVector(nearAxialRecoPos),
                                    VectorUtil::getXYVector(nearStereoRecoPos));

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
