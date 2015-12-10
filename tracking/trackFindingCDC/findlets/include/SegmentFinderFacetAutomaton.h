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
#include <tracking/trackFindingCDC/filters/wirehit_relation/WholeWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/ca/Clusterizer.h>
#include <tracking/trackFindingCDC/creators/FacetCreator.h>

#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>
#include <tracking/trackFindingCDC/filters/facet/RealisticFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet_relation/SimpleFacetRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCTangentSegment.h>

#include <tracking/trackFindingCDC/findlets/Findlet.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <vector>
#include <iterator>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Findlet that generates segments within clusters based on a cellular automaton on triples of hits
    template<class FacetFilter = BaseFacetFilter,
             class FacetRelationFilter = BaseFacetRelationFilter>
    class SegmentFinderFacetAutomaton :
      public Findlet<const CDCWireHitCluster, CDCRecoSegment2D> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHitCluster, CDCRecoSegment2D>;

    public:
      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList) override final
      {
        moduleParamList->addParameter("WriteFacets",
                                      m_param_writeFacets,
                                      "Switch if facets shall be written to the DataStore",
                                      m_param_writeFacets);

        moduleParamList->addParameter("FacetsStoreObjName",
                                      m_param_facetsStoreObjName,
                                      "Name of the output StoreObjPtr of the facets generated within this module.",
                                      m_param_facetsStoreObjName);

        moduleParamList->addParameter("WriteTangentSegments",
                                      m_param_writeTangentSegments,
                                      "Switch if tangent segments shall be written to the DataStore",
                                      m_param_writeTangentSegments);

        moduleParamList->addParameter("TangentSegmentsStoreObjName",
                                      m_param_tangentSegmentsStoreObjName,
                                      "Name of the output StoreObjPtr of the tangent segments generated within this module.",
                                      m_param_tangentSegmentsStoreObjName);

        getFacetFilter()->exposeParameters(moduleParamList);
        getFacetRelationFilter()->exposeParameters(moduleParamList);
      }

      /// Initialize the Module before event processing
      virtual void initialize() override final
      {
        Super::initialize();
        if (m_param_writeFacets) {
          StoreWrappedObjPtr< std::vector<CDCFacet> >::registerTransient(m_param_facetsStoreObjName);
        }

        if (m_param_writeTangentSegments) {
          StoreWrappedObjPtr< std::vector<CDCTangentSegment> >::registerTransient(m_param_tangentSegmentsStoreObjName);
        }

        if (m_ptrFacetFilter) {
          m_ptrFacetFilter->initialize();
        }

        if (m_ptrFacetRelationFilter) {
          m_ptrFacetRelationFilter->initialize();
        }
      }

      /// Start processing the current event
      virtual void beginEvent() override final
      {
        Super::beginEvent();

        /// Attain facet vector on the DataStore if needed.
        if (m_param_writeFacets) {
          StoreWrappedObjPtr< std::vector<CDCFacet> > storedFacets(m_param_facetsStoreObjName);
          storedFacets.create();
        }

        /// Attain tangent vector on the DataStore if needed.
        if (m_param_writeTangentSegments) {
          StoreWrappedObjPtr< std::vector<CDCTangentSegment> > storedTangentSegments(m_param_tangentSegmentsStoreObjName);
          storedTangentSegments.create();
        }
      }

      /// Main function of the segment finding by the cellular automaton.
      virtual void apply(const std::vector<CDCWireHitCluster>& inputClusters,
                         std::vector<CDCRecoSegment2D>& outputSegments) override final;

      /// Terminate the event processing
      virtual void terminate() override final
      {
        if (m_ptrFacetFilter) {
          m_ptrFacetFilter->terminate();
        }

        if (m_ptrFacetRelationFilter) {
          m_ptrFacetRelationFilter->terminate();
        }

        Super::terminate();
      }

    public:
      /// Getter for the current facet filter. The module keeps ownership of the pointer.
      FacetFilter* getFacetFilter()
      { return m_ptrFacetFilter.get(); }

      /// Setter for the facet filter used in the facet creation. The module takes ownership of the pointer.
      void setFacetFilter(std::unique_ptr<FacetFilter> ptrFacetFilter)
      { m_ptrFacetFilter = std::move(ptrFacetFilter); }

      /// Getter for the current facet relation filter. The module keeps ownership of the pointer.
      FacetRelationFilter* getFacetRelationFilter()
      { return m_ptrFacetRelationFilter.get(); }

      /// Setter for the facet relation filter used to connect facets in a network. The module takes ownership of the pointer.
      void setFacetRelationFilter(std::unique_ptr<FacetRelationFilter> ptrFacetRelationFilter)
      { m_ptrFacetRelationFilter = std::move(ptrFacetRelationFilter); }

    private:
      /// Parameter: Switch if facets shall be written to the DataStore
      bool m_param_writeFacets = false;

      /// Parameter: Name of the output StoreObjPtr of the facets generated within this module.
      std::string m_param_facetsStoreObjName = "CDCFacetVector";

      /// Parameter: Switch if tangent segments shall be written to the DataStore
      bool m_param_writeTangentSegments = false;

      /// Parameter: Name of the output StoreObjPtr of the tangent segments generated within this module.
      std::string m_param_tangentSegmentsStoreObjName = "CDCTangentSegmentVector";

    private:
      /// Reference to the filter to be used for the facet generation.
      std::unique_ptr<FacetFilter> m_ptrFacetFilter{new FacetFilter()};

      /// Reference to the relation filter to be used to construct the facet network.
      std::unique_ptr<FacetRelationFilter> m_ptrFacetRelationFilter {new FacetRelationFilter()};

    private:
      //cellular automaton
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<CDCFacet> m_cellularPathFinder;

    private:
      /// Instance of the facet creator doing the main work of creating the facets
      FacetCreator m_facetCreator;

    private: // object pools
      /// Memory for the wire hit neighborhood in within a super cluster.
      WeightedNeighborhood<const CDCWireHit> m_wirehitNeighborhood;

      /// Memory for the constructed facets.
      std::vector<CDCFacet> m_facets;

      /// Memory for the facet neighborhood.
      WeightedNeighborhood<const CDCFacet> m_facetsNeighborhood;

      /// Memory for the facet paths generated from the graph.
      std::vector< std::vector<const CDCFacet*> > m_facetPaths;

    }; // end class SegmentFinder


    template<class FacetFilter, class FacetRelationFilter>
    void SegmentFinderFacetAutomaton<FacetFilter, FacetRelationFilter>::
    apply(const std::vector<CDCWireHitCluster>& inputClusters,
          std::vector<CDCRecoSegment2D>& outputSegments)
    {
      for (const CDCWireHitCluster& cluster : inputClusters) {
        // Skip clusters that have been detected as background
        if (cluster.getBackgroundFlag()) {
          continue;
        }
        B2ASSERT("Expect the clusters to be sorted",
                 std::is_sorted(std::begin(cluster), std::end(cluster)));

        // Create the neighborhood of wire hits on the cluster
        B2DEBUG(100, "Creating the CDCWireHit neighborhood");
        const bool primaryNeighborhoodOnly = false;
        using PrimaryWireHitNeighborhoodFilter = WholeWireHitRelationFilter<primaryNeighborhoodOnly>;

        m_wirehitNeighborhood.clear();
        m_wirehitNeighborhood.appendUsing<PrimaryWireHitNeighborhoodFilter>(cluster);
        B2ASSERT("Wire neighborhood is not symmetric. Check the geometry.", m_wirehitNeighborhood.isSymmetric());
        B2DEBUG(100, "  wirehitNeighborhood.size() = " << m_wirehitNeighborhood.size());

        // Create the facets
        B2DEBUG(100, "Creating the CDCFacets");
        m_facets.clear();
        m_facetCreator.createFacets(*m_ptrFacetFilter, cluster, m_wirehitNeighborhood, m_facets);
        B2ASSERT("Facets have not been sorted", std::is_sorted(m_facets.begin(), m_facets.end()));
        B2DEBUG(100, "  Created " << m_facets.size()  << " CDCFacets");

        // Copy facets to the DataStore
        if (m_param_writeFacets) {
          StoreWrappedObjPtr< std::vector<CDCFacet> > storedFacets(m_param_facetsStoreObjName);
          std::vector<CDCFacet>& facets = *storedFacets;
          for (const CDCFacet& facet : m_facets) {
            facets.push_back(facet);
          }
        }

        // Create the facet neighborhood
        B2DEBUG(100, "Creating the CDCFacet neighborhood");
        m_facetsNeighborhood.clear();
        m_facetsNeighborhood.createUsing(*m_ptrFacetRelationFilter, m_facets);
        B2DEBUG(100, "  Created " << m_facetsNeighborhood.size()  << " FacetsNeighborhoods");

        if (m_facetsNeighborhood.size() == 0) {
          continue; // No neighborhood generated. Next cluster.
        }

        // Apply the cellular automaton in a multipass manner
        m_facetPaths.clear();
        m_cellularPathFinder.apply(m_facets, m_facetsNeighborhood, m_facetPaths);

        outputSegments.reserve(outputSegments.size() + m_facetPaths.size());
        for (const std::vector<const CDCFacet*>& facetPath : m_facetPaths) {
          outputSegments.push_back(CDCRecoSegment2D::condense(facetPath));
        }

        // Copy tangent segments to the DataStore
        if (m_param_writeTangentSegments) {
          StoreWrappedObjPtr< std::vector<CDCTangentSegment> > storedTangentSegments(m_param_tangentSegmentsStoreObjName);
          std::vector<CDCTangentSegment>& tangentSegments = *storedTangentSegments;
          for (const std::vector<const CDCFacet*>& facetPath : m_facetPaths) {
            tangentSegments.push_back(CDCTangentSegment::condense(facetPath));
          }
        }

        m_facetPaths.clear();
        m_facetsNeighborhood.clear();
        m_facets.clear();
        m_wirehitNeighborhood.clear();

        B2DEBUG(100, "  Created " << outputSegments.size()  << " selected CDCRecoSegment2Ds");
      } // end for cluster loop
    }
  } //end namespace TrackFindingCDC
} //end namespace Belle2
