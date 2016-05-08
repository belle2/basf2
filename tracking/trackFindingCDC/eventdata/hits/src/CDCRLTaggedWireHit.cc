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
                                       ERightLeft rlInfo)
  : CDCRLTaggedWireHit(wireHit,
                       rlInfo,
                       wireHit->getRefDriftLength())
{
}

CDCRLTaggedWireHit::CDCRLTaggedWireHit(const CDCWireHit* wireHit,
                                       ERightLeft rlInfo,
                                       double driftLength)
  : m_wireHit(wireHit),
    m_rlInfo(rlInfo),
    m_refDriftLength(driftLength)
{
}

CDCRLTaggedWireHit CDCRLTaggedWireHit::average(const CDCRLTaggedWireHit& rlWireHit1,
                                               const CDCRLTaggedWireHit& rlWireHit2)
{
  B2ASSERT("Average of two CDCRLTaggedWireHits with different wire hits requested.",
           rlWireHit1.getWireHit() == rlWireHit2.getWireHit());

  B2ASSERT("Average of two CDCRLTaggedWireHits with different right left passage information requested.",
           rlWireHit1.getRLInfo() == rlWireHit2.getRLInfo());

  ERightLeft rlInfo = rlWireHit1.getRLInfo();
  const CDCWireHit& wireHit = rlWireHit1.getWireHit();

  double driftLength = (rlWireHit1.getRefDriftLength() +
                        rlWireHit2.getRefDriftLength()) / 2.0;

  CDCRLTaggedWireHit result(&wireHit, rlInfo, driftLength);
  return result;
}

CDCRLTaggedWireHit CDCRLTaggedWireHit::average(const CDCRLTaggedWireHit& rlWireHit1,
                                               const CDCRLTaggedWireHit& rlWireHit2,
                                               const CDCRLTaggedWireHit& rlWireHit3)
{
  B2ASSERT("Average of three CDCRLTaggedWireHits with different wire hits requested.",
           rlWireHit1.getWireHit() == rlWireHit2.getWireHit() and
           rlWireHit2.getWireHit() == rlWireHit3.getWireHit());

  B2ASSERT("Average of three CDCRLTaggedWireHits with different right left passage information requested.",
           rlWireHit1.getRLInfo() == rlWireHit2.getRLInfo() and
           rlWireHit2.getRLInfo() == rlWireHit3.getRLInfo());


  ERightLeft rlInfo = rlWireHit1.getRLInfo();
  const CDCWireHit& wireHit = rlWireHit1.getWireHit();

  double driftLength = (rlWireHit1.getRefDriftLength() +
                        rlWireHit2.getRefDriftLength() +
                        rlWireHit3.getRefDriftLength()) / 3.0;

  CDCRLTaggedWireHit result(&wireHit, rlInfo, driftLength);
  return result;
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
