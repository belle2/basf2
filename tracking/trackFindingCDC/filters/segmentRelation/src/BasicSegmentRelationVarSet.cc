/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentRelation/BasicSegmentRelationVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

BasicSegmentRelationVarSet::BasicSegmentRelationVarSet()
  : Super()
{
}


bool BasicSegmentRelationVarSet::extract(const Relation<const CDCRecoSegment2D>* ptrSegmentRelation)
{
  bool extracted = extractNested(ptrSegmentRelation);
  if (not extracted or not ptrSegmentRelation) return false;

  const Relation<const CDCRecoSegment2D>& segmentPair = *ptrSegmentRelation;

  const CDCRecoSegment2D* ptrFromSegment = segmentPair.getFrom();
  const CDCRecoSegment2D* ptrToSegment = segmentPair.getTo();

  const CDCRecoSegment2D& fromSegment = *ptrFromSegment;
  const CDCRecoSegment2D& toSegment = *ptrToSegment;

  var<named("stereo_kind")>() = static_cast<float>(fromSegment.getStereoKind());
  var<named("sl_id")>() = fromSegment.getISuperLayer();

  var<named("from_size")>() = fromSegment.size();
  var<named("to_size")>() = toSegment.size();
  return true;
}
