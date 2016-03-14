/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/AllSegmentTripleFilter.h"

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CellWeight AllSegmentTripleFilter::operator()(const CDCSegmentTriple& segmentTriple)
{
  const CDCAxialRecoSegment2D* ptrStartSegment = segmentTriple.getStart();
  const CDCStereoRecoSegment2D* ptrMiddleSegment = segmentTriple.getMiddle();
  const CDCAxialRecoSegment2D* ptrEndSegment = segmentTriple.getEnd();

  assert(ptrStartSegment);
  assert(ptrMiddleSegment);
  assert(ptrEndSegment);

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& middleSegment = *ptrMiddleSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  CellWeight result = startSegment.size() + middleSegment.size() + endSegment.size();

  return result;
}
