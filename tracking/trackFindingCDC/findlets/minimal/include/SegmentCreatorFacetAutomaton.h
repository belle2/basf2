/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/ca/MultipassCellularPathFinder.h>

#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <vector>
#include <iterator>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Findlet that generates segments within clusters based on a cellular automaton on triples of hits
    class SegmentCreatorFacetAutomaton :
      public Findlet<const CDCFacet,
      const WeightedRelation<const CDCFacet>,
      CDCRecoSegment2D> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCFacet,
            const WeightedRelation<const CDCFacet>,
            CDCRecoSegment2D>;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Constructs segments by extraction of facet paths in a cellular automaton.";
      }

      /// Main function of the segment finding by the cellular automaton.
      virtual void apply(const std::vector<CDCFacet>& inputFacets,
                         const std::vector<WeightedRelation<const CDCFacet> >& inputFacetRelations,
                         std::vector<CDCRecoSegment2D>& outputSegments) override final
      {
        auto getICluster = [](const CDCFacet & facet) -> int { return facet.getICluster();};
        std::vector<ConstVectorRange<CDCFacet> > facetsByICluster =
          adjacent_groupby(inputFacets.begin(), inputFacets.end(), getICluster);

        for (const ConstVectorRange<CDCFacet>& facetsInCluster : facetsByICluster) {
          if (facetsInCluster.empty()) continue;

          B2ASSERT("Expect the facets to be sorted",
                   std::is_sorted(std::begin(facetsInCluster), std::end(facetsInCluster)));

          // Create the facet neighborhood
          B2DEBUG(100, "Creating the CDCFacet neighborhood");

          // Cut out the chunk of relevant facet relations
          const CDCFacet& firstFacet = facetsInCluster.front();
          const CDCFacet& lastFacet = facetsInCluster.back();
          auto beginFacetRelationInSuperCluster = std::lower_bound(inputFacetRelations.begin(),
                                                                   inputFacetRelations.end(),
                                                                   &firstFacet);
          auto endFacetRelationInSuperCluster = std::upper_bound(inputFacetRelations.begin(),
                                                                 inputFacetRelations.end(),
                                                                 &lastFacet);

          WeightedNeighborhood<const CDCFacet> facetNeighborhood(beginFacetRelationInSuperCluster,
                                                                 endFacetRelationInSuperCluster);

          B2DEBUG(100, "  Created " << facetNeighborhood.size()  << " FacetsNeighborhoods");

          if (facetNeighborhood.size() == 0) {
            continue; // No neighborhood generated. Next cluster.
          }

          // Apply the cellular automaton in a multipass manner
          m_facetPaths.clear();
          m_cellularPathFinder.apply(facetsInCluster, facetNeighborhood, m_facetPaths);

          outputSegments.reserve(outputSegments.size() + m_facetPaths.size());
          for (const std::vector<const CDCFacet*>& facetPath : m_facetPaths) {
            outputSegments.push_back(CDCRecoSegment2D::condense(facetPath));
          }

          B2DEBUG(100, "  Created " << outputSegments.size()  << " selected CDCRecoSegment2Ds");
        } // end cluster loop
      }


    private: // cellular automaton
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<const CDCFacet> m_cellularPathFinder;

    private: // object pools
      /// Memory for the facet paths generated from the graph.
      std::vector< std::vector<const CDCFacet*> > m_facetPaths;

    };

  }
}
