/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;


CDCSegment2D CDCSegment3D::stereoProjectToRef() const
{
  CDCSegment2D result;
  for (const CDCRecoHit3D&  recoHit3D : *this) {
    result.push_back(recoHit3D.stereoProjectToRef());
  }
  result.setAliasScore(getAliasScore());
  return result;
}

CDCSegment3D CDCSegment3D::reconstruct(const CDCSegment2D& segment2D,
                                       const CDCTrajectory2D& trajectory2D)
{
  CDCSegment3D segment3D;
  for (const CDCRecoHit2D& recoHit2D : segment2D) {
    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit2D, trajectory2D);
    segment3D.push_back(recoHit3D);
  }
  segment3D.setAliasScore(segment2D.getAliasScore());
  return segment3D;
}
