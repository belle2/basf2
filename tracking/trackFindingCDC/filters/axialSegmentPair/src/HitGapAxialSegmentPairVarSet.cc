/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/axialSegmentPair/HitGapAxialSegmentPairVarSet.h>

#include <tracking/trackingUtilities/eventdata/tracks/CDCAxialSegmentPair.h>
#include <tracking/trackingUtilities/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackingUtilities/geometry/VectorUtil.h>

#include <tracking/trackingUtilities/numerics/Angle.h>

#include <Math/Vector2D.h>
#include <Math/VectorUtil.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

bool HitGapAxialSegmentPairVarSet::extract(const CDCAxialSegmentPair* ptrAxialSegmentPair)
{
  if (not ptrAxialSegmentPair) return false;

  const CDCAxialSegmentPair& axialSegmentPair = *ptrAxialSegmentPair;

  const CDCSegment2D* ptrFromSegment = axialSegmentPair.getStartSegment();
  const CDCSegment2D* ptrToSegment = axialSegmentPair.getEndSegment();

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

  finitevar<named("delta_hit_pos_phi")>() = ROOT::Math::VectorUtil::DeltaPhi(fromLastHitPos, toFirstHitPos);
  finitevar<named("delta_hit_mom_phi")>() = ROOT::Math::VectorUtil::DeltaPhi(fromLastHitMom, toFirstHitMom);

  double fromLastHitAlpha = fromLastHit.getAlpha();
  double toFirstHitAlpha = toFirstHit.getAlpha();
  finitevar<named("delta_hit_alpha")>() = AngleUtil::normalised(toFirstHitAlpha - fromLastHitAlpha);

  finitevar<named("hit_distance")>() = hitDistance;
  finitevar<named("hit_long_distance")>() = longHitDistance;

  finitevar<named("delta_hit_distance")>() = longHitDistance - hitDistance;

  finitevar<named("from_hit_forward")>() = hitPosGap.Dot(fromLastHitMom);
  finitevar<named("to_hit_forward")>() = hitPosGap.Dot(toFirstHitMom);
  finitevar<named("hit_forward")>() = hitPosGap.Dot(VectorUtil::average(fromLastHitMom, toFirstHitMom));

  const ROOT::Math::XYVector fromStretch = fromLastHitPos - fromFirstHitPos;
  const ROOT::Math::XYVector toStretch = toLastHitPos - toFirstHitPos;

  const double fromLength = fromStretch.R();
  const double toLength = toStretch.R();

  const ROOT::Math::XYVector firstPosGap = toFirstHitPos - fromFirstHitPos;
  const ROOT::Math::XYVector lastPosGap = toLastHitPos - fromLastHitPos;

  const double firstOffset = firstPosGap.R();
  const double lastOffset = lastPosGap.R();

  finitevar<named("hit_ptolemy")>() =
    firstOffset * lastOffset - longHitDistance * hitDistance - fromLength * toLength;

  return true;
}
