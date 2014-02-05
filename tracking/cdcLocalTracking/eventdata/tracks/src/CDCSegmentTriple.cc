/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCSegmentTriple.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCSegmentTriple)



CDCSegmentTriple::CDCSegmentTriple() :
  CDCAxialAxialSegmentPair(),
  m_middleSegment(nullptr)
{;}



CDCSegmentTriple::CDCSegmentTriple(const CDCAxialRecoSegment2D* startSegment, const CDCAxialRecoSegment2D* endSegment) :
  CDCAxialAxialSegmentPair(startSegment, endSegment),
  m_middleSegment(nullptr)
{;}



CDCSegmentTriple::CDCSegmentTriple(
  const CDCAxialRecoSegment2D* startSegment,
  const CDCStereoRecoSegment2D* middleSegment,
  const CDCAxialRecoSegment2D* endSegment
) :
  CDCAxialAxialSegmentPair(startSegment, endSegment),
  m_middleSegment(middleSegment)
{
  if (not startSegment) B2ERROR("CDCSegmentTriple initialized with nullptr as start segment");
  if (not middleSegment) B2ERROR("CDCSegmentTriple initialized with nullptr as middle segment");
  if (not endSegment) B2ERROR("CDCSegmentTriple initialized with nullptr as end segment");
}



CDCSegmentTriple::CDCSegmentTriple(
  const CDCAxialRecoSegment2D* startSegment,
  const CDCStereoRecoSegment2D* middleSegment,
  const CDCAxialRecoSegment2D* endSegment,
  const CDCTrajectory2D& trajectory2D,
  const CDCTrajectorySZ& trajectorySZ
) :
  CDCAxialAxialSegmentPair(startSegment, endSegment, trajectory2D),
  m_middleSegment(middleSegment),
  m_trajectorySZ(trajectorySZ)
{
  if (not startSegment) B2ERROR("CDCSegmentTriple initialized with nullptr as start segment");
  if (not middleSegment) B2ERROR("CDCSegmentTriple initialized with nullptr as middle segment");
  if (not endSegment) B2ERROR("CDCSegmentTriple initialized with nullptr as end segment");
}



CDCSegmentTriple::~CDCSegmentTriple()
{

}



