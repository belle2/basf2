/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/HitGapSegmentPairVarSet.h>
#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

HitGapSegmentPairVarSet::HitGapSegmentPairVarSet()
  : Super()
{
}

bool HitGapSegmentPairVarSet::extract(const CDCSegmentPair* ptrSegmentPair)
{
  bool extracted = extractNested(ptrSegmentPair);
  if (not extracted or not ptrSegmentPair) return false;

  const CDCSegmentPair segmentPair = *ptrSegmentPair;

  const CDCRecoSegment2D* ptrFromSegment = segmentPair.getFromSegment();
  const CDCRecoSegment2D* ptrToSegment = segmentPair.getToSegment();

  const CDCRecoSegment2D& fromSegment = *ptrFromSegment;
  const CDCRecoSegment2D& toSegment = *ptrToSegment;

  const CDCRecoHit2D& fromFirstHit = fromSegment.front();
  const CDCRecoHit2D& fromLastHit = fromSegment.back();

  const CDCRecoHit2D& toFirstHit = toSegment.front();
  const CDCRecoHit2D& toLastHit = toSegment.back();

  const Vector2D fromLastHitPos = fromLastHit.getRecoPos2D();
  const Vector2D fromFirstHitPos = fromFirstHit.getRecoPos2D();

  const Vector2D toFirstHitPos = toFirstHit.getRecoPos2D();
  const Vector2D toLastHitPos = toLastHit.getRecoPos2D();

  const Vector2D hitPosGap = toFirstHitPos - fromLastHitPos;
  const Vector2D longHitPosGap = toLastHitPos - fromFirstHitPos;

  const double hitDistance = hitPosGap.norm();
  const double longHitDistance = longHitPosGap.norm();

  const Vector2D fromLastHitMom = fromLastHit.getFlightDirection2D();
  const Vector2D toFirstHitMom = toFirstHit.getFlightDirection2D();

  var<named("delta_hit_pos_phi")>() = fromLastHitPos.angleWith(toFirstHitPos);
  var<named("delta_hit_mom_phi")>() = fromLastHitMom.angleWith(toFirstHitMom);

  double fromLastHitAlpha = fromLastHit.getAlpha();
  double toFirstHitAlpha = toFirstHit.getAlpha();
  var<named("delta_hit_alpha")>() = AngleUtil::normalised(toFirstHitAlpha - fromLastHitAlpha);

  var<named("hit_distance")>() = hitDistance;
  var<named("hit_long_distance")>() = longHitDistance;

  var<named("delta_hit_distance")>() = longHitDistance - hitDistance;

  var<named("from_hit_forward")>() = hitPosGap.dot(fromLastHitMom);
  var<named("to_hit_forward")>() = hitPosGap.dot(toFirstHitMom);
  var<named("hit_forward")>() = hitPosGap.dot(Vector2D::average(fromLastHitMom, toFirstHitMom));

  const CDCRecoHit2D& axialHit = toFirstHit.isAxial() ? toFirstHit : fromLastHit;
  const CDCRecoHit2D& stereoHit = not toFirstHit.isAxial() ? toFirstHit : fromLastHit;

  const Vector2D axialHitPos = axialHit.getRecoPos2D();
  const Vector2D axialHitMom = axialHit.getFlightDirection2D();

  const CDCWire& stereoWire = stereoHit.getWire();
  const WireLine& wireLine = stereoWire.getWireLine();

  // Caluculate point were the wire is first method
  Vector2D relRefPos = wireLine.refPos2D() - axialHitPos;
  Vector2D movePerZ = wireLine.nominalMovePerZ();

  double z = -relRefPos.cross(axialHitMom) / movePerZ.cross(axialHitMom);
  Vector2D stereoHitPos = wireLine.nominalPos2DAtZ(z);

  var<named("hit_reco_z")>() = z;
  var<named("hit_z_bound_factor")>() =
    std::fmax(wireLine.backwardZ() - z, z - wireLine.forwardZ()) / (wireLine.forwardZ() - wireLine.backwardZ());
  var<named("hit_arclength_gap")>() = (stereoHitPos - axialHitPos).dot(axialHitMom);

  // const Vector2D fromStretch = fromLastHitPos - fromFirstHitPos;
  // const Vector2D toStretch = toLastHitPos - toFirstHitPos;

  // const double fromLength = fromStretch.norm();
  // const double toLength = toStretch.norm();

  // const Vector2D firstPosGap = toFirstHitPos - fromFirstHitPos;
  // const Vector2D lastPosGap = toLastHitPos - fromLastHitPos;

  // const double firstOffset = firstPosGap.norm();
  // const double lastOffset = lastPosGap.norm();

  // var<named("hit_ptolemy")>() =
  // firstOffset * lastOffset - longHitDistance * hitDistance - fromLength * toLength;
  return true;
}
