/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
