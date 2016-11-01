/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPairRelation/MVARealisticSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MVARealisticSegmentPairRelationFilter::MVARealisticSegmentPairRelationFilter()
  : Super(makeUnique<VarSet>(),
          "trackfindingcdc_RealisticSegmentPairRelationFilter",
          0.02)
{
}

Weight MVARealisticSegmentPairRelationFilter::operator()(const Relation<const CDCSegmentPair>& relation)
{
  double mvaResult = Super::operator()(relation);

  const CDCSegmentPair& fromSegmentPair = *relation.getFrom();
  const CDCSegmentPair& toSegmentPair = *relation.getTo();

  size_t fromOverlapSize = fromSegmentPair.getToSegment()->size();
  size_t fromSize = fromOverlapSize + fromSegmentPair.getFromSegment()->size();
  Weight fromWeight = fromSegmentPair.getAutomatonCell().getCellWeight();

  size_t toOverlapSize = toSegmentPair.getFromSegment()->size();
  size_t toSize = toOverlapSize + toSegmentPair.getToSegment()->size();
  Weight toWeight = toSegmentPair.getAutomatonCell().getCellWeight();

  // if (std::isnan(mvaResult)) {
  //   return NAN;
  // } else {
  //   return (fromWeight * fromOverlapSize / fromSize + toWeight * toOverlapSize / toSize) / 2.0;
  // }
  // return mvaResult;
  return mvaResult * (fromWeight * fromOverlapSize / fromSize + toWeight * toOverlapSize / toSize) / 2.0;
}
