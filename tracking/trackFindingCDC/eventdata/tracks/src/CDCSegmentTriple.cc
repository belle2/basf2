/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCAxialSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCStereoSegment2D.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCSegmentTriple::CDCSegmentTriple()
  : m_startSegment(nullptr)
  , m_middleSegment(nullptr)
  , m_endSegment(nullptr)
{
}

CDCSegmentTriple::CDCSegmentTriple(const CDCAxialSegment2D* startSegment,
                                   const CDCAxialSegment2D* endSegment)
  : m_startSegment(startSegment)
  , m_middleSegment(nullptr)
  , m_endSegment(endSegment)
{
}

CDCSegmentTriple::CDCSegmentTriple(const CDCAxialSegmentPair& segmentPair)
  : m_startSegment(segmentPair.getStartSegment())
  , m_middleSegment(nullptr)
  , m_endSegment(segmentPair.getEndSegment())
{
}

CDCSegmentTriple::CDCSegmentTriple(const CDCAxialSegment2D* startSegment,
                                   const CDCStereoSegment2D* middleSegment,
                                   const CDCAxialSegment2D* endSegment)
  : m_startSegment(startSegment)
  , m_middleSegment(middleSegment)
  , m_endSegment(endSegment)
{
  B2ASSERT("CDCSegmentTriple initialized with nullptr as start segment", startSegment);
  B2ASSERT("CDCSegmentTriple initialized with nullptr as middle segment", middleSegment);
  B2ASSERT("CDCSegmentTriple initialized with nullptr as end segment", endSegment);
}

CDCSegmentTriple::CDCSegmentTriple(const CDCAxialSegment2D* startSegment,
                                   const CDCStereoSegment2D* middleSegment,
                                   const CDCAxialSegment2D* endSegment,
                                   const CDCTrajectory3D& trajectory3D)
  : m_startSegment(startSegment)
  , m_middleSegment(middleSegment)
  , m_endSegment(endSegment)
  , m_trajectory3D(trajectory3D)
{
  B2ASSERT("CDCSegmentTriple initialized with nullptr as start segment", startSegment);
  B2ASSERT("CDCSegmentTriple initialized with nullptr as middle segment", middleSegment);
  B2ASSERT("CDCSegmentTriple initialized with nullptr as end segment", endSegment);
}

ISuperLayer CDCSegmentTriple::getStartISuperLayer() const
{
  return ISuperLayerUtil::getFrom(getStartSegment());
}

ISuperLayer CDCSegmentTriple::getMiddleISuperLayer() const
{
  return ISuperLayerUtil::getFrom(getMiddleSegment());
}

ISuperLayer CDCSegmentTriple::getEndISuperLayer() const
{
  return ISuperLayerUtil::getFrom(getEndSegment());
}

CDCTrajectorySZ CDCSegmentTriple::getTrajectorySZ() const
{
  return getTrajectory3D().getTrajectorySZ();
}

CDCTrajectory2D CDCSegmentTriple::getTrajectory2D() const
{
  return getTrajectory3D().getTrajectory2D();
}

void CDCSegmentTriple::unsetAndForwardMaskedFlag() const
{
  getAutomatonCell().unsetMaskedFlag();
  const bool toHits = true;
  getStartSegment()->unsetAndForwardMaskedFlag(toHits);
  getMiddleSegment()->unsetAndForwardMaskedFlag(toHits);
  getEndSegment()->unsetAndForwardMaskedFlag(toHits);
}

void CDCSegmentTriple::setAndForwardMaskedFlag() const
{
  getAutomatonCell().setMaskedFlag();
  const bool toHits = true;
  getStartSegment()->setAndForwardMaskedFlag(toHits);
  getMiddleSegment()->setAndForwardMaskedFlag(toHits);
  getEndSegment()->setAndForwardMaskedFlag(toHits);
}

void CDCSegmentTriple::receiveMaskedFlag() const
{
  const bool fromHits = true;
  getStartSegment()->receiveMaskedFlag(fromHits);
  getMiddleSegment()->receiveMaskedFlag(fromHits);
  getEndSegment()->receiveMaskedFlag(fromHits);

  if (getStartSegment()->getAutomatonCell().hasMaskedFlag() or
      getMiddleSegment()->getAutomatonCell().hasMaskedFlag() or
      getEndSegment()->getAutomatonCell().hasMaskedFlag()) {

    getAutomatonCell().setMaskedFlag();
  }
}
