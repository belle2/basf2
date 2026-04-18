/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentRelation/HitGapSegmentRelationVarSet.h>

#include <tracking/trackingUtilities/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackingUtilities/geometry/VectorUtil.h>
#include <tracking/trackingUtilities/numerics/Angle.h>

#include <framework/geometry/VectorUtil.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

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

  const double hitDistance = hitPosGap.R();
  const double longHitDistance = longHitPosGap.R();

  const Vector2D fromLastHitMom = fromLastHit.getFlightDirection2D();
  const Vector2D toFirstHitMom = toFirstHit.getFlightDirection2D();

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

  const Vector2D fromStretch = fromLastHitPos - fromFirstHitPos;
  const Vector2D toStretch = toLastHitPos - toFirstHitPos;

  const double fromLength = fromStretch.R();
  const double toLength = toStretch.R();

  const Vector2D firstPosGap = toFirstHitPos - fromFirstHitPos;
  const Vector2D lastPosGap = toLastHitPos - fromLastHitPos;

  const double firstOffset = firstPosGap.R();
  const double lastOffset = lastPosGap.R();

  finitevar<named("hit_ptolemy")>() =
    firstOffset * lastOffset - longHitDistance * hitDistance - fromLength * toLength;

  return true;
}
