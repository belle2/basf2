/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHitTriple.h>

#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCRLWireHitTriple::CDCRLWireHitTriple(
  const CDCRLTaggedWireHit& startRLWireHit,
  const CDCRLTaggedWireHit& middleRLWireHit,
  const CDCRLTaggedWireHit& endRLWireHit
):
  m_startRLWireHit(startRLWireHit),
  m_rearRLWireHitPair(middleRLWireHit, endRLWireHit)
{
}

CDCRLWireHitTriple CDCRLWireHitTriple::reversed() const
{
  return CDCRLWireHitTriple(getEndRLWireHit().reversed(),
                            getMiddleRLWireHit().reversed(),
                            getStartRLWireHit().reversed());
}

void CDCRLWireHitTriple::reverse()
{
  CDCRLTaggedWireHit newEndRLWireHit = getStartRLWireHit().reversed();
  setMiddleRLWireHit(getEndRLWireHit().reversed());
  setMiddleRLWireHit(getMiddleRLWireHit().reversed());
  setEndRLWireHit(newEndRLWireHit);
}

CDCRLWireHitTriple::EShape CDCRLWireHitTriple::getShape() const
{
  const CDCWire& startWire = getStartWire();
  const CDCWire& middleWire = getMiddleWire();
  const CDCWire& endWire = getEndWire();

  EWireNeighborKind startToMiddleNeighborKind = startWire.getNeighborKind(middleWire);
  EWireNeighborKind middleToEndNeighborKind   = middleWire.getNeighborKind(endWire);

  if (startToMiddleNeighborKind == EWireNeighborKind::c_None or
      middleToEndNeighborKind == EWireNeighborKind::c_None) {
    return EShape::c_Invalid;
  }

  // Neighbor types are marked on the clock. Difference is so to say an angular value apart from a 12 / (2 * pi) factor
  const int clockDifference = (int)startToMiddleNeighborKind - (int)middleToEndNeighborKind;

  // Difference on the clock modulus 12 such that it is between -6 and 6.
  EShape shape = EShape((clockDifference + 18) % 12 - 6);

  return shape;
}
