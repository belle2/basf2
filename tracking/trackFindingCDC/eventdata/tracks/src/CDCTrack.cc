/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/eventdata/collections/FillGenfitTrack.h>
#include <genfit/TrackCand.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;
using namespace genfit;


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

  for (const CDCRecoHit2D& recoHit2D : segment) {
    const CDCRLWireHit* ptrRLWireHit = &(recoHit2D.getRLWireHit());
    Vector3D recoPos3D(recoHit2D.getRecoPos2D(), 0.0);
    FloatType perpS = m_startTrajectory3D.calcPerpS(recoPos3D);
    push_back(CDCRecoHit3D(ptrRLWireHit, recoPos3D, perpS));
  }

  // TODO: Maybe enhance the estimation of the z coordinate with the superlayer slopes.
}

CDCTrack CDCTrack::condense(const std::vector<const CDCTrack*>& trackPath)
{
  if (trackPath.empty()) {
    return CDCTrack();
  } else if (trackPath.size() == 1) {
    return CDCTrack(*(trackPath[0]));
  } else {
    B2WARNING("Remember to implement multi track concatination");
    return CDCTrack(*(trackPath[0]));
  }
}



bool CDCTrack::fillInto(genfit::TrackCand& trackCand) const
{
  bool reverseRLInfo = false;
  fillHitsInto(*this, trackCand, reverseRLInfo);
  return getStartTrajectory3D().fillInto(trackCand);
}



vector<CDCRecoSegment3D> CDCTrack::splitIntoSegments() const
{
  vector<CDCRecoSegment3D> result;
  ISuperLayerType lastISuperLayer = -1;
  for (const CDCRecoHit3D& recoHit3D : *this) {
    ISuperLayerType iSuperLayer = recoHit3D.getISuperLayer();
    if (result.empty() or lastISuperLayer != iSuperLayer) {
      result.emplace_back();
    }
    result.back().push_back(recoHit3D);
    lastISuperLayer = iSuperLayer;
  }
  return result;
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
  for (CDCRecoHit3D& recoHit3D : *this) {
    recoHit3D.reverse();
    FloatType perpS = recoHit3D.getPerpS();
    recoHit3D.setPerpS(newLastPerpS + lastPerpS - perpS);
  }

  // Reverse the arrangement of hits.
  std::reverse(begin(), end());

}

CDCTrack CDCTrack::reversed() const
{
  CDCTrack reversedTrack(*this);
  reversedTrack.reverse();
  return reversedTrack;
}

void CDCTrack::unsetAndForwardMaskedFlag() const
{
  getAutomatonCell().unsetMaskedFlag();
  for (const CDCRecoHit3D& recoHit3D : *this) {
    const CDCWireHit& wireHit = recoHit3D.getWireHit();
    wireHit.getAutomatonCell().unsetMaskedFlag();
  }
}

void CDCTrack::setAndForwardMaskedFlag() const
{
  getAutomatonCell().setMaskedFlag();
  for (const CDCRecoHit3D& recoHit3D : *this) {
    const CDCWireHit& wireHit = recoHit3D.getWireHit();
    wireHit.getAutomatonCell().setMaskedFlag();
  }
}

void CDCTrack::receiveMaskedFlag() const
{
  for (const CDCRecoHit3D& recoHit3D : *this) {
    const CDCWireHit& wireHit = recoHit3D.getWireHit();
    if (wireHit.getAutomatonCell().hasMaskedFlag()) {
      getAutomatonCell().setMaskedFlag();
      return;
    }
  }
}

void CDCTrack::sort()
{ std::sort(begin(), end()); }
