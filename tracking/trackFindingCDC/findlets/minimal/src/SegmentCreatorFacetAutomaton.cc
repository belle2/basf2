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

#include <tracking/trackFindingCDC/filters/facet/MCFacetFilter.h>
#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <framework/core/ModuleParamList.h>

#include <algorithm>
#include <functional>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string SegmentCreatorFacetAutomaton::getDescription()
{
  return "Constructs segments by extraction of facet paths in a cellular automaton.";
}

void SegmentCreatorFacetAutomaton::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "constructFacetAliases"),
                                m_param_constructFacetAliases,
                                "Switch to construct the alias segment if it is available in the facet graph as well.",
                                m_param_constructFacetAliases);
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

    const int iCluster = firstFacet.getICluster();

    WeightedNeighborhood<const CDCFacet> facetNeighborhood(beginFacetRelationInSuperCluster,
                                                           endFacetRelationInSuperCluster);

    if (facetNeighborhood.size() == 0) {
      continue; // No neighborhood generated. Next cluster.
    }

    // Apply the cellular automaton in a multipass manner
    m_facetPaths.clear();
    m_cellularPathFinder.apply(facetsInCluster, facetNeighborhood, m_facetPaths);

    // Helper function to check if a segment is also present in the graph of facets
    // Used in the search for aliasing segments
    auto getFacetPath = [&facetsInCluster, &facetNeighborhood, &iCluster](CDCSegment2D & segment) {
      CDCRLWireHitSegment rlWireHitSegment = segment.getRLWireHitSegment();
      CDCFacetSegment aliasFacetSegment = CDCFacetSegment::create(rlWireHitSegment);
      std::vector<const CDCFacet*> facetPath;
      for (CDCRLWireHitTriple& rlWireHitTriple : aliasFacetSegment) {
        // Do not forget to set the cluster id as it is a sorting criterion
        rlWireHitTriple.setICluster(iCluster);

        // Check whether the facet is a node in the graph
        auto itFacet = std::lower_bound(facetsInCluster.begin(), facetsInCluster.end(), rlWireHitTriple);
        if (itFacet == facetsInCluster.end()) break;
        if (not(*itFacet == rlWireHitTriple)) break;
        const CDCFacet* facet = &*itFacet;

        // Check whether there is a relation to this new facet
        if (not facetPath.empty()) {
          const CDCFacet* fromFacet = facetPath.back();
          auto relationsFromFacet =
            std::equal_range(facetNeighborhood.begin(), facetNeighborhood.end(), fromFacet);
          if (std::count_if(relationsFromFacet.first, relationsFromFacet.second, Second() == facet) == 0) break;
        }
        facetPath.push_back(facet);
      }
      return facetPath;
    };

    outputSegments.reserve(outputSegments.size() + m_facetPaths.size());
    for (const std::vector<const CDCFacet*>& facetPath : m_facetPaths) {
      outputSegments.push_back(CDCSegment2D::condense(facetPath));
      const CDCSegment2D& segment =  outputSegments.back();

      if (not m_param_constructFacetAliases) continue;

      // Search for aliasing segment in the facet graph
      int nRLSwitches = segment.getNRLSwitches();
      if (nRLSwitches > 2) continue; // Segment is stable against aliases

      CDCSegment2D aliasSegment = segment.getAlias();
      std::vector<const CDCFacet*> aliasFacetPath = getFacetPath(aliasSegment);
      if (aliasFacetPath.size() == facetPath.size()) {
        B2DEBUG(100, "Successful constructed alias");
        outputSegments.push_back(CDCSegment2D::condense(aliasFacetPath));
      } else {
        aliasSegment.reverse();
        std::vector<const CDCFacet*> reverseAliasFacetPath = getFacetPath(aliasSegment);
        if (reverseAliasFacetPath.size() == facetPath.size()) {
          B2DEBUG(100, "Successful constructed REVERSE alias");
          outputSegments.push_back(CDCSegment2D::condense(reverseAliasFacetPath));
        }
      }
    }
  }
}
