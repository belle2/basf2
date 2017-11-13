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

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/numerics/ESign.h>

#include <cdc/dataobjects/CDCSimHit.h>

#include <TVector3.h>

#include <ostream>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCRecoHit2D::CDCRecoHit2D(const CDCRLWireHit& rlWireHit) :
  m_rlWireHit(rlWireHit),
  m_recoDisp2D(Vector2D::getLowest())
{}

CDCRecoHit2D::CDCRecoHit2D(const CDCRLWireHit& rlWireHit,
                           const Vector2D& recoDisp2D) :
  m_rlWireHit(rlWireHit),
  m_recoDisp2D(recoDisp2D)
{}

CDCRecoHit2D CDCRecoHit2D::fromSimHit(const CDCWireHit* wireHit,
                                      const CDCSimHit& simHit)
{
  // find out if the wire is right or left of the track ( view in flight direction )
  Vector3D trackPosToWire{simHit.getPosWire() - simHit.getPosTrack()};
  CDCRecoHit2D recoHit2D(CDCRLWireHit::fromSimHit(wireHit, simHit),
                         Vector2D(-trackPosToWire.x(), -trackPosToWire.y()));

  recoHit2D.snapToDriftCircle();
  return recoHit2D;
}

CDCRecoHit2D CDCRecoHit2D::average(const CDCRecoHit2D& recoHit1,
                                   const CDCRecoHit2D& recoHit2)
{
  CDCRLWireHit rlWireHit =
    CDCRLWireHit::average(recoHit1.getRLWireHit(),
                          recoHit2.getRLWireHit());

  Vector2D displacement =
    Vector2D::average(recoHit1.getRecoDisp2D(),
                      recoHit2.getRecoDisp2D());

  CDCRecoHit2D result(rlWireHit, displacement);
  result.snapToDriftCircle();

  return result;
}

CDCRecoHit2D CDCRecoHit2D::average(const CDCRecoHit2D& recoHit1,
                                   const CDCRecoHit2D& recoHit2,
                                   const CDCRecoHit2D& recoHit3)
{
  CDCRLWireHit rlWireHit =
    CDCRLWireHit::average(recoHit1.getRLWireHit(),
                          recoHit2.getRLWireHit(),
                          recoHit3.getRLWireHit());

  Vector2D displacement =
    Vector2D::average(recoHit1.getRecoDisp2D(),
                      recoHit2.getRecoDisp2D(),
                      recoHit3.getRecoDisp2D());

  CDCRecoHit2D result(rlWireHit, displacement);
  result.snapToDriftCircle();

  return result;
}

CDCRecoHit2D CDCRecoHit2D::fromRecoPos2D(const CDCRLWireHit& rlWireHit,
                                         const Vector2D& recoPos2D,
                                         bool snap)
{
  CDCRecoHit2D result(rlWireHit, recoPos2D - rlWireHit.getRefPos2D());
  if (snap) result.snapToDriftCircle();
  return result;
}

void CDCRecoHit2D::reverse()
{
  m_rlWireHit.reverse();
}

CDCRecoHit2D CDCRecoHit2D::reversed() const
{
  CDCRecoHit2D reversedRecoHit(*this);
  reversedRecoHit.reverse();
  return reversedRecoHit;
}

CDCRecoHit2D CDCRecoHit2D::getAlias() const
{
  return CDCRecoHit2D(getRLWireHit().getAlias(), -getRecoDisp2D());
}


void CDCRecoHit2D::setRefDriftLength(double driftLength, bool snapRecoPos)
{
  double oldDriftLength = m_rlWireHit.getRefDriftLength();
  m_rlWireHit.setRefDriftLength(driftLength);
  if (snapRecoPos) {
    bool switchSide = sign(oldDriftLength) != sign(driftLength);
    snapToDriftCircle(switchSide);
  }
}

void CDCRecoHit2D::snapToDriftCircle(bool switchSide)
{
  m_recoDisp2D.normalizeTo(getRLWireHit().getRefDriftLength());
  if (switchSide) {
    m_recoDisp2D = -m_recoDisp2D;
  }
}

Vector3D CDCRecoHit2D::reconstruct3D(const CDCTrajectory2D& trajectory2D, double z) const
{
  return getRLWireHit().reconstruct3D(trajectory2D, z);
}

std::ostream& TrackFindingCDC::operator<<(std::ostream& output, const CDCRecoHit2D& recohit)
{
  output << "CDCRecoHit2D(" << recohit.getRLWireHit() << ","
         << recohit.getRecoDisp2D() << ")" ;
  return output;
}
