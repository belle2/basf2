/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


CDCRecoSegment2D CDCRecoSegment3D::stereoProjectToRef() const
{
  CDCRecoSegment2D result;
  for (const CDCRecoHit3D&  recoHit3D : *this) {
    result.push_back(recoHit3D.stereoProjectToRef());
  }
  result.setMayAlias(getMayAlias());
  return result;
}

CDCRecoSegment3D CDCRecoSegment3D::reconstruct(const CDCRecoSegment2D& segment2D,
                                               const CDCTrajectory2D& trajectory2D)
{
  CDCRecoSegment3D segment3D;
  for (const CDCRecoHit2D& recoHit2D : segment2D) {
    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit2D, trajectory2D);
    segment3D.push_back(recoHit3D);
  }
  segment3D.setMayAlias(segment2D.getMayAlias());
  return segment3D;
}
