/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SEGMENTFINDERCDCFACETAUTOMATONMODULE_H_
#define SEGMENTFINDERCDCFACETAUTOMATONMODULE_H_

#include <tracking/trackFindingCDC/algorithms/MultipassCellularPathFinder.h>
#include <tracking/trackFindingCDC/algorithms/Clusterizer.h>
#include <tracking/trackFindingCDC/creators/FacetCreator.h>
#include <tracking/trackFindingCDC/filters/wirehit_wirehit/WireHitNeighborChooser.h>
#include <tracking/trackFindingCDC/filters/facet/SimpleFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet_facet/SimpleFacetNeighborChooser.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/eventdata/CDCEventData.h>

#include <framework/datastore/StoreArray.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/modules/trackFinderCDC/SegmentFinderCDCBaseModule.h>

#include <vector>
#include <iterator>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Forward declaration of the module implementing the segment generation by cellular automaton on facets using specific filter instances.
    template<class FacetFilter = BaseFacetFilter, class FacetNeighborChooser = BaseFacetNeighborChooser>
    class SegmentFinderCDCFacetAutomatonImplModule;
  }

  /// Module specialisation using the default Monte Carlo free filters. To be used in production.
  typedef TrackFindingCDC::SegmentFinderCDCFacetAutomatonImplModule <
  TrackFindingCDC::SimpleFacetFilter,
                  TrackFindingCDC::SimpleFacetNeighborChooser
                  > SegmentFinderCDCFacetAutomatonModule;

  namespace TrackFindingCDC {
    template<class FacetFilter, class FacetNeighborChooser>
    class SegmentFinderCDCFacetAutomatonImplModule : public SegmentFinderCDCBaseModule {

    public:
      /// Default constructor initialising the filters with the default settings
      SegmentFinderCDCFacetAutomatonImplModule(EOrientation orientation = c_None) :
        SegmentFinderCDCBaseModule(orientation),
        m_ptrFacetFilter(new FacetFilter()),
        m_ptrFacetNeighborChooser(new FacetNeighborChooser()),
        m_param_writeClusters(false),
        m_param_clustersStoreObjName("CDCWireHitClusterVector"),
        m_param_writeFacets(false),
        m_param_facetsStoreObjName("CDCFacetVector"),
        m_param_writeTangentSegments(false),
        m_param_tangentSegmentsStoreObjName("CDCRecoTangentSegmentVector") {
        addParam("WriteClusters",
                 m_param_writeClusters,
                 "Switch if clusters shall be written to the DataStore",
                 false);

        addParam("ClustersStoreObjName",
                 m_param_clustersStoreObjName,
                 "Name of the output StoreObjPtr of the clusters generated within this module.",
                 std::string("CDCWireHitClusterVector"));

        addParam("WriteFacets",
                 m_param_writeFacets,
                 "Switch if facets shall be written to the DataStore",
                 false);

        addParam("FacetsStoreObjName",
                 m_param_facetsStoreObjName,
                 "Name of the output StoreObjPtr of the facets generated within this module.",
                 std::string("CDCWireHitFacetVector"));

        addParam("WriteTangentSegments",
                 m_param_writeTangentSegments,
                 "Switch if tangent segments shall be written to the DataStore",
                 false);

        addParam("TangentSegmentsStoreObjName",
                 m_param_tangentSegmentsStoreObjName,
                 "Name of the output StoreObjPtr of the tangent segments generated within this module.",
                 std::string("CDCRecoTangentSegmentVector"));
      }

      /// Destructor deleting the filters.
      ~SegmentFinderCDCFacetAutomatonImplModule() {
        if (m_ptrFacetFilter) delete m_ptrFacetFilter;
        m_ptrFacetFilter = nullptr;

        if (m_ptrFacetNeighborChooser) delete m_ptrFacetNeighborChooser;
        m_ptrFacetNeighborChooser = nullptr;
      }

      ///  Initialize the Module before event processing
      virtual void initialize() override {
        SegmentFinderCDCBaseModule::initialize();

        if (m_param_writeClusters) {
          StoreWrappedObjPtr< std::vector<CDCWireHitCluster> >::registerTransient(m_param_clustersStoreObjName);
        }

        if (m_param_writeFacets) {
          StoreWrappedObjPtr< std::vector<CDCRecoFacet> >::registerTransient(m_param_facetsStoreObjName);
        }

        if (m_param_writeTangentSegments) {
          StoreWrappedObjPtr< std::vector<CDCRecoTangentSegment> >::registerTransient(m_param_tangentSegmentsStoreObjName);
        }

        if (m_ptrFacetFilter) {
          m_ptrFacetFilter->initialize();
        }

        if (m_ptrFacetNeighborChooser) {
          m_ptrFacetNeighborChooser->initialize();
        }

      }

      /// Generates the segment from Monte Carlo information. Default orientation is the flight direction.
      virtual void generate(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments) override final;
      // implementation below


      virtual void terminate() override {
        if (m_ptrFacetFilter) {
          m_ptrFacetFilter->terminate();
        }

        if (m_ptrFacetNeighborChooser) {
          m_ptrFacetNeighborChooser->terminate();
        }

        SegmentFinderCDCBaseModule::terminate();
      }

    public:
      /// Getter for the current facet filter. The module keeps ownership of the pointer.
      FacetFilter* getFacetFilter() {
        return m_ptrFacetFilter;
      }

      /// Setter for the facet filter used in the facet creation. The module takes ownership of the pointer.
      void setFacetFilter(FacetFilter* ptrFacetFilter) {
        if (m_ptrFacetFilter) delete m_ptrFacetFilter;
        m_ptrFacetFilter = ptrFacetFilter;
      }

      /// Getter for the current facet filter. The module keeps ownership of the pointer.
      FacetNeighborChooser* getFacetNeighborChooser() {
        return m_ptrFacetNeighborChooser;
      }

      /// Setter for the facet neighbor chooser used to connect facets in a network. The module takes ownership of the pointer.
      void setFacetNeighborChooser(FacetNeighborChooser* ptrFacetNeighborChooser) {
        if (m_ptrFacetNeighborChooser) delete m_ptrFacetNeighborChooser;
        m_ptrFacetNeighborChooser = ptrFacetNeighborChooser;
      }

    private:
      /// Reference to the filter to be used for the facet generation.
      FacetFilter* m_ptrFacetFilter;

      /// Reference to the chooser to be used to construct the facet network.
      FacetNeighborChooser* m_ptrFacetNeighborChooser;

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
      CDCRecoFacetCollection m_facets;

      /// Neighborhood type for facets.
      typedef WeightedNeighborhood<const CDCRecoFacet> CDCRecoFacetNeighborhood;

      /// Memory for the facet neighborhood.
      CDCRecoFacetNeighborhood m_facetsNeighborhood;

      /// Memory for the facet paths generated from the graph.
      std::vector< std::vector<const CDCRecoFacet*> > m_facetPaths;

      /// Memory for the hit clusters in the current superlayer
      std::vector<CDCWireHitCluster> m_clustersInSuperLayer;

      /// Instance of the hit cluster generator
      Clusterizer<CDCWireHit, CDCWireHitCluster> m_wirehitClusterizer;

      //object creators
      /// Instance of the facet creator
      FacetCreator m_facetCreator;

      /// Neighborhood type for wire hits
      typedef WeightedNeighborhood<const CDCWireHit> CDCWireHitNeighborhood;

      /// Memory for the wire hit neighborhood.
      CDCWireHitNeighborhood m_wirehitNeighborhood;

      //cellular automaton
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<CDCRecoFacet> m_cellularPathFinder;

    }; // end class SegmentFinderCDCFacetAutomatonImplModule


    template<class FacetFilter, class FacetNeighborChooser>
    void SegmentFinderCDCFacetAutomatonImplModule<FacetFilter, FacetNeighborChooser>::generate(std::vector<CDCRecoSegment2D>& segments)
    {

      /// Attain cluster vector on the DataStore if needed.
      std::vector<CDCWireHitCluster>* ptrClusters = nullptr;
      if (m_param_writeClusters) {
        StoreWrappedObjPtr< std::vector<CDCWireHitCluster> > storedClusters(m_param_clustersStoreObjName);
        storedClusters.create();
        std::vector<CDCWireHitCluster>& clusters = *storedClusters;
        ptrClusters = &clusters;
      }

      /// Attain facet vector on the DataStore if needed.
      std::vector<CDCRecoFacet>* ptrFacets = nullptr;
      if (m_param_writeFacets) {
        StoreWrappedObjPtr< std::vector<CDCRecoFacet> > storedFacets(m_param_facetsStoreObjName);
        storedFacets.create();
        std::vector<CDCRecoFacet>& facets = *storedFacets;
        ptrFacets = &facets;
      }

      /// Attain tangent vector on the DataStore if needed.
      std::vector<CDCRecoTangentSegment>* ptrTangentSegments = nullptr;
      if (m_param_writeTangentSegments) {
        StoreWrappedObjPtr< std::vector<CDCRecoTangentSegment> > storedTangentSegments(m_param_tangentSegmentsStoreObjName);
        storedTangentSegments.create();
        std::vector<CDCRecoTangentSegment>& tangentSegments = *storedTangentSegments;
        ptrTangentSegments = &tangentSegments;
      }

      const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
      const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

      for (const CDCWireSuperLayer & wireSuperLayer : wireTopology.getWireSuperLayers()) {

        const CDCWireHitTopology::CDCWireHitRange wireHitsInSuperlayer = wireHitTopology.getWireHits(wireSuperLayer);
        //create the neighborhood
        B2DEBUG(100, "Creating the CDCWireHit neighborhood");
        m_wirehitNeighborhood.clear();

        B2DEBUG(100, "  Append clockwise neighborhood");
        m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CW_NEIGHBOR>>(wireHitsInSuperlayer);

        B2DEBUG(100, "  Append clockwise out neighborhood");
        m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CW_OUT_NEIGHBOR>>(wireHitsInSuperlayer);

        B2DEBUG(100, "  Append clockwise in neighborhood");
        m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CW_IN_NEIGHBOR>>(wireHitsInSuperlayer);

        B2DEBUG(100, "  Append counter clockwise neighborhood");
        m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CCW_NEIGHBOR>>(wireHitsInSuperlayer);

        B2DEBUG(100, "  Append counter clockwise out neighborhood");
        m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CCW_OUT_NEIGHBOR>>(wireHitsInSuperlayer);

        B2DEBUG(100, "  Append counter clockwise in neighborhood");
        m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CCW_IN_NEIGHBOR>>(wireHitsInSuperlayer);

        assert(m_wirehitNeighborhood.isSymmetric());

        B2DEBUG(100, "  allWirehitNeighbors.size() = " << m_wirehitNeighborhood.size());

        //create the clusters
        B2DEBUG(100, "Creating the CDCWireHit clusters");
        m_clustersInSuperLayer.clear();
        m_wirehitClusterizer.create(wireHitsInSuperlayer, m_wirehitNeighborhood, m_clustersInSuperLayer);
        B2DEBUG(100, "Created " << m_clustersInSuperLayer.size() << " CDCWireHit clusters in superlayer");

        for (CDCWireHitCluster & cluster : m_clustersInSuperLayer) {
          B2DEBUG(100, "Cluster size: " << cluster.size());
          B2DEBUG(100, "Wire hit neighborhood size: " << m_wirehitNeighborhood.size());

          // Create the facets
          B2DEBUG(100, "Creating the CDCRecoFacets");
          m_facets.clear();
          m_facetCreator.createFacets(*m_ptrFacetFilter, cluster, m_wirehitNeighborhood, m_facets);
          B2DEBUG(100, "  Created " << m_facets.size()  << " CDCRecoFacets");

          // Create the facet neighborhood
          B2DEBUG(100, "Creating the CDCRecoFacet neighborhood");
          m_facetsNeighborhood.clear();
          m_facetsNeighborhood.createUsing(*m_ptrFacetNeighborChooser, m_facets);
          B2DEBUG(100, "  Created " << m_facetsNeighborhood.size()  << " FacetsNeighborhoods");

          if (m_facetsNeighborhood.size() == 0) {
            continue; //No neighborhood generated. Next cluster.
          }

          // Apply the cellular automaton in a multipass manner
          m_facetPaths.clear();
          m_cellularPathFinder.apply(m_facets, m_facetsNeighborhood, m_facetPaths);

          segments.reserve(segments.size() + m_facetPaths.size());
          for (const std::vector<const CDCRecoFacet*>& facetPath : m_facetPaths) {
            segments.push_back(CDCRecoSegment2D::condense(facetPath));
          }

          // Copy tangent segments to the DataStore
          if (m_param_writeTangentSegments and ptrTangentSegments) {
            std::vector<CDCRecoTangentSegment>& tangentSegments = *ptrTangentSegments;
            for (const std::vector<const CDCRecoFacet*>& facetPath : m_facetPaths) {
              tangentSegments.push_back(CDCRecoTangentSegment::condense(facetPath));
            }
          }

          // Move facets to the DataStore
          if (m_param_writeFacets and ptrFacets) {
            std::vector<CDCRecoFacet>& facets = *ptrFacets;
            for (const CDCRecoFacet & facet : m_facets) {
              facets.push_back(std::move(facet));
            }
          }

          m_facetPaths.clear();
          m_facetsNeighborhood.clear();
          m_facets.clear();

          //TODO: combine matching segments here

          B2DEBUG(100, "  Created " << segments.size()  << " selected CDCRecoSegment2Ds");
        } // end for cluster loop

        // Move clusters to the DataStore
        if (m_param_writeClusters and ptrClusters) {
          std::vector<CDCWireHitCluster>& clusters = *ptrClusters;
          clusters.insert(clusters.end(),
                          std::make_move_iterator(m_clustersInSuperLayer.begin()),
                          std::make_move_iterator(m_clustersInSuperLayer.end()));
        }

        m_clustersInSuperLayer.clear();
        m_wirehitNeighborhood.clear();

        //TODO: or combine matching segments here

      } // end for superlayer loop

    }

  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif // SEGMENTFINDERCDCFACETAUTOMATONMODULE_H_
