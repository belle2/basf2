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

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

#include <tracking/modules/trackFinderCDC/SegmentFinderCDCBySuperClusterModule.h>

#include <tracking/trackFindingCDC/findlets/SuperClusterCreator.h>
#include <tracking/trackFindingCDC/findlets/ClusterBackgroundDetector.h>
#include <tracking/trackFindingCDC/findlets/ClusterRefiner.h>
#include <tracking/trackFindingCDC/findlets/SegmentFinderFacetAutomaton.h>

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
      SegmentFinderCDCFacetAutomatonImplModule()
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

        ModuleParamList moduleParamList = this->getParamList();
        m_clusterRefiner.exposeParameters(&moduleParamList);
        m_clusterBackgroundDetector.exposeParameters(&moduleParamList);
        m_segmentFinder.exposeParameters(&moduleParamList);
        this->getSegmentRelationFilter()->exposeParameters(&moduleParamList);
        this->setParamList(moduleParamList);
      }

      /// Initialize the Module before event processing
      virtual void initialize() override
      {
        Super::initialize();
        m_clusterRefiner.initialize();
        m_clusterBackgroundDetector.initialize();
        m_segmentFinder.initialize();

        if (m_param_writeClusters) {
          StoreWrappedObjPtr< std::vector<CDCWireHitCluster> >::registerTransient(m_param_clustersStoreObjName);
        }
      }

      /// Processes the current event
      void event() override
      {
        m_clusterRefiner.beginEvent();
        m_clusterBackgroundDetector.beginEvent();
        m_segmentFinder.beginEvent();
        this->getSegmentRelationFilter()->beginEvent();

        /// Attain cluster vector on the DataStore if needed.
        if (m_param_writeClusters) {
          StoreWrappedObjPtr< std::vector<CDCWireHitCluster> > storedClusters(m_param_clustersStoreObjName);
          storedClusters.create();
        }
        Super::event();
      }

      /// Generates the segment from Monte Carlo information. Default orientation is the flight direction.
      virtual void generateSegmentsFromSuperCluster(const CDCWireHitCluster& superCluster,
                                                    std::vector<CDCRecoSegment2D>& segments) override final;
      // implementation below

      virtual void terminate() override
      {

        m_segmentFinder.terminate();
        m_clusterBackgroundDetector.terminate();
        m_clusterRefiner.terminate();
        Super::terminate();
      }

    private:
      /// Parameter: Switch if clusters shall be written to the DataStore
      bool m_param_writeClusters = false;

      /// Parameter: Name of the output StoreObjPtr of the clusters generated within this module.
      std::string m_param_clustersStoreObjName = "CDCWireHitClusterVector";

    private:
      // Findlets
      /// Creates the clusters from super clusters
      ClusterRefiner<PrimaryWireHitRelationFilter> m_clusterRefiner;

      /// Marks the clusters as background
      ClusterBackgroundDetector<ClusterFilter> m_clusterBackgroundDetector;

      /// Find the segments by composition of facets path from a cellular automaton
      SegmentFinderFacetAutomaton<FacetFilter, FacetRelationFilter> m_segmentFinder;

      /// Memory for the hit clusters
      std::vector<CDCWireHitCluster> m_clustersInSuperCluster ;

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
      std::vector<CDCWireHitCluster> superClusters;
      superClusters.push_back(superCluster);
      m_clustersInSuperCluster.clear();
      m_clusterRefiner.apply(superClusters, m_clustersInSuperCluster);
      m_clusterBackgroundDetector.apply(m_clustersInSuperCluster);
      m_segmentFinder.apply(m_clustersInSuperCluster, segments);

      // Move clusters to the DataStore
      if (m_param_writeClusters) {
        StoreWrappedObjPtr< std::vector<CDCWireHitCluster> > storedClusters(m_param_clustersStoreObjName);
        std::vector<CDCWireHitCluster>& clusters = *storedClusters;
        clusters.insert(clusters.end(),
                        std::make_move_iterator(m_clustersInSuperCluster.begin()),
                        std::make_move_iterator(m_clustersInSuperCluster.end()));
      }

      m_clustersInSuperCluster.clear();
    }

  } //end namespace TrackFindingCDC
} //end namespace Belle2

