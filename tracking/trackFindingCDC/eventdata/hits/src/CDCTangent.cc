/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/hits/CDCTangent.h>
#include <cmath>

using namespace std;
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
  m_line = ParameterLine2D::touchingCircles(getFromWireHit().getRefPos2D(),
                                            getFromRLInfo() * getFromRLWireHit().getRefDriftLength(),
                                            getToWireHit().getRefPos2D(),
                                            getToRLInfo() * getToRLWireHit().getRefDriftLength());
}

void CDCTangent::adjustRLInfo()
{
  ERightLeft newFromRLInfo = getLine().isRightOrLeft(getFromWireHit()->getRefPos2D());
  setFromRLInfo(newFromRLInfo);

  ERightLeft newToRLInfo = getLine().isRightOrLeft(getToWireHit()->getRefPos2D());
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
