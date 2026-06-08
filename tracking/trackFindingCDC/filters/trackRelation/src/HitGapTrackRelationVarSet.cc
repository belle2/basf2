/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/HitGapTrackRelationVarSet.h>

#include <tracking/trackingUtilities/eventdata/tracks/CDCTrack.h>
#include <tracking/trackingUtilities/geometry/VectorUtil.h>
#include <tracking/trackingUtilities/numerics/Angle.h>

#include <framework/geometry/VectorUtil.h>

#include <Math/Vector3D.h>
#include <Math/Vector2D.h>
#include <Math/VectorUtil.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

bool HitGapTrackRelationVarSet::extract(const Relation<const CDCTrack>* ptrTrackRelation)
{
  if (not ptrTrackRelation) return false;

  const Relation<const CDCTrack>& trackPair = *ptrTrackRelation;

  const CDCTrack* ptrFromTrack = trackPair.getFrom();
  const CDCTrack* ptrToTrack = trackPair.getTo();

  const CDCTrack& fromTrack = *ptrFromTrack;
  const CDCTrack& toTrack = *ptrToTrack;

  const CDCRecoHit3D& fromFirstHit = fromTrack.front();
  const CDCRecoHit3D& fromLastHit = fromTrack.back();

  const CDCRecoHit3D& toFirstHit = toTrack.front();
  const CDCRecoHit3D& toLastHit = toTrack.back();

  const ROOT::Math::XYZVector& fromLastHitPos = fromLastHit.getRecoPos3D();
  const ROOT::Math::XYZVector& fromFirstHitPos = fromFirstHit.getRecoPos3D();

  const ROOT::Math::XYZVector& toFirstHitPos = toFirstHit.getRecoPos3D();
  const ROOT::Math::XYZVector& toLastHitPos = toLastHit.getRecoPos3D();

  const ROOT::Math::XYZVector hitPosGap = toFirstHitPos - fromLastHitPos;
  const ROOT::Math::XYZVector longHitPosGap = toLastHitPos - fromFirstHitPos;

  const double hitDistance = hitPosGap.R();
  const double longHitDistance = longHitPosGap.R();

  const ROOT::Math::XYVector fromLastHitMom = fromLastHit.getFlightDirection2D();
  const ROOT::Math::XYVector toFirstHitMom = toFirstHit.getFlightDirection2D();

  finitevar<named("delta_hit_pos_phi")>() = ROOT::Math::VectorUtil::DeltaPhi(fromLastHitPos, toFirstHitPos);
  finitevar<named("delta_hit_mom_phi")>() = ROOT::Math::VectorUtil::DeltaPhi(fromLastHitMom, toFirstHitMom);

  double fromLastHitAlpha = fromLastHit.getAlpha();
  double toFirstHitAlpha = toFirstHit.getAlpha();
  finitevar<named("delta_hit_alpha")>() = AngleUtil::normalised(toFirstHitAlpha - fromLastHitAlpha);

  finitevar<named("delta_hit_z")>() = fromLastHitPos.z() - toFirstHitPos.z();

  finitevar<named("hit_distance")>() = hitDistance;
  finitevar<named("hit_long_distance")>() = longHitDistance;

  finitevar<named("delta_hit_distance")>() = longHitDistance - hitDistance;

  finitevar<named("from_hit_forward")>() = VectorUtil::get2DVector(hitPosGap).Dot(fromLastHitMom);
  finitevar<named("to_hit_forward")>() = VectorUtil::get2DVector(hitPosGap).Dot(toFirstHitMom);
  finitevar<named("hit_forward")>() = VectorUtil::get2DVector(hitPosGap).Dot(VectorUtil::average(fromLastHitMom, toFirstHitMom));

  const ROOT::Math::XYZVector fromStretch = fromLastHitPos - fromFirstHitPos;
  const ROOT::Math::XYZVector toStretch = toLastHitPos - toFirstHitPos;

  const double fromLength = fromStretch.R();
  const double toLength = toStretch.R();

  const ROOT::Math::XYZVector firstPosGap = toFirstHitPos - fromFirstHitPos;
  const ROOT::Math::XYZVector lastPosGap = toLastHitPos - fromLastHitPos;

  const double firstOffset = firstPosGap.R();
  const double lastOffset = lastPosGap.R();

  finitevar<named("hit_ptolemy")>() =
    firstOffset * lastOffset - longHitDistance * hitDistance - fromLength * toLength;

  return true;
}
