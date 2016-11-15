/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentRelation/HitGapSegmentRelationVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool HitGapSegmentRelationVarSet::extract(const Relation<const CDCSegment2D>* ptrSegmentRelation)
{
  if (not ptrSegmentRelation) return false;

  const Relation<const CDCSegment2D>& segmentPair = *ptrSegmentRelation;

  const CDCSegment2D* ptrFromSegment = segmentPair.getFrom();
  const CDCSegment2D* ptrToSegment = segmentPair.getTo();

  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;

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

  finitevar<named("delta_hit_pos_phi")>() = fromLastHitPos.angleWith(toFirstHitPos);
  finitevar<named("delta_hit_mom_phi")>() = fromLastHitMom.angleWith(toFirstHitMom);

  double fromLastHitAlpha = fromLastHit.getAlpha();
  double toFirstHitAlpha = toFirstHit.getAlpha();
  finitevar<named("delta_hit_alpha")>() = AngleUtil::normalised(toFirstHitAlpha - fromLastHitAlpha);

  finitevar<named("hit_distance")>() = hitDistance;
  finitevar<named("hit_long_distance")>() = longHitDistance;

  finitevar<named("delta_hit_distance")>() = longHitDistance - hitDistance;

  finitevar<named("from_hit_forward")>() = hitPosGap.dot(fromLastHitMom);
  finitevar<named("to_hit_forward")>() = hitPosGap.dot(toFirstHitMom);
  finitevar<named("hit_forward")>() = hitPosGap.dot(Vector2D::average(fromLastHitMom, toFirstHitMom));

  const Vector2D fromStretch = fromLastHitPos - fromFirstHitPos;
  const Vector2D toStretch = toLastHitPos - toFirstHitPos;

  const double fromLength = fromStretch.norm();
  const double toLength = toStretch.norm();

  const Vector2D firstPosGap = toFirstHitPos - fromFirstHitPos;
  const Vector2D lastPosGap = toLastHitPos - fromLastHitPos;

  const double firstOffset = firstPosGap.norm();
  const double lastOffset = lastPosGap.norm();

  finitevar<named("hit_ptolemy")>() =
    firstOffset * lastOffset - longHitDistance * hitDistance - fromLength * toLength;

  return true;
}
