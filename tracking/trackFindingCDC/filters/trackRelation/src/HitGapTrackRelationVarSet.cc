/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/HitGapTrackRelationVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/numerics/Angle.h>

using namespace Belle2;
using namespace TrackFindingCDC;

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

  const Vector3D fromLastHitPos = fromLastHit.getRecoPos3D();
  const Vector3D fromFirstHitPos = fromFirstHit.getRecoPos3D();

  const Vector3D toFirstHitPos = toFirstHit.getRecoPos3D();
  const Vector3D toLastHitPos = toLastHit.getRecoPos3D();

  const Vector3D hitPosGap = toFirstHitPos - fromLastHitPos;
  const Vector3D longHitPosGap = toLastHitPos - fromFirstHitPos;

  const double hitDistance = hitPosGap.norm();
  const double longHitDistance = longHitPosGap.norm();

  const Vector2D fromLastHitMom = fromLastHit.getFlightDirection2D();
  const Vector2D toFirstHitMom = toFirstHit.getFlightDirection2D();

  finitevar<named("delta_hit_pos_phi")>() = fromLastHitPos.angleWith(toFirstHitPos);
  finitevar<named("delta_hit_mom_phi")>() = fromLastHitMom.angleWith(toFirstHitMom);

  double fromLastHitAlpha = fromLastHit.getAlpha();
  double toFirstHitAlpha = toFirstHit.getAlpha();
  finitevar<named("delta_hit_alpha")>() = AngleUtil::normalised(toFirstHitAlpha - fromLastHitAlpha);

  finitevar<named("delta_hit_z")>() = fromLastHitPos.z() - toFirstHitPos.z();

  finitevar<named("hit_distance")>() = hitDistance;
  finitevar<named("hit_long_distance")>() = longHitDistance;

  finitevar<named("delta_hit_distance")>() = longHitDistance - hitDistance;

  finitevar<named("from_hit_forward")>() = hitPosGap.xy().dot(fromLastHitMom);
  finitevar<named("to_hit_forward")>() = hitPosGap.xy().dot(toFirstHitMom);
  finitevar<named("hit_forward")>() = hitPosGap.xy().dot(Vector2D::average(fromLastHitMom, toFirstHitMom));

  const Vector3D fromStretch = fromLastHitPos - fromFirstHitPos;
  const Vector3D toStretch = toLastHitPos - toFirstHitPos;

  const double fromLength = fromStretch.norm();
  const double toLength = toStretch.norm();

  const Vector3D firstPosGap = toFirstHitPos - fromFirstHitPos;
  const Vector3D lastPosGap = toLastHitPos - fromLastHitPos;

  const double firstOffset = firstPosGap.norm();
  const double lastOffset = lastPosGap.norm();

  finitevar<named("hit_ptolemy")>() =
    firstOffset * lastOffset - longHitDistance * hitDistance - fromLength * toLength;

  return true;
}
