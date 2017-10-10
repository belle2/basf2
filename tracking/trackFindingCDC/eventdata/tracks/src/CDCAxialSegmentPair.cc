/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCAxialSegment2D.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>

#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCAxialSegmentPair::CDCAxialSegmentPair()
  : m_startSegment(nullptr)
  , m_endSegment(nullptr)
{
}

CDCAxialSegmentPair::CDCAxialSegmentPair(const CDCAxialSegment2D* startSegment,
                                         const CDCAxialSegment2D* endSegment)
  : m_startSegment(startSegment)
  , m_endSegment(endSegment)
{
  B2ASSERT("CDCAxialSegmentPair initialized with nullptr as start segment", startSegment);
  B2ASSERT("CDCAxialSegmentPair initialized with nullptr as end segment", endSegment);
}

CDCAxialSegmentPair::CDCAxialSegmentPair(const CDCAxialSegment2D* startSegment,
                                         const CDCAxialSegment2D* endSegment,
                                         const CDCTrajectory2D& trajectory2D)
  : m_startSegment(startSegment)
  , m_endSegment(endSegment)
  , m_trajectory2D(trajectory2D)
{
  B2ASSERT("CDCAxialSegmentPair initialized with nullptr as start segment", startSegment);
  B2ASSERT("CDCAxialSegmentPair initialized with nullptr as end segment", endSegment);
}

ISuperLayer CDCAxialSegmentPair::getStartISuperLayer() const
{
  return ISuperLayerUtil::getFrom(getStartSegment());
}

ISuperLayer CDCAxialSegmentPair::getEndISuperLayer() const
{
  return ISuperLayerUtil::getFrom(getEndSegment());
}

void CDCAxialSegmentPair::unsetAndForwardMaskedFlag() const
{
  getAutomatonCell().unsetMaskedFlag();
  const bool toHits = true;
  getStartSegment()->unsetAndForwardMaskedFlag(toHits);
  getEndSegment()->unsetAndForwardMaskedFlag(toHits);
}

void CDCAxialSegmentPair::setAndForwardMaskedFlag() const
{
  getAutomatonCell().setMaskedFlag();
  const bool toHits = true;
  getStartSegment()->setAndForwardMaskedFlag(toHits);
  getEndSegment()->setAndForwardMaskedFlag(toHits);
}

void CDCAxialSegmentPair::receiveMaskedFlag() const
{
  const bool fromHits = true;
  getStartSegment()->receiveMaskedFlag(fromHits);
  getEndSegment()->receiveMaskedFlag(fromHits);

  if (getStartSegment()->getAutomatonCell().hasMaskedFlag() or
      getEndSegment()->getAutomatonCell().hasMaskedFlag()) {
    getAutomatonCell().setMaskedFlag();
  }
}

EForwardBackward CDCAxialSegmentPair::isCoaligned(const CDCTrajectory2D& trajectory2D) const
{
  EForwardBackward startIsCoaligned = trajectory2D.isForwardOrBackwardTo(*(getStartSegment()));
  EForwardBackward endIsCoaligned = trajectory2D.isForwardOrBackwardTo(*(getEndSegment()));
  if (startIsCoaligned == EForwardBackward::c_Forward and
      endIsCoaligned == EForwardBackward::c_Forward) {
    return EForwardBackward::c_Forward;
  } else if (startIsCoaligned == EForwardBackward::c_Backward and
             endIsCoaligned == EForwardBackward::c_Backward) {
    return EForwardBackward::c_Backward;
  } else {
    return EForwardBackward::c_Invalid;
  }
}
