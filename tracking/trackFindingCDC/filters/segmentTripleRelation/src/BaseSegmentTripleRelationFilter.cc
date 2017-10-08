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

#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>

#include <algorithm>
#include <cassert>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::Filter<Relation<const CDCSegmentTriple> >;

std::vector<const CDCSegmentTriple*> BaseSegmentTripleRelationFilter::getPossibleNeighbors(
  const CDCSegmentTriple* segmentTriple,
  const std::vector<const CDCSegmentTriple*>::const_iterator& itBegin,
  const std::vector<const CDCSegmentTriple*>::const_iterator& itEnd) const
{
  assert(std::is_sorted(itBegin, itEnd, LessOf<Deref>()) &&
         "Expected segment triples to be sorted");
  const CDCSegment2D* endSegment = segmentTriple->getEndSegment();

  ConstVectorRange<const CDCSegmentTriple*> neighbors{
    std::equal_range(itBegin, itEnd, &endSegment, LessOf<Deref>())};
  return {neighbors.begin(), neighbors.end()};
}

Weight BaseSegmentTripleRelationFilter::operator()(const CDCSegmentTriple& from
                                                   __attribute__((unused)),
                                                   const CDCSegmentTriple& to
                                                   __attribute__((unused)))
{
  return 1;
}

Weight BaseSegmentTripleRelationFilter::operator()(const Relation<const CDCSegmentTriple>& relation)
{
  const CDCSegmentTriple* ptrFrom(relation.first);
  const CDCSegmentTriple* ptrTo(relation.second);
  if ((ptrFrom == nullptr) or (ptrTo == nullptr)) return NAN;
  return this->operator()(*ptrFrom, *ptrTo);
}
