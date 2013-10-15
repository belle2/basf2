/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCRecoHit2D.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCRecoHit2D)

CDCRecoHit2D::CDCRecoHit2D() :
  m_wirehit(&(CDCWireHit::getLowest())),
  m_displacement(Vector2D::getLowest()),
  m_rlInfo(LEFT)
{;}


CDCRecoHit2D::CDCRecoHit2D(
  const CDCWireHit* wirehit,
  RightLeftInfo rlInfo
) :
  m_wirehit(wirehit),
  m_displacement(Vector2D::getLowest()),
  m_rlInfo(rlInfo)
{
  if (wirehit == nullptr) B2WARNING("Recohit with nullptr as wire hit");
}


CDCRecoHit2D::CDCRecoHit2D(
  const CDCWireHit* wirehit,
  const Vector2D& displacement,
  RightLeftInfo rlInfo
) :
  m_wirehit(wirehit),
  m_displacement(displacement),
  m_rlInfo(rlInfo)
{
  if (wirehit == nullptr) B2WARNING("Recohit with nullptr as wire hit");
}

CDCRecoHit2D::~CDCRecoHit2D() {;}

CDCRecoHit2D CDCRecoHit2D::fromSimHit(
  const CDCWireHit* wirehit,
  const CDCSimHit& simhit
)
{
  if (wirehit == nullptr) B2WARNING("Recohit with nullptr as wire hit");

  // find out if the wire is right or left of the track ( view in flight direction )
  Vector3D trackPosToWire =  simhit.getPosWire();
  trackPosToWire.subtract(simhit.getPosTrack());

  Vector3D directionOfFlight = simhit.getMomentum();

  RightLeftInfo rlInfo = trackPosToWire.xy().isRightOrLeftOf(directionOfFlight.xy());

  CDCRecoHit2D recohit2D(wirehit, Vector2D(-trackPosToWire.x(), -trackPosToWire.y()), rlInfo);
  recohit2D.snapToDriftCircle();

  return recohit2D;

}

CDCRecoHit2D
CDCRecoHit2D::average(
  const CDCRecoHit2D& recohit1,
  const CDCRecoHit2D& recohit2
)
{

  if (!(recohit1.getWireHit()->IsEqual(recohit2.getWireHit()))) {
    B2ERROR("Average of two CDCRecoHit2Ds with different wirehits requested, returning first given RecoHit");
    return recohit1;
  }

  Vector2D displacement = Vector2D::average(recohit1.getRefDisp2D(), recohit2.getRefDisp2D());

  if (recohit1.getRLInfo() *  recohit2.getRLInfo() < 0)
    B2WARNING("Averaged two CDCRecoHit2Ds with opposite left right information");

  RightLeftInfo rlInfo = averageInfo(recohit1.getRLInfo(), recohit2.getRLInfo()) ;

  CDCRecoHit2D result(recohit1.getWireHit(), displacement, rlInfo);
  result.snapToDriftCircle();

  return result;

}


CDCRecoHit2D
CDCRecoHit2D::average(
  const CDCRecoHit2D& recohit1,
  const CDCRecoHit2D& recohit2 ,
  const CDCRecoHit2D& recohit3
)
{

  if (!(recohit1.getWireHit()->IsEqual(recohit2.getWireHit()) &&
        recohit2.getWireHit()->IsEqual(recohit3.getWireHit()))) {
    B2ERROR("Average of three CDCRecoHit2Ds with different wirehits requested, returning first given RecoHit");
    return recohit1;
  }

  //check for opposite sign of the recohits
  if (recohit1.getRLInfo() * recohit2.getRLInfo() < 0  or
      recohit2.getRLInfo() * recohit3.getRLInfo() < 0  or
      recohit3.getRLInfo() * recohit1.getRLInfo() < 0)
    B2WARNING("Averaged three CDCRecoHit2Ds with incompatible left right information");


  Vector2D displacement = Vector2D::average(recohit1.getRefDisp2D() ,
                                            recohit2.getRefDisp2D() ,
                                            recohit3.getRefDisp2D());

  RightLeftInfo rlInfo = averageInfo(recohit1.getRLInfo() ,
                                     recohit2.getRLInfo() ,
                                     recohit3.getRLInfo()) ;



  CDCRecoHit2D result(recohit1.getWireHit(), displacement, rlInfo);
  result.snapToDriftCircle();

  return result;

}

CDCRecoHit2D CDCRecoHit2D::fromAbsPos2D(
  const CDCWireHit* wirehit,
  RightLeftInfo rlinfo,
  const Vector2D& pos2D,
  bool snap
)
{
  CDCRecoHit2D result(wirehit, pos2D - wirehit->getRefPos2D(), rlinfo);
  if (snap) result.snapToDriftCircle();
  return result;
}



