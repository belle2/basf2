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

#include <tracking/trackFindingCDC/algorithms/MultipassCellularPathFinder.h>
#include <tracking/trackFindingCDC/algorithms/Clusterizer.h>
#include <tracking/trackFindingCDC/filters/wirehit_relation/WholeWireHitRelationFilter.h>


#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/eventdata/CDCEventData.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/basemodules/SegmentFinderCDCBaseModule.h>

#include <vector>
#include <iterator>
#include <assert.h>

namespace Belle2 {

  namespace TrackFindingCDC {
    class SegmentFinderCDCBySuperClusterModule : public SegmentFinderCDCBaseModule {

    private:
      typedef SegmentFinderCDCBaseModule Super;

    public:
      /// Default constructor initialising the filters with the default settings
      SegmentFinderCDCBySuperClusterModule(ETrackOrientation segmentOrientation = c_None) :
        SegmentFinderCDCBaseModule(segmentOrientation),
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

      }

      /// Generates the segment.
      virtual void generateSegments(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments) override final;


      /// Generates the segment in the given super cluster of hits
      virtual void generateSegmentsFromSuperCluster(const CDCWireHitCluster& superCluster,
                                                    std::vector<CDCRecoSegment2D>& segments);

      virtual void terminate() override
      {
        Super::terminate();
      }

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
      typedef WeightedNeighborhood<const CDCWireHit> CDCWireHitNeighborhood;

      /// Memory for the secondary wire hit neighborhood in a super layer.
      CDCWireHitNeighborhood m_secondaryWirehitNeighborhood;

      /// Instance of the hit cluster generator
      Clusterizer<CDCWireHit, CDCWireHitCluster> m_wirehitClusterizer;

    }; // end class SegmentFinderCDCBySuperClusterModule


  } //end namespace TrackFindingCDC
} //end namespace Belle2
