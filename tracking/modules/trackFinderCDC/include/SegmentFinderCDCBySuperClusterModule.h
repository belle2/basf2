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
#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/basemodules/SegmentFinderCDCBaseModule.h>

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
      SegmentFinderCDCBySuperClusterModule() :
        m_ptrSegmentRelationFilter(new SegmentRelationFilter()),
        m_param_writeSuperClusters(false),
        m_param_superClustersStoreObjName("CDCWireHitSuperClusterVector")
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
      }

      /// Initialize the Module before event processing
      virtual void initialize() override
      {
        Super::initialize();

        if (m_param_writeSuperClusters) {
          StoreWrappedObjPtr< std::vector<CDCWireHitCluster> >::registerTransient(m_param_superClustersStoreObjName);
        }

        if (m_ptrSegmentRelationFilter) {
          m_ptrSegmentRelationFilter->initialize();
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
        if (m_ptrSegmentRelationFilter) {
          m_ptrSegmentRelationFilter->terminate();
        }

        Super::terminate();
      }

      /// Getter for the current segment relation filter. The module keeps ownership of the pointer.
      SegmentRelationFilter* getSegmentRelationFilter()
      {
        return m_ptrSegmentRelationFilter.get();
      }

      /// Setter for the segment relation filter used to connect segments in a network. The module takes ownership of the pointer.
      void setSegmentRelationFilter(std::unique_ptr<SegmentRelationFilter> ptrSegmentRelationFilter)
      {
        m_ptrSegmentRelationFilter = std::move(ptrSegmentRelationFilter);
      }

    private:
      /// Reference to the relation filter to be used to construct the segment network in each supercluster
      std::unique_ptr<SegmentRelationFilter> m_ptrSegmentRelationFilter;

    private:
      /// Parameter: Switch if superclusters shall be written to the DataStore
      bool m_param_writeSuperClusters;

      /// Parameter: Name of the output StoreObjPtr of the superclusters generated within this module.
      std::string m_param_superClustersStoreObjName;

    private:
      /** Memory for the hit superclusters in the current superlayer
       *  Superclusters generated from the secondary neighborhood
       *  of hits and allow for hit inefficiencies along the trajectory.
       */
      std::vector<CDCWireHitCluster> m_superClustersInSuperLayer;

      /// Neighborhood type for wire hits
      typedef WeightedNeighborhood<CDCWireHit> CDCWireHitNeighborhood;

      /// Memory for the secondary wire hit neighborhood in a super layer.
      CDCWireHitNeighborhood m_secondaryWirehitNeighborhood;

      /// Instance of the hit cluster generator
      Clusterizer<CDCWireHit, CDCWireHitCluster> m_wirehitClusterizer;

      /// Neighborhood type for segments.
      typedef WeightedNeighborhood<const CDCRecoSegment2D> CDCSegmentNeighborhood;

      /// Memory for the segments in each super cluster
      std::vector<std::vector<CDCRecoSegment2D> > m_segmentsByISuperCluster;

      /// Memory for the symmetrised segments in each super cluster
      std::vector<std::vector<CDCRecoSegment2D> > m_symmetricSegmentsByISuperCluster;

    }; // end class SegmentFinderCDCBySuperClusterModule




    template<class SegmentRelationFilter>
    void
    SegmentFinderCDCBySuperClusterModule<SegmentRelationFilter>
    ::generateSegments(std::vector<CDCRecoSegment2D>& segments)
    {
      /// Attain super cluster vector on the DataStore if needed.
      if (m_param_writeSuperClusters) {
        StoreWrappedObjPtr< std::vector<CDCWireHitCluster> > storedSuperClusters(m_param_superClustersStoreObjName);
        storedSuperClusters.create();
      }

      const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
      CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
      std::vector<CDCWireHit> wireHits = wireHitTopology.getWireHits();

      // Event global super cluster id for each super cluster.
      int iSuperCluster = -1;

      m_segmentsByISuperCluster.clear();
      m_symmetricSegmentsByISuperCluster.clear();

      for (const CDCWireSuperLayer& wireSuperLayer : wireTopology.getWireSuperLayers()) {
        VectorRange<CDCWireHit> wireHitsInSuperlayer = std::equal_range(wireHits.begin(), wireHits.end(), wireSuperLayer);

        //create the secondary neighborhood of wire hits
        B2DEBUG(100, "Creating the secondary CDCWireHit neighborhood");
        m_secondaryWirehitNeighborhood.clear();

        const bool withSecondaryNeighborhood = true;
        m_secondaryWirehitNeighborhood.appendUsing<WholeWireHitRelationFilter<withSecondaryNeighborhood>>(wireHitsInSuperlayer);
        assert(m_secondaryWirehitNeighborhood.isSymmetric());

        B2DEBUG(100, "  seconaryWirehitNeighborhood.size() = " << m_secondaryWirehitNeighborhood.size());

        // Create the super clusters
        B2DEBUG(100, "Creating the CDCWireHit super clusters");
        m_superClustersInSuperLayer.clear();
        m_wirehitClusterizer.create(wireHitsInSuperlayer,
                                    m_secondaryWirehitNeighborhood,
                                    m_superClustersInSuperLayer);

        B2DEBUG(100, "Created " << m_superClustersInSuperLayer.size() <<
                " CDCWireHit superclusters in superlayer");

        for (CDCWireHitCluster& superCluster : m_superClustersInSuperLayer) {
          ++iSuperCluster;
          std::sort(std::begin(superCluster), std::end(superCluster));
          assert(std::is_sorted(std::begin(superCluster), std::end(superCluster)));


          m_segmentsByISuperCluster.emplace_back();
          std::vector<CDCRecoSegment2D>& segmentsInSuperCluster = m_segmentsByISuperCluster.back();
          segmentsInSuperCluster.clear();

          generateSegmentsFromSuperCluster(superCluster, segmentsInSuperCluster);

          for (CDCRecoSegment2D& segment : segmentsInSuperCluster) {
            segment.setISuperCluster(iSuperCluster);
          }

          m_symmetricSegmentsByISuperCluster.emplace_back();
          std::vector<CDCRecoSegment2D>& symmetricSegmentsInSuperCluster = m_symmetricSegmentsByISuperCluster.back();
          symmetricSegmentsInSuperCluster.clear();
          segmentsInSuperCluster.reserve(2 * segmentsInSuperCluster.size());

          for (const CDCRecoSegment2D& segment : segmentsInSuperCluster) {
            symmetricSegmentsInSuperCluster.push_back(segment);
            symmetricSegmentsInSuperCluster.push_back(segment.reversed());
          }

          CDCSegmentNeighborhood segmentsNeighborhood;
          segmentsNeighborhood.clear();
          segmentsNeighborhood.createUsing(*m_ptrSegmentRelationFilter,
                                           symmetricSegmentsInSuperCluster);

          MultipassCellularPathFinder<CDCRecoSegment2D> cellularPathFinder;
          std::vector< std::vector<const CDCRecoSegment2D*> > segmentPaths;
          cellularPathFinder.apply(symmetricSegmentsInSuperCluster,
                                   segmentsNeighborhood,
                                   segmentPaths);

          for (const std::vector<const CDCRecoSegment2D*>& segmentPath : segmentPaths) {
            segments.push_back(CDCRecoSegment2D::condense(segmentPath));
          }

        } // end super cluster loop

        // Move clusters to the DataStore
        if (m_param_writeSuperClusters) {
          StoreWrappedObjPtr< std::vector<CDCWireHitCluster> >
          storedSuperClusters(m_param_superClustersStoreObjName);
          std::vector<CDCWireHitCluster>& superClusters = *storedSuperClusters;
          superClusters.insert(superClusters.end(),
                               std::make_move_iterator(m_superClustersInSuperLayer.begin()),
                               std::make_move_iterator(m_superClustersInSuperLayer.end()));
        }

        m_superClustersInSuperLayer.clear();
        m_secondaryWirehitNeighborhood.clear();

      } // end for superlayer loop
    }

  } //end namespace TrackFindingCDC
} //end namespace Belle2
