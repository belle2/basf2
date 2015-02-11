/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCTrack.h"

#include <tracking/trackFindingCDC/eventdata/collections/FillGenfitTrack.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;
using namespace genfit;

TRACKFINDINGCDC_SwitchableClassImp(CDCTrack)

CDCTrack::CDCTrack(const CDCRecoSegment2D& segment) :
  m_startTrajectory3D(segment.getTrajectory2D()),
  m_endTrajectory3D(segment.getTrajectory2D())
{
  if (segment.empty()) return;

  // Adjust the start point
  const CDCRecoHit2D& startRecoHit2D = segment.front();
  const CDCRecoHit2D& endRecoHit2D = segment.back();

  Vector3D startPos3D(startRecoHit2D.getRecoPos2D(), 0.0);
  Vector3D endPos3D(endRecoHit2D.getRecoPos2D(), 0.0);

  m_startTrajectory3D.setLocalOrigin(startPos3D);
  m_endTrajectory3D.setLocalOrigin(endPos3D);

  for (const CDCRecoHit2D & recoHit2D : segment) {
    const CDCRLWireHit* ptrRLWireHit = &(recoHit2D.getRLWireHit());
    Vector3D recoPos3D(recoHit2D.getRecoPos2D(), 0.0);
    FloatType perpS = m_startTrajectory3D.calcPerpS(recoPos3D);
    push_back(CDCRecoHit3D(ptrRLWireHit, recoPos3D, perpS));
  }

  // TODO: Maybe enhance the estimation of the z coordinate with the superlayer slopes.
}



bool CDCTrack::fillInto(genfit::TrackCand& trackCand) const
{
  // Add the hits
  if (getFBInfo() == BACKWARD) {
    bool reverseRLInfo = true;
    fillHitsInto(reverseRange(), trackCand, reverseRLInfo);

  } else {
    bool reverseRLInfo = false;
    fillHitsInto(*this, trackCand, reverseRLInfo);
  }

  // Translate the trajectory information
  if (getFBInfo() == BACKWARD) {
    CDCTrajectory3D endTrajectory3D = getEndTrajectory3D().reversed();
    return endTrajectory3D.fillInto(trackCand);
  } else {
    return getStartTrajectory3D().fillInto(trackCand);
  }

}




void CDCTrack::reverse()
{
  if (empty()) return;

  // Exchange the forward and backward trajectory and reverse them
  std::swap(m_startTrajectory3D, m_endTrajectory3D);
  m_startTrajectory3D.reverse();
  m_endTrajectory3D.reverse();

  const CDCRecoHit3D& lastRecoHit3D = back();
  FloatType lastPerpS = lastRecoHit3D.getPerpS();
  FloatType newLastPerpS = m_startTrajectory3D.calcPerpS(lastRecoHit3D.getRecoPos3D());

  // Reverse the left right passage hypotheses and reverse the measured travel distance
  for (CDCRecoHit3D & recoHit3D : *this) {
    recoHit3D.reverse();
    FloatType perpS = recoHit3D.getPerpS();
    recoHit3D.setPerpS(newLastPerpS + lastPerpS - perpS);
  }

  // Reverse the arrangement of hits.
  std::reverse(begin(), end());

}

CDCTrack CDCTrack::reversed() const
{
  CDCTrack reversedTrack;
  reversedTrack.reverse();
  return reversedTrack;
}
