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
#include <tracking/trackFindingCDC/filters/wirehit_relation/WholeWireHitRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segment_relation/BaseSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/basemodules/SegmentFinderCDCBaseModule.h>

#include <tracking/trackFindingCDC/findlets/SuperClusterCreator.h>
#include <tracking/trackFindingCDC/findlets/SegmentMerger.h>

#include <vector>
#include <iterator>
#include <assert.h>

namespace Belle2 {

  namespace TrackFindingCDC {
    template<class SegmentRelationFilter = BaseSegmentRelationFilter>
    class SegmentFinderCDCBySuperClusterModule : public SegmentFinderCDCBaseModule {

    private:
      typedef SegmentFinderCDCBaseModule Super;

    public:
      /// Default constructor initialising the filters with the default settings
      SegmentFinderCDCBySuperClusterModule()
      {
        setDescription("Generates segments from super clusters and tries to merge segments in the same supercluster.");

        addParam("WriteSuperClusters",
                 m_param_writeSuperClusters,
                 "Switch if superclusters shall be written to the DataStore",
                 false);

        addParam("SuperClustersStoreObjName",
                 m_param_superClustersStoreObjName,
                 "Name of the output StoreObjPtr of the super clusters generated within this module.",
                 std::string("CDCWireHitSuperClusterVector"));

        ModuleParamList moduleParamList = this->getParamList();
        m_superClusterCreator.exposeParameters(&moduleParamList);
        m_segmentMerger.exposeParameters(&moduleParamList);
        this->setParamList(moduleParamList);
      }

      /// Initialize the Module before event processing
      virtual void initialize() override
      {
        Super::initialize();
        m_superClusterCreator.initialize();
        m_segmentMerger.initialize();

        if (m_param_writeSuperClusters) {
          StoreWrappedObjPtr< std::vector<CDCWireHitCluster> >::registerTransient(m_param_superClustersStoreObjName);
        }
      }

      /// Generates the segment.
      virtual void generateSegments(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments) override final;


      /// Generates the segment in the given super cluster of hits
      virtual void generateSegmentsFromSuperCluster(const CDCWireHitCluster& /*superCluster*/,
                                                    std::vector<CDCRecoSegment2D>& /*segments*/)
      {
      }

      virtual void terminate() override
      {
        m_segmentMerger.terminate();
        m_superClusterCreator.terminate();
        Super::terminate();
      }

    private:
      /// Parameter: Switch if superclusters shall be written to the DataStore
      bool m_param_writeSuperClusters = false;

      /// Parameter: Name of the output StoreObjPtr of the superclusters generated within this module.
      std::string m_param_superClustersStoreObjName = "CDCWireHitSuperClusterVector";

    private:
      /** Memory for the hit superclusters in the current superlayer
       *  Superclusters generated from the secondary neighborhood
       *  of hits and allow for hit inefficiencies along the trajectory.
       */
      std::vector<CDCWireHitCluster> m_superClusters;

      /// Memory for the segments in each super cluster
      std::vector<std::vector<CDCRecoSegment2D> > m_segmentsByISuperCluster;

      /// Composes the super clusters.
      SuperClusterCreator m_superClusterCreator;

      /// Merges segments with closeby segments of the same super cluster
      SegmentMerger<SegmentRelationFilter> m_segmentMerger;

    }; // end class SegmentFinderCDCBySuperClusterModule


    template<class SegmentRelationFilter>
    void
    SegmentFinderCDCBySuperClusterModule<SegmentRelationFilter>
    ::generateSegments(std::vector<CDCRecoSegment2D>& segments)
    {
      m_superClusterCreator.beginEvent();
      m_segmentMerger.beginEvent();
      m_superClusters.clear();
      m_segmentsByISuperCluster.clear();

      CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
      std::vector<CDCWireHit>& wireHits = wireHitTopology.getWireHits();
      m_superClusterCreator.apply(wireHits, m_superClusters);

      int iSuperCluster = -1;
      for (CDCWireHitCluster& superCluster : m_superClusters) {
        ++iSuperCluster;
        assert(std::is_sorted(std::begin(superCluster), std::end(superCluster)));

        m_segmentsByISuperCluster.emplace_back();
        std::vector<CDCRecoSegment2D>& segmentsInSuperCluster = m_segmentsByISuperCluster.back();
        segmentsInSuperCluster.clear();

        generateSegmentsFromSuperCluster(superCluster, segmentsInSuperCluster);
        m_segmentMerger.apply(segmentsInSuperCluster, segments);
      } // end super cluster loop


      // Attain super cluster vector on the DataStore if needed.
      if (m_param_writeSuperClusters) {
        StoreWrappedObjPtr< std::vector<CDCWireHitCluster> > storedSuperClusters(m_param_superClustersStoreObjName);
        storedSuperClusters.create();
      }

      // Move clusters to the DataStore
      if (m_param_writeSuperClusters) {
        StoreWrappedObjPtr< std::vector<CDCWireHitCluster> >
        storedSuperClusters(m_param_superClustersStoreObjName);
        std::vector<CDCWireHitCluster>& superClusters = *storedSuperClusters;
        superClusters.swap(m_superClusters);
      }
    }

  } //end namespace TrackFindingCDC
} //end namespace Belle2
