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

using namespace Belle2;
using namespace TrackFindingCDC;


CDCSegmentTriple::CDCSegmentTriple()
  : m_startSegment(nullptr),
    m_middleSegment(nullptr),
    m_endSegment(nullptr)
{
}


CDCSegmentTriple::CDCSegmentTriple(const CDCAxialSegment2D* startSegment,
                                   const CDCAxialSegment2D* endSegment)
  : m_startSegment(startSegment),
    m_middleSegment(nullptr),
    m_endSegment(endSegment)
{
}


CDCSegmentTriple:: CDCSegmentTriple(const CDCAxialSegmentPair& segmentPair)
  : m_startSegment(segmentPair.getStartSegment()),
    m_middleSegment(nullptr),
    m_endSegment(segmentPair.getEndSegment())
{
}


CDCSegmentTriple::CDCSegmentTriple(const CDCAxialSegment2D* startSegment,
                                   const CDCStereoSegment2D* middleSegment,
                                   const CDCAxialSegment2D* endSegment)
  : m_startSegment(startSegment),
    m_middleSegment(middleSegment),
    m_endSegment(endSegment)
{
  B2ASSERT("CDCSegmentTriple initialized with nullptr as start segment", startSegment);
  B2ASSERT("CDCSegmentTriple initialized with nullptr as middle segment", middleSegment);
  B2ASSERT("CDCSegmentTriple initialized with nullptr as end segment", endSegment);
}


CDCSegmentTriple::CDCSegmentTriple(const CDCAxialSegment2D* startSegment,
                                   const CDCStereoSegment2D* middleSegment,
                                   const CDCAxialSegment2D* endSegment,
                                   const CDCTrajectory3D& trajectory3D)
  : m_startSegment(startSegment),
    m_middleSegment(middleSegment),
    m_endSegment(endSegment),
    m_trajectory3D(trajectory3D)
{
  B2ASSERT("CDCSegmentTriple initialized with nullptr as start segment", startSegment);
  B2ASSERT("CDCSegmentTriple initialized with nullptr as middle segment", middleSegment);
  B2ASSERT("CDCSegmentTriple initialized with nullptr as end segment", endSegment);
}
