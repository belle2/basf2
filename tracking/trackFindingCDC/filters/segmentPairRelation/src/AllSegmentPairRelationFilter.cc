/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPairRelation/AllSegmentPairRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight AllSegmentPairRelationFilter::operator()(const CDCSegmentPair& fromSegmentPair,
                                                const CDCSegmentPair& toSegmentPair)
{
  size_t fromOverlapSize = fromSegmentPair.getToSegment()->size();
  size_t fromSize = fromOverlapSize + fromSegmentPair.getFromSegment()->size();
  Weight fromWeight = fromSegmentPair.getAutomatonCell().getCellWeight();

  size_t toOverlapSize = toSegmentPair.getFromSegment()->size();
  size_t toSize = toOverlapSize + toSegmentPair.getToSegment()->size();
  Weight toWeight = toSegmentPair.getAutomatonCell().getCellWeight();

  return (fromWeight * fromOverlapSize / fromSize + toWeight * toOverlapSize / toSize) / 2.0;
}
