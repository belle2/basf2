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
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Findlet that generates segments within clusters based on a cellular automaton on triples of hits
    template<class AFacetRelationFilter>
    class SegmentCreatorFacetAutomaton :
      public Findlet<const CDCFacet, CDCRecoSegment2D> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCFacet, CDCRecoSegment2D>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      SegmentCreatorFacetAutomaton()
      {
        addProcessingSignalListener(&m_facetRelationFilter);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Constructs segments by extraction of facet paths in a cellular automaton.";
      }

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList,
                            const std::string& prefix = "") override final
      {
        m_facetRelationFilter.exposeParameters(moduleParamList, prefix);
      }

      /// Main function of the segment finding by the cellular automaton.
      virtual void apply(const std::vector<CDCFacet>& inputFacets,
                         std::vector<CDCRecoSegment2D>& outputSegments) override final;

    private:
      /// Reference to the relation filter to be used to construct the facet network.
      AFacetRelationFilter m_facetRelationFilter;

    private:
      //cellular automaton
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<const CDCFacet> m_cellularPathFinder;

    private: // object pools
      /// Memory for the facet neighborhood.
      WeightedNeighborhood<const CDCFacet> m_facetsNeighborhood;

      /// Memory for the facet paths generated from the graph.
      std::vector< std::vector<const CDCFacet*> > m_facetPaths;

    }; // end class SegmentCreator


    template<class AFacetRelationFilter>
    void SegmentCreatorFacetAutomaton<AFacetRelationFilter>::
    apply(const std::vector<CDCFacet>& inputFacets,
          std::vector<CDCRecoSegment2D>& outputSegments)
    {
      auto getICluster = [](const CDCFacet & facet) -> int { return facet.getICluster();};
      std::vector<ConstVectorRange<CDCFacet> > facetsByICluster =
        adjacent_groupby(inputFacets.begin(), inputFacets.end(), getICluster);

      for (const ConstVectorRange<CDCFacet>& facetsInCluster : facetsByICluster) {
        B2ASSERT("Expect the facets to be sorted",
                 std::is_sorted(std::begin(facetsInCluster), std::end(facetsInCluster)));

        // Create the facet neighborhood
        B2DEBUG(100, "Creating the CDCFacet neighborhood");
        m_facetsNeighborhood.clear();
        m_facetsNeighborhood.appendUsing(m_facetRelationFilter, facetsInCluster);
        B2DEBUG(100, "  Created " << m_facetsNeighborhood.size()  << " FacetsNeighborhoods");

        if (m_facetsNeighborhood.size() == 0) {
          continue; // No neighborhood generated. Next cluster.
        }

        // Apply the cellular automaton in a multipass manner
        m_facetPaths.clear();
        m_cellularPathFinder.apply(facetsInCluster, m_facetsNeighborhood, m_facetPaths);

        outputSegments.reserve(outputSegments.size() + m_facetPaths.size());
        for (const std::vector<const CDCFacet*>& facetPath : m_facetPaths) {
          outputSegments.push_back(CDCRecoSegment2D::condense(facetPath));
        }

        B2DEBUG(100, "  Created " << outputSegments.size()  << " selected CDCRecoSegment2Ds");
      } // end cluster loop
    }
  } //end namespace TrackFindingCDC
} //end namespace Belle2
