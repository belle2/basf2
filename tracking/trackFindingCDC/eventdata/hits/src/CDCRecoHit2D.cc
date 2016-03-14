/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <cdc/dataobjects/CDCSimHit.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


CDCRecoHit2D::CDCRecoHit2D() :
  m_rlWireHit(nullptr),
  m_recoDisp2D(Vector2D::getLowest())
{;}


CDCRecoHit2D::CDCRecoHit2D(const CDCRLWireHit* rlWireHit) :
  m_rlWireHit(rlWireHit),
  m_recoDisp2D(Vector2D::getLowest())
{
  if (rlWireHit == nullptr) B2ERROR("Initialization of two dimensional reconstructed hit with nullptr as oriented wire hit");
}


CDCRecoHit2D::CDCRecoHit2D(const CDCRLWireHit* rlWireHit,
                           const Vector2D& recoDisp2D) :
  m_rlWireHit(rlWireHit),
  m_recoDisp2D(recoDisp2D)
{
  if (rlWireHit == nullptr) B2ERROR("Initialization of two dimensional reconstructed hit with nullptr as oriented wire hit");
}

CDCRecoHit2D CDCRecoHit2D::fromSimHit(const CDCWireHit* wireHit,
                                      const CDCSimHit& simHit)
{
  if (wireHit == nullptr) {
    B2ERROR("Recohit with nullptr as wire hit");
    return CDCRecoHit2D();
  }

  // find out if the wire is right or left of the track ( view in flight direction )
  Vector3D trackPosToWire{simHit.getPosWire() - simHit.getPosTrack()};
  Vector3D directionOfFlight{simHit.getMomentum()};

  RightLeftInfo rlInfo = trackPosToWire.xy().isRightOrLeftOf(directionOfFlight.xy());

  const CDCRLWireHit* rlWireHit = CDCWireHitTopology::getInstance().getRLWireHit(*wireHit, rlInfo);

  CDCRecoHit2D recoHit2D(rlWireHit, Vector2D(-trackPosToWire.x(), -trackPosToWire.y()));
  recoHit2D.snapToDriftCircle();

  return recoHit2D;
}

CDCRecoHit2D
CDCRecoHit2D::average(const CDCRecoHit2D& recoHit1,
                      const CDCRecoHit2D& recoHit2)
{

  if (not(recoHit1.getRLWireHit() == recoHit2.getRLWireHit())) {
    B2ERROR("Average of two CDCRecoHit2Ds with different oriented wire hits requested, returning first given RecoHit");
    return recoHit1;
  }

  Vector2D displacement = Vector2D::average(recoHit1.getRecoDisp2D(), recoHit2.getRecoDisp2D());

  CDCRecoHit2D result(&(recoHit1.getRLWireHit()), displacement);
  result.snapToDriftCircle();

  return result;
}


CDCRecoHit2D
CDCRecoHit2D::average(const CDCRecoHit2D& recoHit1,
                      const CDCRecoHit2D& recoHit2,
                      const CDCRecoHit2D& recoHit3)
{
  if (not(recoHit1.getRLWireHit() == recoHit2.getRLWireHit() and
          recoHit2.getRLWireHit() == recoHit3.getRLWireHit())) {
    B2ERROR("Average of three CDCRecoHit2Ds with different wirehits requested, returning first given RecoHit");
    return recoHit1;
  }

  Vector2D displacement = Vector2D::average(recoHit1.getRecoDisp2D() ,
                                            recoHit2.getRecoDisp2D() ,
                                            recoHit3.getRecoDisp2D());

  CDCRecoHit2D result(&(recoHit1.getRLWireHit()), displacement);
  result.snapToDriftCircle();

  return result;

}

CDCRecoHit2D CDCRecoHit2D::fromRecoPos2D(
  const CDCRLWireHit* rlWireHit,
  const Vector2D& pos2D,
  bool snap
)
{
  CDCRecoHit2D result(rlWireHit, pos2D - rlWireHit->getRefPos2D());
  if (snap) result.snapToDriftCircle();
  return result;
}



void CDCRecoHit2D::reverse()
{
  const CDCRLWireHit* reverseRLWireHit =
    CDCWireHitTopology::getInstance().getReverseOf(getRLWireHit());
  setRLWireHit(reverseRLWireHit);
}



CDCRecoHit2D CDCRecoHit2D::reversed() const
{
  return CDCRecoHit2D(CDCWireHitTopology::getInstance().getReverseOf(getRLWireHit()), getRecoDisp2D());
}
