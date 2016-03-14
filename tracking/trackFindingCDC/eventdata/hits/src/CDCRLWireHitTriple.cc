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


CDCRLWireHitTriple::Shape CDCRLWireHitTriple::getShape() const
{
  const CDCWire& startWire = getStartWire();
  const CDCWire& middleWire = getMiddleWire();
  const CDCWire& endWire = getEndWire();

  WireNeighborKind startToMiddleNeighborKind = startWire.getNeighborKind(middleWire);
  WireNeighborKind middleToEndNeighborKind   = middleWire.getNeighborKind(endWire);

  if (not startToMiddleNeighborKind.isValid() or
      not middleToEndNeighborKind.isValid()) {
    return Shape();
  }

  // Neighbor types are marked on the clock. Difference is so to say an angular value apart from a 12 / (2 * pi) factor
  const short oClockDelta = middleToEndNeighborKind.getOClockDirection() - startToMiddleNeighborKind.getOClockDirection();
  const short startToMiddleCellDistance = startToMiddleNeighborKind.getCellDistance();
  const short middleToEndCellDistance =  middleToEndNeighborKind.getCellDistance();
  return Shape(startToMiddleCellDistance,
               middleToEndCellDistance,
               symmetricModulo(oClockDelta, 12));
}
