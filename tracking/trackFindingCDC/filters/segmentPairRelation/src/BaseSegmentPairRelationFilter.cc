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

std::vector<const CDCSegmentPair*> BaseSegmentPairRelationFilter::getPossibleNeighbors(
  const CDCSegmentPair* segmentPair,
  const std::vector<const CDCSegmentPair*>::const_iterator& itBegin,
  const std::vector<const CDCSegmentPair*>::const_iterator& itEnd) const
{
  assert(std::is_sorted(itBegin, itEnd, LessOf<Deref>()) && "Expected segment pairs to be sorted");

  const CDCSegment2D* toSegment = segmentPair->getToSegment();

  ConstVectorRange<const CDCSegmentPair*> neighbors{
    std::equal_range(itBegin, itEnd, &toSegment, LessOf<Deref>())};
  return {neighbors.begin(), neighbors.end()};
}
