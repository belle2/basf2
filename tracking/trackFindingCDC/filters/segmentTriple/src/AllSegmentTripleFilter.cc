/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTriple/AllSegmentTripleFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight AllSegmentTripleFilter::operator()(const CDCSegmentTriple& segmentTriple)
{
  const CDCAxialSegment2D* ptrStartSegment = segmentTriple.getStartSegment();
  const CDCStereoSegment2D* ptrMiddleSegment = segmentTriple.getMiddleSegment();
  const CDCAxialSegment2D* ptrEndSegment = segmentTriple.getEndSegment();

  assert(ptrStartSegment);
  assert(ptrMiddleSegment);
  assert(ptrEndSegment);

  const CDCAxialSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialSegment2D& middleSegment = *ptrMiddleSegment;
  const CDCAxialSegment2D& endSegment = *ptrEndSegment;

  Weight result = startSegment.size() + middleSegment.size() + endSegment.size();

  return result;
}
