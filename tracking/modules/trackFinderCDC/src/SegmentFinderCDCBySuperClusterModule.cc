/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/SegmentFinderCDCBySuperClusterModule.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


void SegmentFinderCDCBySuperClusterModule::generateSegments(std::vector<CDCRecoSegment2D>& segments)
{
  /// Attain super cluster vector on the DataStore if needed.
  if (m_param_writeSuperClusters) {
    StoreWrappedObjPtr< std::vector<CDCWireHitCluster> > storedSuperClusters(m_param_superClustersStoreObjName);
    storedSuperClusters.create();
  }

  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  // Event global super cluster id for each super cluster.
  int iSuperCluster = -1;

  for (const CDCWireSuperLayer& wireSuperLayer : wireTopology.getWireSuperLayers()) {

    const CDCWireHitTopology::CDCWireHitRange wireHitsInSuperlayer = wireHitTopology.getWireHits(wireSuperLayer);

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

      generateSegmentsFromSuperCluster(superCluster, segments);

      //segments.back().setISuperCluster(iSuperCluster);

      // Combine matching segments


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


void
SegmentFinderCDCBySuperClusterModule
::generateSegmentsFromSuperCluster(const CDCWireHitCluster&,
                                   std::vector<CDCRecoSegment2D>&)
{
}
