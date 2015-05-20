/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/entities/CDCTangent.h>

#include <cmath>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


TRACKFINDINGCDC_SwitchableClassImp(CDCTangent)



CDCTangent::CDCTangent(): CDCRLWireHitPair(), m_line() {;}



CDCTangent::CDCTangent(const CDCRLWireHitPair& rlWireHitPair):
  CDCRLWireHitPair(rlWireHitPair), m_line()
{
  adjustLine();
}



CDCTangent::CDCTangent(const CDCRLWireHit* fromRLWireHit, const CDCRLWireHit* toRLWireHit):
  CDCRLWireHitPair(fromRLWireHit, toRLWireHit), m_line()
{
  adjustLine();
}



CDCTangent::CDCTangent(const CDCRLWireHitPair& rlWireHitPair, const ParameterLine2D& line):
  CDCRLWireHitPair(rlWireHitPair), m_line(line)
{;}



CDCTangent::CDCTangent(const CDCRLWireHit* fromRLWireHit, const CDCRLWireHit* toRLWireHit, const ParameterLine2D& line):
  CDCRLWireHitPair(fromRLWireHit, toRLWireHit), m_line(line)
{;}



CDCTangent::CDCTangent(
  const CDCRecoHit2D& fromRecoHit,
  const CDCRecoHit2D& toRecoHit
):
  CDCRLWireHitPair(&(fromRecoHit.getRLWireHit()), &(toRecoHit.getRLWireHit())),
  m_line(ParameterLine2D::throughPoints(fromRecoHit.getRecoPos2D(),
                                        toRecoHit.getRecoPos2D()))
{;}



CDCTangent::~CDCTangent() {;}



void CDCTangent::adjustLine()
{

  m_line  = constructTouchingLine(
              getFromWireHit().getRefPos2D(),
              getFromRLInfo() * getFromWireHit().getRefDriftLength() ,
              getToWireHit().getRefPos2D(),
              getToRLInfo() * getToWireHit().getRefDriftLength()
            );

}

void CDCTangent::adjustRLInfo()
{

  RightLeftInfo newFromRLInfo = getLine().isRightOrLeft(getFromWireHit()->getRefPos2D());
  setFromRLInfo(newFromRLInfo);

  RightLeftInfo newToRLInfo = getLine().isRightOrLeft(getToWireHit()->getRefPos2D());
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

/** This returns sum of the squared distances of the two touch points to the circle */
FloatType CDCTangent::getSquaredDist2D(const  CDCTrajectory2D& trajectory2D) const
{
  FloatType fromDistance =  trajectory2D.getDist2D(getFromRecoPos2D());
  FloatType toDistance   =  trajectory2D.getDist2D(getToRecoPos2D());
  return fromDistance * fromDistance + toDistance * toDistance;
}



ParameterLine2D CDCTangent::constructTouchingLine(
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

