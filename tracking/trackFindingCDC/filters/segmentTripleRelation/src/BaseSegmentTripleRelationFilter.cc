/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTripleRelation/BaseSegmentTripleRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include <tracking/trackFindingCDC/filters/base/RelationFilter.icc.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>

#include <vector>
#include <algorithm>
#include <cassert>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::RelationFilter<const CDCSegmentTriple>;

BaseSegmentTripleRelationFilter::BaseSegmentTripleRelationFilter() = default;

BaseSegmentTripleRelationFilter::~BaseSegmentTripleRelationFilter() = default;

std::vector<const CDCSegmentTriple*> BaseSegmentTripleRelationFilter::getPossibleTos(
  const CDCSegmentTriple* from,
  const std::vector<const CDCSegmentTriple*>& segmentTriples) const
{
  assert(std::is_sorted(segmentTriples.begin(), segmentTriples.end(), LessOf<Deref>()) &&
         "Expected segment triples to be sorted");

  const CDCSegment2D* endSegment = from->getEndSegment();

  ConstVectorRange<const CDCSegmentTriple*> neighbors{
    std::equal_range(segmentTriples.begin(), segmentTriples.end(), &endSegment, LessOf<Deref>())};
  return {neighbors.begin(), neighbors.end()};
}
