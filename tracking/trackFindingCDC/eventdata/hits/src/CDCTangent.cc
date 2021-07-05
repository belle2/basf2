/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/hits/CDCTangent.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHitPair.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

#include <tracking/trackFindingCDC/geometry/ParameterLine2D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/numerics/ERightLeft.h>

#include <iostream>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCTangent::CDCTangent(const CDCRLWireHitPair& rlWireHitPair):
  CDCRLWireHitPair(rlWireHitPair), m_line()
{
  adjustLine();
}

CDCTangent::CDCTangent(const CDCRLWireHit& fromRLWireHit,
                       const CDCRLWireHit& toRLWireHit):
  CDCRLWireHitPair(fromRLWireHit, toRLWireHit), m_line()
{
  adjustLine();
}

CDCTangent::CDCTangent(const CDCRLWireHitPair& rlWireHitPair,
                       const ParameterLine2D& line):
  CDCRLWireHitPair(rlWireHitPair),
  m_line(line)
{}

CDCTangent::CDCTangent(const CDCRLWireHit& fromRLWireHit,
                       const CDCRLWireHit& toRLWireHit,
                       const ParameterLine2D& line):
  CDCRLWireHitPair(fromRLWireHit, toRLWireHit),
  m_line(line)
{}

void CDCTangent::adjustLine()
{
  m_line = ParameterLine2D::touchingCircles(getFromRLWireHit().getRefPos2D(),
                                            getFromRLInfo() * getFromRLWireHit().getRefDriftLength(),
                                            getToRLWireHit().getRefPos2D(),
                                            getToRLInfo() * getToRLWireHit().getRefDriftLength());
}

void CDCTangent::adjustRLInfo()
{
  ERightLeft newFromRLInfo = getLine().isRightOrLeft(getFromRLWireHit().getRefPos2D());
  setFromRLInfo(newFromRLInfo);

  ERightLeft newToRLInfo = getLine().isRightOrLeft(getToRLWireHit().getRefPos2D());
  setToRLInfo(newToRLInfo);
}

void CDCTangent::reverse()
{
  CDCRLWireHitPair::reverse();

  //reverse the direction of flight
  m_line.reverse();

  //adjust the support point to be now at the other wirehit touch position
  m_line.passiveMoveAtBy(-1);
}

CDCTangent CDCTangent::reversed() const
{
  return CDCTangent(CDCRLWireHitPair::reversed(),
                    ParameterLine2D::throughPoints(getToRecoPos2D(), getFromRecoPos2D()));
}

Vector2D CDCTangent::getFromRecoDisp2D() const
{
  return getFromRecoPos2D() - getFromRLWireHit().getRefPos2D();
}

Vector2D CDCTangent::getToRecoDisp2D() const
{
  return getToRecoPos2D() - getToRLWireHit().getRefPos2D();
}

CDCRecoHit2D CDCTangent::getFromRecoHit2D() const
{
  return CDCRecoHit2D::fromRecoPos2D(getFromRLWireHit(), getFromRecoPos2D());
}

CDCRecoHit2D CDCTangent::getToRecoHit2D() const
{
  return CDCRecoHit2D::fromRecoPos2D(getToRLWireHit(), getToRecoPos2D());
}

std::ostream& TrackFindingCDC::operator<<(std::ostream& output, const CDCTangent& tangent)
{
  output << "Tangent" << std::endl;
  output << "From : " << tangent.getFromRLWireHit() << " " <<  tangent.getFromRecoDisp2D() << std::endl;
  output << "To : " << tangent.getToRLWireHit() << " " <<  tangent.getToRecoDisp2D()  << std::endl;
  return output;
}
