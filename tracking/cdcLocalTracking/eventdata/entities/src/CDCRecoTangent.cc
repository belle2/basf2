/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCRecoTangent.h"

#include <cmath>

#include <tracking/cdcLocalTracking/eventtopology/CDCWireHitTopology.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


ClassImpInCDCLocalTracking(CDCRecoTangent)



CDCRecoTangent::CDCRecoTangent(): CDCRLWireHitPair(), m_line() {;}



CDCRecoTangent::CDCRecoTangent(const CDCRLWireHitPair& rlWireHitPair):
  CDCRLWireHitPair(rlWireHitPair), m_line()
{
  adjustLine();
}



CDCRecoTangent::CDCRecoTangent(const CDCRLWireHit* fromRLWireHit, const CDCRLWireHit* toRLWireHit):
  CDCRLWireHitPair(fromRLWireHit, toRLWireHit), m_line()
{
  adjustLine();
}



CDCRecoTangent::CDCRecoTangent(const CDCRLWireHitPair& rlWireHitPair, const ParameterLine2D& line):
  CDCRLWireHitPair(rlWireHitPair), m_line(line)
{;}



CDCRecoTangent::CDCRecoTangent(const CDCRLWireHit* fromRLWireHit, const CDCRLWireHit* toRLWireHit, const ParameterLine2D& line):
  CDCRLWireHitPair(fromRLWireHit, toRLWireHit), m_line(line)
{;}



CDCRecoTangent::CDCRecoTangent(
  const CDCRecoHit2D& fromRecoHit,
  const CDCRecoHit2D& toRecoHit
):
  CDCRLWireHitPair(CDCWireHitTopology::getInstance().getRLWireHit(*(fromRecoHit.getWireHit()), fromRecoHit.getRLInfo()),
                   CDCWireHitTopology::getInstance().getRLWireHit(*(toRecoHit.getWireHit()), toRecoHit.getRLInfo())),
  m_line(ParameterLine2D::throughPoints(fromRecoHit.getRefPos2D(),
                                        toRecoHit.getRefPos2D()))
{;}



CDCRecoTangent::~CDCRecoTangent() {;}



void CDCRecoTangent::adjustLine()
{

  m_line  = constructTouchingLine(
              getFromWireHit().getRefPos2D(),
              getFromRLInfo() * getFromWireHit().getRefDriftLength() ,
              getToWireHit().getRefPos2D(),
              getToRLInfo() * getToWireHit().getRefDriftLength()
            );

}

void CDCRecoTangent::adjustRLInfo()
{

  RightLeftInfo newFromRLInfo = getLine().isRightOrLeft(getFromWireHit()->getRefPos2D());
  setFromRLInfo(newFromRLInfo);

  RightLeftInfo newToRLInfo = getLine().isRightOrLeft(getToWireHit()->getRefPos2D());
  setToRLInfo(newToRLInfo);

}

void CDCRecoTangent::reverse()
{

  CDCRLWireHitPair::reverse();

  //reverse the direction of flight
  m_line.reverse();

  //adjust the support point to be now at the other wirehit touch position
  m_line.passiveMoveAtBy(-1);
}

CDCRecoTangent CDCRecoTangent::reversed() const
{
  return CDCRecoTangent(CDCRLWireHitPair::reversed(),
                        ParameterLine2D::throughPoints(getToRefTouch2D(), getFromRefTouch2D()));
}

/** This returns sum of the squared distances of the two touch points to the circle */
FloatType CDCRecoTangent::getSquaredDist2D(const  CDCTrajectory2D& trajectory2D) const
{
  FloatType fromDistance =  trajectory2D.getDist2D(getFromRefTouch2D());
  FloatType toDistance   =  trajectory2D.getDist2D(getToRefTouch2D());
  return fromDistance * fromDistance + toDistance * toDistance;
}



ParameterLine2D CDCRecoTangent::constructTouchingLine(
  const Vector2D& fromCenter,
  const FloatType& fromSignedRadius,
  const Vector2D& toCenter,
  const FloatType& toSignedRadius
)
{

  Vector2D connecting = toCenter - fromCenter;

  //Normalize to the coordinate system vector, but keep the original norm
  const FloatType norm = connecting.normalize();

  FloatType kappa = (fromSignedRadius - toSignedRadius) / norm;
  FloatType cokappa = sqrt(1 - kappa * kappa);

  Vector2D fromPos = Vector2D(connecting, kappa * fromSignedRadius, cokappa * fromSignedRadius);
  fromPos += fromCenter;

  Vector2D toPos   = Vector2D(connecting, kappa * toSignedRadius,   cokappa * toSignedRadius);
  toPos   += toCenter;

  // if ( isNAN(toPos.x()) or isNAN(toPos.y()) or isNAN(fromPos.x()) or isNAN(fromPos.y()) ){

  //   cout << fromCenter << endl;
  //   cout << fromSignedRadius << endl;
  //   cout << toCenter << endl;
  //   cout << toSignedRadius << endl << endl;

  //   cout << toCenter - fromCenter << endl;
  //   cout << norm << endl;
  //   cout << ( fromSignedRadius - toSignedRadius ) << endl;
  //   cout << kappa << endl;
  //   cout << cokappa << endl << endl;

  //   cout << fromPos << endl;
  //   cout << toPos << endl << endl << endl;
  // }

  return ParameterLine2D::throughPoints(fromPos, toPos);

}

