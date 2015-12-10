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
#include <tracking/trackFindingCDC/ca/Clusterizer.h>
#include <tracking/trackFindingCDC/creators/FacetCreator.h>
#include <tracking/trackFindingCDC/filters/wirehit_relation/WholeWireHitRelationFilter.h>
#include <tracking/trackFindingCDC/filters/cluster/BaseClusterFilter.h>
#include <tracking/trackFindingCDC/filters/cluster/AllClusterFilter.h>
#include <tracking/trackFindingCDC/filters/facet/RealisticFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet_relation/SimpleFacetRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCTangentSegment.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <framework/datastore/StoreArray.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/modules/trackFinderCDC/SegmentFinderCDCBySuperClusterModule.h>

#include <vector>
#include <iterator>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Forward declaration of the module implementing the segment generation by cellular automaton on facets using specific filter instances.
    template<class ClusterFilter = BaseClusterFilter,
             class FacetFilter = BaseFacetFilter,
             class FacetRelationFilter = BaseFacetRelationFilter,
             class SegmentRelationFilter = BaseSegmentRelationFilter>
    class SegmentFinderCDCFacetAutomatonImplModule;
  }

  /// Module specialisation using the default Monte Carlo free filters. To be used in production.
  typedef TrackFindingCDC::SegmentFinderCDCFacetAutomatonImplModule <
  TrackFindingCDC::AllClusterFilter,
                  TrackFindingCDC::RealisticFacetFilter,
                  TrackFindingCDC::SimpleFacetRelationFilter >
                  SegmentFinderCDCFacetAutomatonModule;

  namespace TrackFindingCDC {
    template<class ClusterFilter,
             class FacetFilter,
             class FacetRelationFilter,
             class SegmentRelationFilter>
    class SegmentFinderCDCFacetAutomatonImplModule :
      public SegmentFinderCDCBySuperClusterModule<SegmentRelationFilter> {

    private:
      /// Type of the base class
      typedef SegmentFinderCDCBySuperClusterModule<SegmentRelationFilter> Super;

    public:
      /// Default constructor initialising the filters with the default settings
      SegmentFinderCDCFacetAutomatonImplModule() :
        m_ptrClusterFilter(new ClusterFilter()),
        m_ptrFacetFilter(new FacetFilter()),
        m_ptrFacetRelationFilter(new FacetRelationFilter()),
        m_param_writeClusters(false),
        m_param_clustersStoreObjName("CDCWireHitClusterVector"),
        m_param_writeFacets(false),
        m_param_facetsStoreObjName("CDCFacetVector"),
        m_param_writeTangentSegments(false),
        m_param_tangentSegmentsStoreObjName("CDCTangentSegmentVector")
      {

        this->setDescription("Generates segments from hits using a cellular automaton build from hit triples (facets).");

        this->addParam("WriteClusters",
                       m_param_writeClusters,
                       "Switch if clusters shall be written to the DataStore",
                       false);

        this->addParam("ClustersStoreObjName",
                       m_param_clustersStoreObjName,
                       "Name of the output StoreObjPtr of the clusters generated within this module.",
                       std::string("CDCWireHitClusterVector"));

        this->addParam("WriteFacets",
                       m_param_writeFacets,
                       "Switch if facets shall be written to the DataStore",
                       false);

        this->addParam("FacetsStoreObjName",
                       m_param_facetsStoreObjName,
                       "Name of the output StoreObjPtr of the facets generated within this module.",
                       std::string("CDCFacetVector"));

        this->addParam("WriteTangentSegments",
                       m_param_writeTangentSegments,
                       "Switch if tangent segments shall be written to the DataStore",
                       false);

        this->addParam("TangentSegmentsStoreObjName",
                       m_param_tangentSegmentsStoreObjName,
                       "Name of the output StoreObjPtr of the tangent segments generated within this module.",
                       std::string("CDCTangentSegmentVector"));

        ModuleParamList moduleParamList = this->getParamList();

        this->getClusterFilter()->exposeParameters(&moduleParamList);
        this->getFacetFilter()->exposeParameters(&moduleParamList);
        this->getFacetRelationFilter()->exposeParameters(&moduleParamList);
        this->getSegmentRelationFilter()->exposeParameters(&moduleParamList);

        this->setParamList(moduleParamList);
      }

      /// Initialize the Module before event processing
      virtual void initialize() override
      {
        Super::initialize();

        if (m_param_writeClusters) {
          StoreWrappedObjPtr< std::vector<CDCWireHitCluster> >::registerTransient(m_param_clustersStoreObjName);
        }

        if (m_param_writeFacets) {
          StoreWrappedObjPtr< std::vector<CDCFacet> >::registerTransient(m_param_facetsStoreObjName);
        }

        if (m_param_writeTangentSegments) {
          StoreWrappedObjPtr< std::vector<CDCTangentSegment> >::registerTransient(m_param_tangentSegmentsStoreObjName);
        }

        if (m_ptrClusterFilter) {
          m_ptrClusterFilter->initialize();
        }

        if (m_ptrFacetFilter) {
          m_ptrFacetFilter->initialize();
        }

        if (m_ptrFacetRelationFilter) {
          m_ptrFacetRelationFilter->initialize();
        }

      }

      /// Processes the current event
      void event() override
      {
        this->getClusterFilter()->beginEvent();
        this->getFacetFilter()->beginEvent();
        this->getFacetRelationFilter()->beginEvent();
        this->getSegmentRelationFilter()->beginEvent();

        /// Attain cluster vector on the DataStore if needed.
        if (m_param_writeClusters) {
          StoreWrappedObjPtr< std::vector<CDCWireHitCluster> > storedClusters(m_param_clustersStoreObjName);
          storedClusters.create();
        }

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

        Super::event();
      }

      /// Generates the segment from Monte Carlo information. Default orientation is the flight direction.
      virtual void generateSegmentsFromSuperCluster(const CDCWireHitCluster& superCluster,
                                                    std::vector<CDCRecoSegment2D>& segments) override final;
      // implementation below

      virtual void terminate() override
      {
        if (m_ptrClusterFilter) {
          m_ptrClusterFilter->terminate();
        }

        if (m_ptrFacetFilter) {
          m_ptrFacetFilter->terminate();
        }

        if (m_ptrFacetRelationFilter) {
          m_ptrFacetRelationFilter->terminate();
        }

        Super::terminate();
      }

    public:
      /// Getter for the current cluster filter. The module keeps ownership of the pointer.
      ClusterFilter* getClusterFilter()
      {
        return m_ptrClusterFilter.get();
      }

      /// Setter for the cluster filter used in the cluster creation. The module takes ownership of the pointer.
      void setClusterFilter(std::unique_ptr<ClusterFilter> ptrClusterFilter)
      {
        m_ptrClusterFilter = std::move(ptrClusterFilter);
      }

      /// Getter for the current facet filter. The module keeps ownership of the pointer.
      FacetFilter* getFacetFilter()
      {
        return m_ptrFacetFilter.get();
      }

      /// Setter for the facet filter used in the facet creation. The module takes ownership of the pointer.
      void setFacetFilter(std::unique_ptr<FacetFilter> ptrFacetFilter)
      {
        m_ptrFacetFilter = std::move(ptrFacetFilter);
      }

      /// Getter for the current facet relation filter. The module keeps ownership of the pointer.
      FacetRelationFilter* getFacetRelationFilter()
      {
        return m_ptrFacetRelationFilter.get();
      }

      /// Setter for the facet relation filter used to connect facets in a network. The module takes ownership of the pointer.
      void setFacetRelationFilter(std::unique_ptr<FacetRelationFilter> ptrFacetRelationFilter)
      {
        m_ptrFacetRelationFilter = std::move(ptrFacetRelationFilter);
      }

    private:
      /// Reference to the filter to be used for the facet generation.
      std::unique_ptr<ClusterFilter> m_ptrClusterFilter;

      /// Reference to the filter to be used for the facet generation.
      std::unique_ptr<FacetFilter> m_ptrFacetFilter;

      /// Reference to the relation filter to be used to construct the facet network.
      std::unique_ptr<FacetRelationFilter> m_ptrFacetRelationFilter;

    private:
      /// Parameter: Switch if clusters shall be written to the DataStore
      bool m_param_writeClusters;

      /// Parameter: Name of the output StoreObjPtr of the clusters generated within this module.
      std::string m_param_clustersStoreObjName;

      /// Parameter: Switch if facets shall be written to the DataStore
      bool m_param_writeFacets;

      /// Parameter: Name of the output StoreObjPtr of the facets generated within this module.
      std::string m_param_facetsStoreObjName;

      /// Parameter: Switch if tangent segments shall be written to the DataStore
      bool m_param_writeTangentSegments;

      /// Parameter: Name of the output StoreObjPtr of the tangent segments generated within this module.
      std::string m_param_tangentSegmentsStoreObjName;

    private:
      //object pools
      /// Memory for the constructed facets.
      std::vector<CDCFacet> m_facets;

      /// Neighborhood type for facets.
      typedef WeightedNeighborhood<const CDCFacet> CDCFacetNeighborhood;

      /// Memory for the facet neighborhood.
      CDCFacetNeighborhood m_facetsNeighborhood;

      /// Memory for the facet paths generated from the graph.
      std::vector< std::vector<const CDCFacet*> > m_facetPaths;

      /// Memory for the hit clusters in the current supercluster
      std::vector<CDCWireHitCluster> m_clustersInSuperCluster;

      /// Instance of the hit cluster generator
      Clusterizer<CDCWireHit, CDCWireHitCluster> m_wirehitClusterizer;

      //object creators
      /// Instance of the facet creator
      FacetCreator m_facetCreator;

      /// Neighborhood type for wire hits
      typedef WeightedNeighborhood<const CDCWireHit> CDCWireHitNeighborhood;

      /// Memory for the wire hit neighborhood in a super cluster.
      CDCWireHitNeighborhood m_wirehitNeighborhood;

      //cellular automaton
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<CDCFacet> m_cellularPathFinder;

    }; // end class SegmentFinderCDCFacetAutomatonImplModule


    template<class ClusterFilter,
             class FacetFilter,
             class FacetRelationFilter,
             class SegmentRelationFilter>
    void
    SegmentFinderCDCFacetAutomatonImplModule <ClusterFilter,
                                             FacetFilter,
                                             FacetRelationFilter,
                                             SegmentRelationFilter>
                                             ::generateSegmentsFromSuperCluster(const CDCWireHitCluster& superCluster,
                                                 std::vector<CDCRecoSegment2D>& segments)
    {
      // create the neighborhood
      B2DEBUG(100, "Creating the CDCWireHit neighborhood");
      m_wirehitNeighborhood.clear();

      const bool primaryNeighborhoodOnly = false;
      m_wirehitNeighborhood.appendUsing<WholeWireHitRelationFilter<primaryNeighborhoodOnly>>(superCluster);
      assert(m_wirehitNeighborhood.isSymmetric());
      B2DEBUG(100, "  wirehitNeighborhood.size() = " << m_wirehitNeighborhood.size());

      //create the clusters
      B2DEBUG(100, "Creating the CDCWireHit clusters");
      m_clustersInSuperCluster.clear();
      m_wirehitClusterizer.createFromPointers(superCluster, m_wirehitNeighborhood, m_clustersInSuperCluster);
      B2DEBUG(100, "Created " << m_clustersInSuperCluster.size() << " CDCWireHit clusters with super cluster");

      for (CDCWireHitCluster& cluster : m_clustersInSuperCluster) {
        std::sort(std::begin(cluster), std::end(cluster));
        assert(std::is_sorted(std::begin(cluster), std::end(cluster)));

        B2DEBUG(100, "Analyse the cluster for background");
        CellWeight clusterWeight = (*m_ptrClusterFilter)(cluster);
        if (isNotACell(clusterWeight)) {
          // Cluster detected as background
          for (const CDCWireHit* wireHit : cluster) {
            wireHit->getAutomatonCell().setBackgroundFlag();
            wireHit->getAutomatonCell().setTakenFlag();
          }
          // skip to next cluster
          continue;
        }

        B2DEBUG(100, "Cluster size: " << cluster.size());
        B2DEBUG(100, "Wire hit neighborhood size: " << m_wirehitNeighborhood.size());

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
          continue; //No neighborhood generated. Next cluster.
        }

        // Apply the cellular automaton in a multipass manner
        m_facetPaths.clear();
        m_cellularPathFinder.apply(m_facets, m_facetsNeighborhood, m_facetPaths);

        segments.reserve(segments.size() + m_facetPaths.size());
        for (const std::vector<const CDCFacet*>& facetPath : m_facetPaths) {
          segments.push_back(CDCRecoSegment2D::condense(facetPath));
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

        B2DEBUG(100, "  Created " << segments.size()  << " selected CDCRecoSegment2Ds");
      } // end for cluster loop

      // Move clusters to the DataStore
      if (m_param_writeClusters) {
        StoreWrappedObjPtr< std::vector<CDCWireHitCluster> > storedClusters(m_param_clustersStoreObjName);
        std::vector<CDCWireHitCluster>& clusters = *storedClusters;
        clusters.insert(clusters.end(),
                        std::make_move_iterator(m_clustersInSuperCluster.begin()),
                        std::make_move_iterator(m_clustersInSuperCluster.end()));
      }

      m_clustersInSuperCluster.clear();
      m_wirehitNeighborhood.clear();

      // TODO: combine matching segments here
    }

  } //end namespace TrackFindingCDC
} //end namespace Belle2

