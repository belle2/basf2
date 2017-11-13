/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPairRelation/BaseSegmentPairRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>

#include <tracking/trackFindingCDC/filters/base/RelationFilter.icc.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>

#include <algorithm>
#include <cassert>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::RelationFilter<const CDCSegmentPair>;

BaseSegmentPairRelationFilter::BaseSegmentPairRelationFilter() = default;

BaseSegmentPairRelationFilter::~BaseSegmentPairRelationFilter() = default;

std::vector<const CDCSegmentPair*> BaseSegmentPairRelationFilter::getPossibleTos(
  const CDCSegmentPair* from,
  const std::vector<const CDCSegmentPair*>& segmentPairs) const
{
  assert(std::is_sorted(segmentPairs.begin(), segmentPairs.end(), LessOf<Deref>()) &&
         "Expected segment pairs to be sorted");

  const CDCSegment2D* toSegment = from->getToSegment();

  ConstVectorRange<const CDCSegmentPair*> neighbors{
    std::equal_range(segmentPairs.begin(), segmentPairs.end(), &toSegment, LessOf<Deref>())};
  return {neighbors.begin(), neighbors.end()};
}
