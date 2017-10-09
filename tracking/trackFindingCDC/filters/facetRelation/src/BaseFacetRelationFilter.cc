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

#include <tracking/trackFindingCDC/filters/base/RelationFilter.icc.h>

#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>

#include <vector>
#include <algorithm>
#include <cassert>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::RelationFilter<const CDCFacet>;

BaseFacetRelationFilter::BaseFacetRelationFilter() = default;

BaseFacetRelationFilter::~BaseFacetRelationFilter() = default;

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
