/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/HitGapSegmentPairVarSet.h>

#include <tracking/trackingUtilities/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackingUtilities/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackingUtilities/geometry/VectorUtil.h>
#include <tracking/trackingUtilities/numerics/Angle.h>

#include <cdc/topology/CDCWire.h>

#include <framework/geometry/VectorUtil.h>

#include <Math/Vector2D.h>

using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

bool HitGapSegmentPairVarSet::extract(const CDCSegmentPair* ptrSegmentPair)
{
  if (not ptrSegmentPair) return false;

  const CDCSegmentPair& segmentPair = *ptrSegmentPair;

  const CDCSegment2D* ptrFromSegment = segmentPair.getFromSegment();
  const CDCSegment2D* ptrToSegment = segmentPair.getToSegment();

  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;

  const CDCRecoHit2D& fromFirstHit = fromSegment.front();
  const CDCRecoHit2D& fromLastHit = fromSegment.back();

  const CDCRecoHit2D& toFirstHit = toSegment.front();
  const CDCRecoHit2D& toLastHit = toSegment.back();

  const ROOT::Math::XYVector fromLastHitPos = fromLastHit.getRecoPos2D();
  const ROOT::Math::XYVector fromFirstHitPos = fromFirstHit.getRecoPos2D();

  const ROOT::Math::XYVector toFirstHitPos = toFirstHit.getRecoPos2D();
  const ROOT::Math::XYVector toLastHitPos = toLastHit.getRecoPos2D();

  const ROOT::Math::XYVector hitPosGap = toFirstHitPos - fromLastHitPos;
  const ROOT::Math::XYVector longHitPosGap = toLastHitPos - fromFirstHitPos;

  const double hitDistance = hitPosGap.R();
  const double longHitDistance = longHitPosGap.R();

  const ROOT::Math::XYVector fromLastHitMom = fromLastHit.getFlightDirection2D();
  const ROOT::Math::XYVector toFirstHitMom = toFirstHit.getFlightDirection2D();

  finitevar<named("delta_hit_pos_phi")>() = VectorUtil::Angle(fromLastHitPos, toFirstHitPos);
  finitevar<named("delta_hit_mom_phi")>() = VectorUtil::Angle(fromLastHitMom, toFirstHitMom);

  double fromLastHitAlpha = fromLastHit.getAlpha();
  double toFirstHitAlpha = toFirstHit.getAlpha();
  finitevar<named("delta_hit_alpha")>() = AngleUtil::normalised(toFirstHitAlpha - fromLastHitAlpha);

  finitevar<named("hit_distance")>() = hitDistance;
  finitevar<named("hit_long_distance")>() = longHitDistance;

  finitevar<named("delta_hit_distance")>() = longHitDistance - hitDistance;

  finitevar<named("from_hit_forward")>() = hitPosGap.Dot(fromLastHitMom);
  finitevar<named("to_hit_forward")>() = hitPosGap.Dot(toFirstHitMom);
  finitevar<named("hit_forward")>() = hitPosGap.Dot(VectorUtil::average(fromLastHitMom, toFirstHitMom));

  const CDCRecoHit2D& axialHit = toFirstHit.isAxial() ? toFirstHit : fromLastHit;
  const CDCRecoHit2D& stereoHit = not toFirstHit.isAxial() ? toFirstHit : fromLastHit;

  const ROOT::Math::XYVector axialHitPos = axialHit.getRecoPos2D();
  const ROOT::Math::XYVector axialHitMom = axialHit.getFlightDirection2D();

  const CDCWire& stereoWire = stereoHit.getWire();
  const WireLine& wireLine = stereoWire.getWireLine();

  // Calculate point were the wire is first method
  ROOT::Math::XYVector relRefPos = wireLine.refPos2D() - axialHitPos;
  ROOT::Math::XYVector movePerZ = wireLine.nominalMovePerZ();

  double z = VectorUtil::Cross(-relRefPos, axialHitMom) / VectorUtil::Cross(movePerZ, axialHitMom);
  ROOT::Math::XYVector stereoHitPos = stereoHit.getRecoPos2D() + movePerZ * z;

  finitevar<named("hit_reco_z")>() = z;
  finitevar<named("hit_z_bound_factor")>() = wireLine.outOfZBoundsFactor(z);
  double arcLengthGap = (stereoHitPos - axialHitPos).Dot(axialHitMom);
  finitevar<named("hit_arclength_gap")>() = toFirstHit.isAxial() ? -arcLengthGap : arcLengthGap;

  // const ROOT::Math::XYVector fromStretch = fromLastHitPos - fromFirstHitPos;
  // const ROOT::Math::XYVector toStretch = toLastHitPos - toFirstHitPos;

  // const double fromLength = fromStretch.R();
  // const double toLength = toStretch.R();

  // const ROOT::Math::XYVector firstPosGap = toFirstHitPos - fromFirstHitPos;
  // const ROOT::Math::XYVector lastPosGap = toLastHitPos - fromLastHitPos;

  // const double firstOffset = firstPosGap.R();
  // const double lastOffset = lastPosGap.R();

  // finitevar<named("hit_ptolemy")>() =
  //   firstOffset * lastOffset - longHitDistance * hitDistance - fromLength * toLength;

  return true;
}
