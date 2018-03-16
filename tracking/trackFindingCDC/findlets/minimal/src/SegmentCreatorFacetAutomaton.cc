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

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCFacetSegment.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRLWireHitSegment.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <algorithm>
#include <functional>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  void setRLWireHit(CDCRLWireHitTriple& rlWireHitTriple, int iRLWireHit, const CDCRLWireHit& rlWireHit)
  {
    if (iRLWireHit == 0) {
      rlWireHitTriple.setStartRLWireHit(rlWireHit);
    } else if (iRLWireHit == 1) {
      rlWireHitTriple.setMiddleRLWireHit(rlWireHit);
    } else if (iRLWireHit == 2) {
      rlWireHitTriple.setEndRLWireHit(rlWireHit);
    } else {
      B2ASSERT("", false);
    }
  }
}

std::string SegmentCreatorFacetAutomaton::getDescription()
{
  return "Constructs segments by extraction of facet paths in a cellular automaton.";
}

void SegmentCreatorFacetAutomaton::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "searchReversed"),
                                m_param_searchReversed,
                                "Switch to construct the reversed segment if it is available in the facet graph as well.",
                                m_param_searchReversed);

  moduleParamList->addParameter(prefixed(prefix, "searchAlias"),
                                m_param_searchAlias,
                                "Switch to construct the alias segment if it is available in the facet graph as well.",
                                m_param_searchAlias);

  moduleParamList->addParameter(prefixed(prefix, "relaxSingleLayerSearch"),
                                m_param_relaxSingleLayerSearch,
                                "Switch to relax the alias and reverse search for segments contained in a single layer.",
                                m_param_relaxSingleLayerSearch);

  moduleParamList->addParameter(prefixed(prefix, "allSingleAliases"),
                                m_param_allSingleAliases,
                                "Switch to activate the write out of all available orientations of single facet segments.",
                                m_param_allSingleAliases);
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

    // Obtain the facets as pointers
    std::vector<const CDCFacet*> facetPtrsInCluster = as_pointers<const CDCFacet>(facetsInCluster);

    // Cut out the chunk of relevant facet relations
    const CDCFacet& firstFacet = facetsInCluster.front();
    auto beginFacetRelationInCluster =
      std::lower_bound(inputFacetRelations.begin(), inputFacetRelations.end(), &firstFacet);

    const CDCFacet& lastFacet = facetsInCluster.back();
    auto endFacetRelationInCluster =
      std::upper_bound(inputFacetRelations.begin(), inputFacetRelations.end(), &lastFacet);

    const int iCluster = firstFacet.getICluster();

    std::vector<WeightedRelation<const CDCFacet>>
                                               facetRelationsInCluster(beginFacetRelationInCluster,
                                                   endFacetRelationInCluster);

    // Apply the cellular automaton in a multipass manner
    m_facetPaths.clear();
    m_cellularPathFinder.apply(facetPtrsInCluster, facetRelationsInCluster, m_facetPaths);

    // Helper function to check if a given reverse or alias segment is
    // also present in the graph of facets. Used in the search for
    // aliasing segments.
    auto getFacetPath = [&facetsInCluster,
                         &facetRelationsInCluster,
    &iCluster](const CDCSegment2D & segment, bool checkRelations = true) {
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
        if (not facetPath.empty() and checkRelations) {
          const CDCFacet* fromFacet = facetPath.back();
          auto relationsFromFacet = std::equal_range(facetRelationsInCluster.begin(),
                                                     facetRelationsInCluster.end(),
                                                     fromFacet);
          if (std::count_if(relationsFromFacet.first, relationsFromFacet.second, Second() == facet) == 0) break;
        }
        facetPath.push_back(facet);
      }
      return facetPath;
    };

    // Reserve enough space to prevent reallocation and invalidated references
    size_t additionalSpace = m_facetPaths.size();
    if (m_param_searchReversed) additionalSpace *= 2;
    if (m_param_searchAlias) additionalSpace *= 2;
    outputSegments.reserve(outputSegments.size() + additionalSpace);

    for (const std::vector<const CDCFacet*>& facetPath : m_facetPaths) {
      // If path is only a single facet long - forward all viable orientations if requested
      if (m_param_allSingleAliases and facetPath.size() == 1) {
        const CDCFacet& originalSingleFacet = *facetPath.front();

        int nSingleFacets = 0;

        // Helper object to construct other single facet paths
        std::vector<const CDCFacet*> singleFacetPath;
        singleFacetPath.reserve(1);

        std::array<int, 3> permIndices{0, 1, 2};
        CDCRLWireHitTriple rlWireHitTriple = originalSingleFacet;

        for (int iPerm = 0; iPerm < 6; ++iPerm) {
          setRLWireHit(rlWireHitTriple, permIndices[0], originalSingleFacet.getStartRLWireHit());
          setRLWireHit(rlWireHitTriple, permIndices[1], originalSingleFacet.getMiddleRLWireHit());
          setRLWireHit(rlWireHitTriple, permIndices[2], originalSingleFacet.getEndRLWireHit());
          std::next_permutation(permIndices.begin(), permIndices.end()); // Prepare for next round

          for (ERightLeft startRLInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
            rlWireHitTriple.setStartRLInfo(startRLInfo);
            for (ERightLeft middleRLInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
              rlWireHitTriple.setMiddleRLInfo(middleRLInfo);
              for (ERightLeft endRLInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
                rlWireHitTriple.setEndRLInfo(endRLInfo);

                auto itFacet = std::lower_bound(facetsInCluster.begin(),
                                                facetsInCluster.end(),
                                                rlWireHitTriple);

                if (itFacet == facetsInCluster.end())continue;
                if (not(*itFacet == rlWireHitTriple)) continue;

                const CDCFacet* singleFacet = &*itFacet;
                singleFacetPath.clear();
                singleFacetPath.push_back(singleFacet);
                outputSegments.push_back(CDCSegment2D::condense(singleFacetPath));
                outputSegments.back()->setReverseFlag();
                outputSegments.back()->setAliasFlag();
                ++nSingleFacets;
              }
            }
          }
        }
        B2ASSERT("At least one single facet added", nSingleFacets > 0);

        // Skip the reset of the alias searches
        continue;
      }

      outputSegments.reserve(outputSegments.size() + 4);
      outputSegments.push_back(CDCSegment2D::condense(facetPath));
      const CDCSegment2D* segment = &outputSegments.back();


      // Check for the special situation where the segment is confined to one layer
      // Relax the alias search a bit to better capture the situation
      bool checkRelations = true;
      if (m_param_relaxSingleLayerSearch) {
        auto differentILayer = [](const CDCRecoHit2D & lhs, const CDCRecoHit2D & rhs) {
          return lhs.getWire().getILayer() != rhs.getWire().getILayer();
        };
        auto itLayerSwitch = std::adjacent_find(segment->begin(), segment->end(), differentILayer);
        const bool onlyOneLayer = itLayerSwitch == segment->end();
        checkRelations = not onlyOneLayer;
      }

      const CDCSegment2D* reverseSegment = nullptr;
      if (m_param_searchReversed) {
        std::vector<const CDCFacet*> reverseFacetPath = getFacetPath(segment->reversed(), checkRelations);
        if (reverseFacetPath.size() == facetPath.size()) {
          B2DEBUG(100, "Successful constructed REVERSE");
          outputSegments.push_back(CDCSegment2D::condense(reverseFacetPath));
          reverseSegment = &outputSegments.back();

          (*segment)->setReverseFlag(true);
          (*reverseSegment)->setReverseFlag(true);
        }
      }

      if (not m_param_searchAlias) continue;

      // Search for aliasing segment in the facet graph
      int nRLSwitches = segment->getNRLSwitches();
      if (nRLSwitches > 2) continue; // Segment is stable against aliases

      const CDCSegment2D* aliasSegment = nullptr;
      std::vector<const CDCFacet*> aliasFacetPath = getFacetPath(segment->getAlias(), checkRelations);
      if (aliasFacetPath.size() == facetPath.size()) {
        B2DEBUG(100, "Successful constructed alias");
        outputSegments.push_back(CDCSegment2D::condense(aliasFacetPath));
        aliasSegment = &outputSegments.back();

        (*segment)->setAliasFlag(true);
        (*aliasSegment)->setAliasFlag(true);
      }

      const CDCSegment2D* reverseAliasSegment = nullptr;
      if (m_param_searchReversed) {
        std::vector<const CDCFacet*> reverseAliasFacetPath =
          getFacetPath(segment->reversed().getAlias(), checkRelations);
        if (reverseAliasFacetPath.size() == facetPath.size()) {
          B2DEBUG(100, "Successful constructed REVERSE alias");
          outputSegments.push_back(CDCSegment2D::condense(reverseAliasFacetPath));
          reverseAliasSegment = &outputSegments.back();
          if (aliasSegment != nullptr) {
            (*aliasSegment)->setReverseFlag(true);
            (*reverseAliasSegment)->setReverseFlag(true);
          }
        }
      }

      if (reverseSegment != nullptr and reverseAliasSegment != nullptr) {
        (*reverseSegment)->setAliasFlag(true);
        (*reverseAliasSegment)->setAliasFlag(true);
      }
    }
  }
}
