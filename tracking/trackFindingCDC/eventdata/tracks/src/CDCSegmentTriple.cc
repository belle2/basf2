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

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


CDCSegmentTriple::CDCSegmentTriple() :
  CDCAxialSegmentPair(),
  m_middleSegment(nullptr)
{
}


CDCSegmentTriple::CDCSegmentTriple(const CDCAxialRecoSegment2D* startSegment,
                                   const CDCAxialRecoSegment2D* endSegment) :
  CDCAxialSegmentPair(startSegment, endSegment),
  m_middleSegment(nullptr)
{
}


CDCSegmentTriple:: CDCSegmentTriple(const CDCAxialSegmentPair& segmentPair) :
  CDCAxialSegmentPair(segmentPair),
  m_middleSegment(nullptr)
{
}


CDCSegmentTriple::CDCSegmentTriple(const CDCAxialRecoSegment2D* startSegment,
                                   const CDCStereoRecoSegment2D* middleSegment,
                                   const CDCAxialRecoSegment2D* endSegment) :
  CDCAxialSegmentPair(startSegment, endSegment),
  m_middleSegment(middleSegment)
{
  B2ASSERT("CDCSegmentTriple initialized with nullptr as start segment", startSegment);
  B2ASSERT("CDCSegmentTriple initialized with nullptr as middle segment", middleSegment);
  B2ASSERT("CDCSegmentTriple initialized with nullptr as end segment", endSegment);
}


CDCSegmentTriple::CDCSegmentTriple(const CDCAxialRecoSegment2D* startSegment,
                                   const CDCStereoRecoSegment2D* middleSegment,
                                   const CDCAxialRecoSegment2D* endSegment,
                                   const CDCTrajectory2D& trajectory2D,
                                   const CDCTrajectorySZ& trajectorySZ) :
  CDCAxialSegmentPair(startSegment, endSegment, trajectory2D),
  m_middleSegment(middleSegment),
  m_trajectorySZ(trajectorySZ)
{
  B2ASSERT("CDCSegmentTriple initialized with nullptr as start segment", startSegment);
  B2ASSERT("CDCSegmentTriple initialized with nullptr as middle segment", middleSegment);
  B2ASSERT("CDCSegmentTriple initialized with nullptr as end segment", endSegment);
}
