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

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


CDCRecoSegment2D CDCRecoSegment3D::projectXY() const
{
  CDCRecoSegment2D result;
  for (const CDCRecoHit3D&  recoHit3D : *this) {
    result.push_back(recoHit3D.projectXY());
  }
  return result;
}
