/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
