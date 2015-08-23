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

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


CDCAxialSegmentPair::CDCAxialSegmentPair() :
  m_startSegment(nullptr),
  m_endSegment(nullptr)
{

}



CDCAxialSegmentPair::CDCAxialSegmentPair(const CDCAxialRecoSegment2D* startSegment,
                                         const CDCAxialRecoSegment2D* endSegment) :
  m_startSegment(startSegment), m_endSegment(endSegment)
{
  if (not startSegment) B2ERROR("CDCAxialSegmentPair initialized with nullptr as start segment");
  if (not endSegment) B2ERROR("CDCAxialSegmentPair initialized with nullptr as end segment");
}



CDCAxialSegmentPair::CDCAxialSegmentPair(const CDCAxialRecoSegment2D* startSegment,
                                         const CDCAxialRecoSegment2D* endSegment,
                                         const CDCTrajectory2D& trajectory2D) :
  m_startSegment(startSegment),
  m_endSegment(endSegment),
  m_trajectory2D(trajectory2D)
{

  if (not startSegment) B2ERROR("CDCAxialSegmentPair initialized with nullptr as start segment");
  if (not endSegment) B2ERROR("CDCAxialSegmentPair initialized with nullptr as end segment");

}
