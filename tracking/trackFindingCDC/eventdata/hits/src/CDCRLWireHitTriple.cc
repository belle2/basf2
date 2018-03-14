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

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/topology/WireNeighborKind.h>
#include <tracking/trackFindingCDC/numerics/Modulo.h>

#include <iostream>
#include <limits.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCRLWireHitTriple::Shape::Shape()
  : m_startToMiddleCellDistance(CHAR_MAX / 2)
  , m_middleToEndCellDistance(CHAR_MAX / 2)
  , m_oClockDelta(SHRT_MIN)
{
}

CDCRLWireHitTriple::Shape::Shape(const short startToMiddleCellDistance,
                                 const short middleToEndCellDistance,
                                 const short oClockDelta)
  : m_startToMiddleCellDistance(startToMiddleCellDistance)
  , m_middleToEndCellDistance(middleToEndCellDistance)
  , m_oClockDelta(oClockDelta)
{
}

short CDCRLWireHitTriple::Shape::getStartToMiddleCellDistance() const
{
  return m_startToMiddleCellDistance;
}

short CDCRLWireHitTriple::Shape::getMiddleToEndCellDistance() const
{
  return m_middleToEndCellDistance;
}

short CDCRLWireHitTriple::Shape::getCellExtend() const
{
  return m_startToMiddleCellDistance + m_middleToEndCellDistance;
}

short CDCRLWireHitTriple::Shape::getOClockDelta() const
{
  return m_oClockDelta;
}

bool CDCRLWireHitTriple::Shape::isValid() const
{
  return getCellExtend() >= 2 and getCellExtend() <= 4;
}

CDCRLWireHitTriple::CDCRLWireHitTriple(const CDCRLWireHit& startRLWireHit,
                                       const CDCRLWireHit& middleRLWireHit,
                                       const CDCRLWireHit& endRLWireHit,
                                       int iCluster)
  : m_startRLWireHit(startRLWireHit)
  , m_rearRLWireHitPair(middleRLWireHit, endRLWireHit, iCluster)
{
}

CDCRLWireHitTriple CDCRLWireHitTriple::reversed() const
{
  return CDCRLWireHitTriple(getEndRLWireHit().reversed(),
                            getMiddleRLWireHit().reversed(),
                            getStartRLWireHit().reversed(),
                            getICluster());
}

void CDCRLWireHitTriple::reverse()
{
  CDCRLWireHit newEndRLWireHit = getStartRLWireHit().reversed();
  setMiddleRLWireHit(getEndRLWireHit().reversed());
  setMiddleRLWireHit(getMiddleRLWireHit().reversed());
  setEndRLWireHit(newEndRLWireHit);
}

CDCRLWireHitTriple CDCRLWireHitTriple::getAlias() const
{
  return CDCRLWireHitTriple(getStartRLWireHit().getAlias(),
                            getMiddleRLWireHit().getAlias(),
                            getEndRLWireHit().getAlias(),
                            getICluster());
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
               symmetricModuloFast(oClockDelta, 12));
}

std::ostream& TrackFindingCDC::
operator<<(std::ostream& output, const CDCRLWireHitTriple& rlWireHitTriple)
{
  return (output << "Start : " << rlWireHitTriple.getStartRLWireHit() << " "
          << "Middle : "
          << rlWireHitTriple.getMiddleRLWireHit()
          << " "
          << "End : "
          << rlWireHitTriple.getEndRLWireHit());
}
