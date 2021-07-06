/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
