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

using namespace Belle2;
using namespace TrackFindingCDC;

Weight AllSegmentTripleFilter::operator()(const CDCSegmentTriple& segmentTriple)
{
  const CDCAxialRecoSegment2D* ptrStartSegment = segmentTriple.getStartSegment();
  const CDCStereoRecoSegment2D* ptrMiddleSegment = segmentTriple.getMiddleSegment();
  const CDCAxialRecoSegment2D* ptrEndSegment = segmentTriple.getEndSegment();

  assert(ptrStartSegment);
  assert(ptrMiddleSegment);
  assert(ptrEndSegment);

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& middleSegment = *ptrMiddleSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  Weight result = startSegment.size() + middleSegment.size() + endSegment.size();

  return result;
}
