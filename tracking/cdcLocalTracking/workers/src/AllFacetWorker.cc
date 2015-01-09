/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/AllFacetWorker.h"

#include <tracking/cdcLocalTracking/creators/FacetCreator.h>
#include <tracking/cdcLocalTracking/algorithms/Clusterizer.h>

#include <tracking/cdcLocalTracking/filters/facet/AllFacetFilter.h>
#include <tracking/cdcLocalTracking/filters/wirehit_wirehit/WireHitNeighborChooser.h>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


void AllFacetWorker::generate()
{
  // Clear all facets and regenerate them
  m_recoFacets.clear();

  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  for (const CDCWireSuperLayer & wireSuperLayer : wireTopology.getWireSuperLayers()) {

    // Examine only the hit in the current superlayer
    const CDCWireHitTopology::CDCWireHitRange wireHitsInSuperlayer = wireHitTopology.getWireHits(wireSuperLayer);

    // Create the neighborhood
    B2DEBUG(100, "Creating the CDCWireHit neighborhood");
    WeightedNeighborhood<const CDCWireHit> m_wirehitNeighborhood;

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

    // Security check if the neighbor of hits is symmetric
    bool isSymmetric = m_wirehitNeighborhood.isSymmetric();
    B2DEBUG(100, "  Check symmetry " << isSymmetric);
    B2DEBUG(100, "  allWirehitNeighbors.size() = " << m_wirehitNeighborhood.size());

    if (not isSymmetric) {
      B2WARNING("  The wire neighborhood is not symmetric");
    }

    // Create the clusters
    B2DEBUG(100, "Creating the CDCWireHit clusters");
    std::vector<CDCWireHitCluster> m_clustersInSuperLayer;
    m_clustersInSuperLayer.reserve(20);

    Clusterizer<CDCWireHit, CDCWireHitCluster> m_wirehitClusterizer;
    m_wirehitClusterizer.create(wireHitsInSuperlayer, m_wirehitNeighborhood, m_clustersInSuperLayer);

    B2DEBUG(100, "Created " << m_clustersInSuperLayer.size() << " CDCWireHit clusters in superlayer");


    for (CDCWireHitCluster & cluster : m_clustersInSuperLayer) {

      B2DEBUG(100, "Cluster size: " << cluster.size());
      B2DEBUG(100, "Wire hit neighborhood size: " << m_wirehitNeighborhood.size());

      // Create the facets
      B2DEBUG(100, "Creating the CDCRecoFacets");

      FacetCreator<AllFacetFilter> m_allFacetCreator;
      m_allFacetCreator.createFacets(cluster, m_wirehitNeighborhood, m_recoFacets);

      B2DEBUG(100, "  Created " << m_recoFacets.size()  << " CDCRecoFacets");

    } // end for cluster
  } // end for superlayer


  if (m_copyToDataStoreForDebug) {
    copyToDataStoreForDebug();
  }

}





void AllFacetWorker::copyToDataStoreForDebug() const
{

#ifdef CDCLOCALTRACKING_USE_ROOT
  // IO for monitoring in python
  StoreArray < CDCRecoFacet > storedRecoFacets;
  storedRecoFacets.create(true); //Force all content to be cleared.
  for (const CDCRecoFacet & recoFacet : m_recoFacets) {
    storedRecoFacets.appendNew(recoFacet);
  }
#endif

}



