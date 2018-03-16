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

#include <tracking/trackFindingCDC/filters/base/RelationFilter.icc.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>

#include <tracking/trackFindingCDC/utilities/Functional.h>

#include <vector>
#include <algorithm>
#include <cassert>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::RelationFilter<const CDCSegment2D>;

BaseSegmentRelationFilter::BaseSegmentRelationFilter() = default;

BaseSegmentRelationFilter::~BaseSegmentRelationFilter() = default;

std::vector<const CDCSegment2D*> BaseSegmentRelationFilter::getPossibleTos(
  const CDCSegment2D* from,
  const std::vector<const CDCSegment2D*>& segments) const
{
  assert(std::is_sorted(segments.begin(), segments.end(), LessOf<Deref>()) &&
         "Expected segments to be sorted");

  ConstVectorRange<const CDCSegment2D*> tos{
    std::equal_range(segments.begin(), segments.end(), from, LessOf<Deref>())};

  return {tos.begin(), tos.end()};
}

Weight BaseSegmentRelationFilter::operator()(const Relation<const CDCSegment2D>& relation)
{
  const CDCSegment2D* from = relation.getFrom();
  const CDCSegment2D* to = relation.getTo();
  if (from == to) return NAN; // Prevent relation to same.
  if ((from == nullptr) or (to == nullptr)) return NAN;

  // Make an overlap check to prevent aliases and reverse segments to be linked
  std::vector<const CDCWireHit*> fromWireHits;
  fromWireHits.reserve(from->size());
  for (const CDCRecoHit2D& recoHit2D : *from) {
    fromWireHits.push_back(&recoHit2D.getWireHit());
  }
  std::sort(fromWireHits.begin(), fromWireHits.end());
  int nOverlap = 0;
  for (const CDCRecoHit2D& recoHit2D : *to) {
    if (std::binary_search(fromWireHits.begin(), fromWireHits.end(), &recoHit2D.getWireHit())) {
      ++nOverlap;
    }
  }

  if (1.0 * nOverlap / from->size() > 0.8 or 1.0 * nOverlap / from->size() > 0.8) {
    return NAN;
  }

  return this->operator()(*from, *to);
}
