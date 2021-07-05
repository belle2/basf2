/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/axialSegmentPair/AllAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight AllAxialSegmentPairFilter::operator()(const CDCAxialSegmentPair& axialSegmentPair)
{
  const CDCAxialSegment2D* ptrStartSegment = axialSegmentPair.getStartSegment();
  const CDCAxialSegment2D* ptrEndSegment = axialSegmentPair.getEndSegment();

  assert(ptrStartSegment);
  assert(ptrEndSegment);

  const CDCAxialSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialSegment2D& endSegment = *ptrEndSegment;

  return startSegment.size() + endSegment.size();
}
