/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLTaggedWireHit.h>

#include <cdc/dataobjects/CDCSimHit.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCRLTaggedWireHit::CDCRLTaggedWireHit(const CDCWireHit* wireHit,
                                       ERightLeft rlInfo) :
  m_wireHit(wireHit),
  m_rlInfo(rlInfo)
{
  B2ASSERT("Recohit with nullptr as wire hit", wireHit != nullptr);
}

CDCRLTaggedWireHit CDCRLTaggedWireHit::fromSimHit(const CDCWireHit* wirehit,
                                                  const CDCSimHit& simhit)
{
  // find out if the wire is right or left of the track ( view in flight direction )
  Vector3D trackPosToWire{simhit.getPosWire() - simhit.getPosTrack()};
  Vector3D directionOfFlight{simhit.getMomentum()};

  ERightLeft rlInfo = trackPosToWire.xy().isRightOrLeftOf(directionOfFlight.xy());

  CDCRLTaggedWireHit rlWireHit(wirehit, rlInfo);

  return rlWireHit;
}
