/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facetRelation/BaseFacetRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>

#include <algorithm>
#include <cassert>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::Filter<Relation<const CDCFacet> >;

std::vector<const CDCFacet*> BaseFacetRelationFilter::getPossibleNeighbors(
  const CDCFacet* facet,
  const std::vector<const CDCFacet*>::const_iterator& itBegin,
  const std::vector<const CDCFacet*>::const_iterator& itEnd) const
{
  // Expensive assert - but true one the less
  // assert(std::is_sorted(itBegin, itEnd, LessOf<Deref>()) && "Expected facets to be sorted");

  const CDCRLWireHitPair& rearRLWireHitPair = facet->getRearRLWireHitPair();
  ConstVectorRange<const CDCFacet*> neighbors{
    std::equal_range(itBegin, itEnd, &rearRLWireHitPair, LessOf<Deref>())};
  return {neighbors.begin(), neighbors.end()};
}

Weight BaseFacetRelationFilter::operator()(const CDCFacet& from __attribute__((unused)),
                                           const CDCFacet& to __attribute__((unused)))
{
  return 1;
}

Weight BaseFacetRelationFilter::operator()(const Relation<const CDCFacet>& relation)
{
  const CDCFacet* ptrFrom(relation.getFrom());
  const CDCFacet* ptrTo(relation.getTo());
  if ((ptrFrom == nullptr) or (ptrTo == nullptr)) return NAN;
  return this->operator()(*ptrFrom, *ptrTo);
}
