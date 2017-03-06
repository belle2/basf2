/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorFacetAutomaton.h>

#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <algorithm>
#include <functional>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string SegmentCreatorFacetAutomaton::getDescription()
{
  return "Constructs segments by extraction of facet paths in a cellular automaton.";
}

void SegmentCreatorFacetAutomaton::apply(
  const std::vector<CDCFacet>& inputFacets,
  const std::vector<WeightedRelation<const CDCFacet>>& inputFacetRelations,
  std::vector<CDCSegment2D>& outputSegments)
{
  std::vector<ConstVectorRange<CDCFacet>> facetsByICluster =
                                         adjacent_groupby(inputFacets.begin(), inputFacets.end(), std::mem_fn(&CDCFacet::getICluster));

  for (const ConstVectorRange<CDCFacet>& facetsInCluster : facetsByICluster) {
    if (facetsInCluster.empty()) continue;

    B2ASSERT("Expect the facets to be sorted",
             std::is_sorted(std::begin(facetsInCluster), std::end(facetsInCluster)));

    // Cut out the chunk of relevant facet relations
    const CDCFacet& firstFacet = facetsInCluster.front();
    auto beginFacetRelationInSuperCluster =
      std::lower_bound(inputFacetRelations.begin(), inputFacetRelations.end(), &firstFacet);

    const CDCFacet& lastFacet = facetsInCluster.back();
    auto endFacetRelationInSuperCluster =
      std::upper_bound(inputFacetRelations.begin(), inputFacetRelations.end(), &lastFacet);

    WeightedNeighborhood<const CDCFacet> facetNeighborhood(beginFacetRelationInSuperCluster,
                                                           endFacetRelationInSuperCluster);

    if (facetNeighborhood.size() == 0) {
      continue; // No neighborhood generated. Next cluster.
    }

    // Apply the cellular automaton in a multipass manner
    m_facetPaths.clear();
    m_cellularPathFinder.apply(facetsInCluster, facetNeighborhood, m_facetPaths);

    outputSegments.reserve(outputSegments.size() + m_facetPaths.size());
    for (const std::vector<const CDCFacet*>& facetPath : m_facetPaths) {
      outputSegments.push_back(CDCSegment2D::condense(facetPath));
    }
  }
}
