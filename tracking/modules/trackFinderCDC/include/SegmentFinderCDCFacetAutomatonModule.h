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

#include <tracking/trackFindingCDC/filters/cluster/BaseClusterFilter.h>
#include <tracking/trackFindingCDC/filters/cluster/AllClusterFilter.h>
#include <tracking/trackFindingCDC/filters/facet/RealisticFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet_relation/SimpleFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segment_relation/BaseSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>


#include <tracking/trackFindingCDC/basemodules/SegmentFinderCDCBaseModule.h>

#include <tracking/trackFindingCDC/findlets/minimal/SuperClusterCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterBackgroundDetector.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterRefiner.h>
#include <tracking/trackFindingCDC/findlets/minimal/FacetCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentMerger.h>

#include <tracking/trackFindingCDC/findlets/base/StoreVectorSwapper.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <vector>

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
      public SegmentFinderCDCBaseModule {

    private:
      /// Type of the base class
      typedef SegmentFinderCDCBaseModule Super;

    public:
      /// Default constructor initialising the filters with the default settings
      SegmentFinderCDCFacetAutomatonImplModule()
      {

        this->setDescription("Generates segments from hits using a cellular automaton build from hit triples (facets).");
        ModuleParamList moduleParamList = this->getParamList();
        m_superClusterCreator.exposeParameters(&moduleParamList);
        m_clusterRefiner.exposeParameters(&moduleParamList);
        m_clusterBackgroundDetector.exposeParameters(&moduleParamList);
        m_facetCreator.exposeParameters(&moduleParamList);
        m_segmentCreator.exposeParameters(&moduleParamList);
        m_segmentMerger.exposeParameters(&moduleParamList);

        m_superClusterSwapper.exposeParameters(&moduleParamList);
        m_clusterSwapper.exposeParameters(&moduleParamList);
        m_facetSwapper.exposeParameters(&moduleParamList);

        this->setParamList(moduleParamList);
      }

      /// Initialize the Module before event processing
      virtual void initialize() override
      {
        Super::initialize();
        m_superClusterCreator.initialize();
        m_clusterRefiner.initialize();
        m_clusterBackgroundDetector.initialize();
        m_facetCreator.initialize();
        m_segmentCreator.initialize();
        m_segmentMerger.initialize();

        m_superClusterSwapper.initialize();
        m_clusterSwapper.initialize();
        m_facetSwapper.initialize();
      }

      /// Processes the current event
      void event() override
      {
        m_superClusterCreator.beginEvent();
        m_clusterRefiner.beginEvent();
        m_clusterBackgroundDetector.beginEvent();
        m_facetCreator.beginEvent();
        m_segmentCreator.beginEvent();
        m_segmentMerger.beginEvent();

        m_superClusterSwapper.beginEvent();
        m_clusterSwapper.beginEvent();
        m_facetSwapper.beginEvent();

        m_superClusters.clear();
        m_clusters.clear();
        m_facets.clear();
        m_segments.clear();
        m_mergedSegments.clear();

        Super::event();
      }

      /// Generates the segment.
      virtual void generateSegments(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments) override final;

      // implementation below
      virtual void terminate() override
      {
        m_facetSwapper.terminate();
        m_clusterSwapper.terminate();
        m_superClusterSwapper.terminate();

        m_segmentMerger.terminate();
        m_segmentCreator.terminate();
        m_facetCreator.terminate();
        m_clusterBackgroundDetector.terminate();
        m_clusterRefiner.terminate();
        m_superClusterCreator.terminate();
        Super::terminate();
      }

    private:
      // Findlets
      /// Composes the super clusters.
      SuperClusterCreator m_superClusterCreator;

      /// Creates the clusters from super clusters
      ClusterRefiner<PrimaryWireHitRelationFilter> m_clusterRefiner;

      /// Marks the clusters as background
      ClusterBackgroundDetector<ClusterFilter> m_clusterBackgroundDetector;

      /// Find the segments by composition of facets path from a cellular automaton
      FacetCreator<FacetFilter> m_facetCreator;

      /// Find the segments by composition of facets path from a cellular automaton
      SegmentCreatorFacetAutomaton<FacetRelationFilter> m_segmentCreator;

      /// Merges segments with closeby segments of the same super cluster
      SegmentMerger<SegmentRelationFilter> m_segmentMerger;

      /// Puts the internal super clusters on the DataStore
      StoreVectorSwapper<CDCWireHitCluster> m_superClusterSwapper{
        "CDCWireHitSuperClusterVector",
        "superCluster",
        "wire hit super cluster"
      };

      /// Puts the internal clusters on the DataStore
      StoreVectorSwapper<CDCWireHitCluster> m_clusterSwapper{"CDCWireHitClusterVector"};

      /// Puts the internal clusters on the DataStore
      StoreVectorSwapper<CDCFacet> m_facetSwapper{"CDCFacetVector"};



      /// Memory for the wire hit clusters
      std::vector<CDCWireHitCluster> m_clusters;

      /// Memory for the wire hit super cluster
      std::vector<CDCWireHitCluster> m_superClusters;

      /// Memory for the reconstructed segments
      std::vector<CDCFacet> m_facets;

      /// Memory for the reconstructed segments
      std::vector<CDCRecoSegment2D> m_segments;

      /// Memory for the reconstructed segments
      std::vector<CDCRecoSegment2D> m_mergedSegments;

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
                                             ::generateSegments(std::vector<CDCRecoSegment2D>& outputSegments)
    {
      CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
      std::vector<CDCWireHit>& wireHits = wireHitTopology.getWireHits();

      m_superClusterCreator.apply(wireHits, m_superClusters);
      m_clusterRefiner.apply(m_superClusters, m_clusters);
      m_clusterBackgroundDetector.apply(m_clusters);
      m_facetCreator.apply(m_clusters, m_facets);
      m_segmentCreator.apply(m_facets, m_segments);
      m_segmentMerger.apply(m_segments, m_mergedSegments);

      // Move superclusters to the DataStore
      m_superClusterSwapper.apply(m_superClusters);
      // Move clusters to the DataStore
      m_clusterSwapper.apply(m_clusters);
      // Move facets to the DataStore
      m_facetSwapper.apply(m_facets);

      outputSegments.swap(m_mergedSegments);
    }

  } //end namespace TrackFindingCDC
} //end namespace Belle2
