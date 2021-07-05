/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPairRelation/BasicSegmentPairRelationVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool BasicSegmentPairRelationVarSet::extract(const Relation<const CDCSegmentPair>* ptrSegmentPairRelation)
{
  if (not ptrSegmentPairRelation) return false;

  const CDCSegmentPair* fromSegmentPair = ptrSegmentPairRelation->first;
  const CDCSegmentPair* toSegmentPair   = ptrSegmentPairRelation->second;
  const CDCSegment2D* middleSegment = fromSegmentPair->getToSegment();

  var<named("middle_sl_id")>() = middleSegment->getISuperLayer();
  var<named("middle_is_axial")>() = middleSegment->isAxial();
  var<named("from_size")>() = fromSegmentPair->size();
  var<named("to_size")>() = toSegmentPair->size();
  var<named("middle_size")>() = middleSegment->size();

  return true;
}
