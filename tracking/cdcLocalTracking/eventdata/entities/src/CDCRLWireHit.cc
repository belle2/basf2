/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCRLWireHit.h"

#include <tracking/cdcLocalTracking/eventtopology/CDCWireHitTopology.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCRLWireHit)

CDCRLWireHit::CDCRLWireHit() :
  m_wirehit(&(CDCWireHit::getLowest())),
  m_rlInfo(LEFT)
{;}


CDCRLWireHit::CDCRLWireHit(
  const CDCWireHit* wirehit,
  RightLeftInfo rlInfo
) :
  m_wirehit(wirehit),
  m_rlInfo(rlInfo)
{
  if (wirehit == nullptr) B2WARNING("Recohit with nullptr as wire hit");
}

CDCRLWireHit::~CDCRLWireHit() {;}

CDCRLWireHit CDCRLWireHit::fromSimHit(
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

  CDCRLWireHit rlWireHit(wirehit, rlInfo);

  return rlWireHit;

}


const CDCRLWireHit* CDCRLWireHit::reversed() const
{ return CDCWireHitTopology::getInstance().getReverseOf(*this); }
