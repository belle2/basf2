/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

std::vector<const CDCFacet*> BaseFacetRelationFilter::getPossibleTos(
  const CDCFacet* from,
  const std::vector<const CDCFacet*>& facets) const
{
  const CDCRLWireHitPair& rearRLWireHitPair = from->getRearRLWireHitPair();

  ConstVectorRange<const CDCFacet*> neighbors{
    std::equal_range(facets.begin(), facets.end(), &rearRLWireHitPair, LessOf<Deref>())};
  return {neighbors.begin(), neighbors.end()};
}
