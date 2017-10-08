/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentRelation/BaseSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>

#include <tracking/trackFindingCDC/utilities/Functional.h>

#include <algorithm>
#include <cassert>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::Filter<Relation<const CDCSegment2D> >;

BaseSegmentRelationFilter::BaseSegmentRelationFilter() = default;

BaseSegmentRelationFilter::~BaseSegmentRelationFilter() = default;

std::vector<const CDCSegment2D*> BaseSegmentRelationFilter::getPossibleNeighbors(
  const CDCSegment2D* segment,
  const std::vector<const CDCSegment2D*>::const_iterator& itBegin,
  const std::vector<const CDCSegment2D*>::const_iterator& itEnd) const
{
  assert(std::is_sorted(itBegin, itEnd, LessOf<Deref>()) && "Expected segments to be sorted");
  ConstVectorRange<const CDCSegment2D*> neighbors{
    std::equal_range(itBegin, itEnd, segment, LessOf<Deref>())};
  return {neighbors.begin(), neighbors.end()};
}

Weight BaseSegmentRelationFilter::operator()(const CDCSegment2D& from __attribute__((unused)),
                                             const CDCSegment2D& to __attribute__((unused)))
{
  return 1;
}

Weight BaseSegmentRelationFilter::operator()(const Relation<const CDCSegment2D>& relation)
{
  const CDCSegment2D* ptrFrom(relation.first);
  const CDCSegment2D* ptrTo(relation.second);
  if (ptrFrom == ptrTo) return NAN; // Prevent relation to same.
  if ((ptrFrom == nullptr) or (ptrTo == nullptr)) return NAN;

  // Make an overlap check to prevent aliases and reverse segments to be linked
  std::vector<const CDCWireHit*> fromWireHits;
  fromWireHits.reserve(ptrFrom->size());
  for (const CDCRecoHit2D& recoHit2D : *ptrFrom) {
    fromWireHits.push_back(&recoHit2D.getWireHit());
  }
  std::sort(fromWireHits.begin(), fromWireHits.end());
  int nOverlap = 0;
  for (const CDCRecoHit2D& recoHit2D : *ptrTo) {
    if (std::binary_search(fromWireHits.begin(), fromWireHits.end(), &recoHit2D.getWireHit())) {
      ++nOverlap;
    }
  }

  if (1.0 * nOverlap / ptrFrom->size() > 0.8 or 1.0 * nOverlap / ptrFrom->size() > 0.8) {
    return NAN;
  }

  return operator()(*ptrFrom, *ptrTo);
}
