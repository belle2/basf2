/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/AllSegmentPairFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight AllSegmentPairFilter::operator()(const CDCSegmentPair& segmentPair)
{
  const CDCSegment2D* ptrStartSegment = segmentPair.getFromSegment();
  const CDCSegment2D* ptrEndSegment = segmentPair.getToSegment();

  assert(ptrStartSegment);
  assert(ptrEndSegment);

  const CDCSegment2D& startSegment = *ptrStartSegment;
  const CDCSegment2D& endSegment = *ptrEndSegment;
  return startSegment.size() + endSegment.size();
}
