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

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


CDCRLWireHitTriple::CDCRLWireHitTriple():
  m_startRLWireHit(nullptr),
  m_rearRLWireHitPair(nullptr, nullptr)
{
  B2WARNING("CDCRLWireHitTiple initialized with three nullptr");
}

CDCRLWireHitTriple::CDCRLWireHitTriple(
  const CDCRLWireHit* startRLWireHit,
  const CDCRLWireHit* middleRLWireHit,
  const CDCRLWireHit* endRLWireHit
):
  m_startRLWireHit(startRLWireHit),
  m_rearRLWireHitPair(middleRLWireHit, endRLWireHit)
{
  assert(startRLWireHit);
  assert(middleRLWireHit);
  assert(endRLWireHit);
}

CDCRLWireHitTriple CDCRLWireHitTriple::reversed() const
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  return CDCRLWireHitTriple(wireHitTopology.getReverseOf(getEndRLWireHit()),
                            wireHitTopology.getReverseOf(getMiddleRLWireHit()),
                            wireHitTopology.getReverseOf(getStartRLWireHit()));
}

void CDCRLWireHitTriple::reverse()
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  const CDCRLWireHit* newStartRLWireHit = wireHitTopology.getReverseOf(getEndRLWireHit());
  const CDCRLWireHit* newMiddleRLWireHit = wireHitTopology.getReverseOf(getMiddleRLWireHit());
  const CDCRLWireHit* newEndRLWireHit = wireHitTopology.getReverseOf(getStartRLWireHit());

  setStartRLWireHit(newStartRLWireHit);
  setMiddleRLWireHit(newMiddleRLWireHit);
  setEndRLWireHit(newEndRLWireHit);

}

void CDCRLWireHitTriple::setStartRLInfo(const RightLeftInfo startRLInfo)
{
  if (startRLInfo != getStartRLInfo()) {
    const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
    const CDCRLWireHit* newStartRLWireHit = wireHitTopology.getReverseOf(getStartRLWireHit());
    setStartRLWireHit(newStartRLWireHit);
  }
}



void CDCRLWireHitTriple::setMiddleRLInfo(const RightLeftInfo middleRLInfo)
{
  getRearRLWireHitPair().setFromRLInfo(middleRLInfo);
}



void CDCRLWireHitTriple::setEndRLInfo(const RightLeftInfo endRLInfo)
{
  getRearRLWireHitPair().setToRLInfo(endRLInfo);
}



CDCRLWireHitTriple::Shape CDCRLWireHitTriple::getShape() const
{
  const CDCWire& startWire = getStartWire();
  const CDCWire& middleWire = getMiddleWire();
  const CDCWire& endWire = getEndWire();

  WireNeighborType startToMiddleNeighborType = startWire.isNeighborWith(middleWire);
  WireNeighborType middleToEndNeighborType   = middleWire.isNeighborWith(endWire);

  if (startToMiddleNeighborType == NOT_NEIGHBORS or
      middleToEndNeighborType == NOT_NEIGHBORS) {
    return ILLSHAPED;
  }

  // Neighbor types are marked on the clock. Difference is so to say an angular value apart from a 12 / (2 * pi) factor
  const int clockDifference = (int)startToMiddleNeighborType - (int)middleToEndNeighborType;

  // Difference on the clock modulus 12 such that it is between -6 and 6.
  Shape shape = (clockDifference + 18) % 12 - 6;

  return shape;
}
