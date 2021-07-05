/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/TrailSegmentPairVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool TrailSegmentPairVarSet::extract(const CDCSegmentPair* ptrSegmentPair)
{
  if (not ptrSegmentPair) return false;

  const CDCSegmentPair& segmentPair = *ptrSegmentPair;

  const CDCSegment2D* ptrFromSegment = segmentPair.getFromSegment();
  const CDCSegment2D* ptrToSegment = segmentPair.getToSegment();

  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;
  /*
  const CDCSegment2D* ptrAxialSegment = segmentPair.getAxialSegment();
  const CDCSegment2D& axialSegment = *ptrAxialSegment;

  const CDCSegment2D* ptrStereoSegment = segmentPair.getStereoSegment();
  const CDCSegment2D& stereoSegment = *ptrStereoSegment;

  bool fromIsAxial = ptrFromSegment == ptrAxialSegment;

  // Segment fit should have been done at this point
  const CDCTrajectory2D& fromFit = fromSegment.getTrajectory2D();
  const CDCTrajectory2D& toFit = toSegment.getTrajectory2D();
  const CDCTrajectory2D& axialFit = axialSegment.getTrajectory2D();
  const CDCTrajectory2D& stereoFit = stereoSegment.getTrajectory2D();

  // Hits
  const CDCRecoHit2D& fromFirstHit = fromSegment.front();
  const CDCRecoHit2D& fromLastHit = fromSegment.back();
  const CDCRecoHit2D& toFirstHit = toSegment.front();
  const CDCRecoHit2D& toLastHit = toSegment.back();

  const CDCRecoHit2D& nearAxialHit = fromIsAxial ? fromLastHit : toFirstHit;
  const CDCRecoHit2D& nearStereoHit = not fromIsAxial ? fromLastHit : toFirstHit;
  const CDCRecoHit2D& farAxialHit = fromIsAxial ? fromFirstHit : toLastHit;
  const CDCRecoHit2D& farStereoHit = not fromIsAxial ? fromFirstHit : toLastHit;
  */

  int fromNRLSwitches = fromSegment.getNRLSwitches();
  int toNRLSwitches = toSegment.getNRLSwitches();
  double fromRLAsymmetry = fromSegment.getRLAsymmetry();
  double toRLAsymmetry = toSegment.getRLAsymmetry();

  var<named("from_rl_asym")>() = fromRLAsymmetry * fromSegment.size();
  var<named("to_rl_asym")>()   = toRLAsymmetry * toSegment.size();

  var<named("from_rl_switches")>() = fromNRLSwitches < 12 ? fromNRLSwitches : 12;
  var<named("to_rl_switches")>()   = toNRLSwitches < 12 ? toNRLSwitches : 12;

  return true;
}
