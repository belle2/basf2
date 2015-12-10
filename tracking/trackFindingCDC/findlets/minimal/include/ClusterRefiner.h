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

#include <tracking/trackFindingCDC/ca/Clusterizer.h>
#include <tracking/trackFindingCDC/filters/wirehit_relation/PrimaryWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>
#include <iterator>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Refines the clustering of wire hits from super clusters to clusters
    template<class WireHitRelationFilter = PrimaryWireHitRelationFilter>
    class ClusterRefiner:
      public Findlet<const CDCWireHitCluster, CDCWireHitCluster> {

    private:
      /// Type of the base class
      typedef Findlet<const CDCWireHitCluster, CDCWireHitCluster> Super;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      ClusterRefiner()
      {
        addSubordinaryProcessingSignalListener(&m_wireHitRelationFilter);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Breaks bigger wire hit super cluster into smaller wire hit clusters";
      }

    public:
      /// Main algorithm applying the cluster refinement
      virtual void apply(const std::vector<CDCWireHitCluster>& inputSuperClusters,
                         std::vector<CDCWireHitCluster>& outputClusters) override final
      {
        for (const CDCWireHitCluster& superCluster : inputSuperClusters) {
          B2ASSERT("Expect the clusters to be sorted", std::is_sorted(superCluster.begin(),
                                                                      superCluster.end()));
          m_wirehitNeighborhood.clear();
          m_wirehitNeighborhood.appendUsing(m_wireHitRelationFilter, superCluster);

          const std::size_t nClustersBefore = outputClusters.size();
          m_wirehitClusterizer.createFromPointers(superCluster, m_wirehitNeighborhood, outputClusters);
          const std::size_t nClustersAfter = outputClusters.size();

          // Update the super cluster id of the just created clusters
          int iSuperCluster = superCluster.getISuperCluster();
          for (std::size_t iCluster = nClustersBefore; iCluster < nClustersAfter; ++iCluster) {
            CDCWireHitCluster& cluster = outputClusters[iCluster];
            cluster.setISuperCluster(iSuperCluster);
            std::sort(cluster.begin(), cluster.end());
          }
        }
      }

    private:
      /// Instance of the hit cluster generator
      Clusterizer<CDCWireHit, CDCWireHitCluster> m_wirehitClusterizer;

      /// Memory for the wire hit neighborhood in a super cluster.
      WeightedNeighborhood<CDCWireHit> m_wirehitNeighborhood;

      /// Wire hit neighborhood relation filter
      WireHitRelationFilter m_wireHitRelationFilter;

    }; // end class

  } // end namespace TrackFindingCDC
} // end namespace Belle2
