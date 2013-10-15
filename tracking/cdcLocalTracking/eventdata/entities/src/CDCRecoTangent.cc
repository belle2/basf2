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
#include <TObjArray.h>



using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


ClassImpInCDCLocalTracking(CDCRecoTangent)

CDCRecoTangent::CDCRecoTangent():
  m_wirehits(&(CDCWireHit::getLowest()) , &(CDCWireHit::getLowest())),
  m_fromRLInfo(LEFT),
  m_toRLInfo(LEFT)
{;}

CDCRecoTangent::CDCRecoTangent(
  const CDCWireHit* fromWireHit,
  const RightLeftInfo& fromRL,

  const CDCWireHit* toWireHit,
  const RightLeftInfo& toRL
) :
  m_wirehits(fromWireHit, toWireHit),
  m_fromRLInfo(fromRL),
  m_toRLInfo(toRL)
{
  adjustLine();
  if (fromWireHit == nullptr) B2WARNING("Tangent with nullptr as wire hit");
  if (toWireHit == nullptr) B2WARNING("Tangent with nullptr as wire hit");
}

CDCRecoTangent::CDCRecoTangent(
  const CDCRecoHit2D& fromRecoHit,
  const CDCRecoHit2D& toRecoHit
) :
  m_wirehits(fromRecoHit.getWireHit(), toRecoHit.getWireHit()),
  m_fromRLInfo(fromRecoHit.getRLInfo()),
  m_toRLInfo(toRecoHit.getRLInfo()),
  m_line(ParameterLine2D::throughPoints(fromRecoHit.getRefPos2D(),
                                        toRecoHit.getRefPos2D()))
{;}


CDCRecoTangent::CDCRecoTangent(
  const CDCWireHit* fromWireHit,
  const RightLeftInfo& fromRL,

  const CDCWireHit* toWireHit,
  const RightLeftInfo& toRL,
  const ParameterLine2D& line
) :
  m_wirehits(fromWireHit, toWireHit),
  m_fromRLInfo(fromRL),
  m_toRLInfo(toRL),
  m_line(line)
{
  if (fromWireHit == nullptr) B2WARNING("Tangent with nullptr as wire hit");
  if (toWireHit == nullptr) B2WARNING("Tangent with nullptr as wire hit");
}



/** Destructor. */
CDCRecoTangent::~CDCRecoTangent() {;}

void CDCRecoTangent::adjustLine()
{

  m_line  = constructTouchingLine(
              getFromWireHit()->getRefPos2D(),
              getFromRLInfo() * getFromWireHit()->getRefDriftLength() ,
              getToWireHit()->getRefPos2D(),
              getToRLInfo() * getToWireHit()->getRefDriftLength()
            );

}

void CDCRecoTangent::adjustRLInfo()
{

  if (getFromWireHit()->getRefDriftLength() == 0.0) {
    m_fromRLInfo = 0;
  } else {
    m_fromRLInfo = getLine().isRightOrLeft(getFromWireHit()->getRefPos2D());
  }
  if (getToWireHit()->getRefDriftLength() == 0.0) {
    m_toRLInfo = 0;
  } else {
    m_toRLInfo   = getLine().isRightOrLeft(getToWireHit()->getRefPos2D());
  }

}

void CDCRecoTangent::reverse()
{
  std::swap(m_wirehits.first, m_wirehits.second);
  std::swap(m_fromRLInfo, m_toRLInfo);
  m_fromRLInfo = -m_fromRLInfo;
  m_toRLInfo   = -m_toRLInfo;

  //reverse the direction of flight
  m_line.reverse();

  //adjust the support point to be now at the other wirehit touch position
  m_line.passiveMoveAtBy(-1);
}

CDCRecoTangent CDCRecoTangent::reversed() const
{

  return CDCRecoTangent(getToWireHit(),  -getToRLInfo(),
                        getFromWireHit(), -getFromRLInfo(),
                        ParameterLine2D::throughPoints(getToRefTouch2D(), getFromRefTouch2D()));
}

/** This returns sum of the squared distances of the two touch points to the circle */
FloatType CDCRecoTangent::getSquaredDist2D(const  CDCTrajectory2D& trajectory2D) const
{
  FloatType fromDistance =  trajectory2D.getDist2D(getFromRefTouch2D());
  FloatType toDistance   =  trajectory2D.getDist2D(getToRefTouch2D());
  return fromDistance * fromDistance + toDistance * toDistance;
}




/** Prints all wires contained in the collection mainly for debuging */
/*ostream& operator<<(ostream& output, const CDCRecoTangent& tangent){
  output << "RecoTangent" << endl;
  output << "From : " << *(tangent.getFromRecoHit().getWire()) << endl;
  output << "To : " << *(tangent.getToRecoHit().getWire()) << endl;
  return output;
}*/

const CDCWireHit* CDCRecoTangent::commonWireHit(const CDCRecoTangent& recoTangent) const
{

  if (getFromWireHit() != nullptr and recoTangent.hasWireHit(*getFromWireHit())) return getFromWireHit();
  else if (getFromWireHit() != nullptr and recoTangent.hasWireHit(*getToWireHit())) return getToWireHit();
  else return nullptr;

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

