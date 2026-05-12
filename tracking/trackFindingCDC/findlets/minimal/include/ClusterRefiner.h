/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <tracking/trackingUtilities/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>

#include <tracking/trackingUtilities/ca/Clusterizer.h>

#include <tracking/trackingUtilities/filters/base/RelationFilterUtil.h>

#include <tracking/trackingUtilities/utilities/WeightedRelation.h>

#include <vector>
#include <string>
#include <algorithm>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Refines the clustering of wire hits from super clusters to clustexrs
    template <class AWireHitRelationFilter>
    class ClusterRefiner : public
      TrackingUtilities::Findlet<const TrackingUtilities::CDCWireHitCluster, TrackingUtilities::CDCWireHitCluster> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<const TrackingUtilities::CDCWireHitCluster, TrackingUtilities::CDCWireHitCluster>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      ClusterRefiner()
      {
        this->addProcessingSignalListener(&m_wireHitRelationFilter);
      }

      /// Short description of the findlet
      std::string getDescription() final
      {
        return "Breaks bigger wire hit super cluster into smaller wire hit clusters";
      }

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final
      {
        m_wireHitRelationFilter.exposeParameters(moduleParamList, prefix);
      }

    public:
      /// Main algorithm applying the cluster refinement
      void apply(const std::vector<TrackingUtilities::CDCWireHitCluster>& inputSuperClusters,
                 std::vector<TrackingUtilities::CDCWireHitCluster>& outputClusters) final
      {
        for (const TrackingUtilities::CDCWireHitCluster& superCluster : inputSuperClusters) {
          B2ASSERT("Expect the clusters to be sorted", std::is_sorted(superCluster.begin(),
                                                                      superCluster.end()));

          // Obtain the wire hits as pointers
          const std::vector<TrackingUtilities::CDCWireHit*>& wireHitPtrs = superCluster;

          // Create the wire hit relations within the supercluster.
          m_wireHitRelations.clear();
          TrackingUtilities::RelationFilterUtil::appendUsing(m_wireHitRelationFilter, wireHitPtrs, m_wireHitRelations);
          const std::size_t nClustersBefore = outputClusters.size();
          m_wireHitClusterizer.apply(superCluster, m_wireHitRelations, outputClusters);
          const std::size_t nClustersAfter = outputClusters.size();

          // Update the super cluster id of the just created clusters
          int iSuperCluster = superCluster.getISuperCluster();
          for (std::size_t iCluster = nClustersBefore; iCluster < nClustersAfter; ++iCluster) {
            TrackingUtilities::CDCWireHitCluster& cluster = outputClusters[iCluster];
            cluster.setISuperCluster(iSuperCluster);
            std::sort(cluster.begin(), cluster.end());
          }
        }
      }

    private:
      /// Instance of the hit cluster generator
      TrackingUtilities::Clusterizer<TrackingUtilities::CDCWireHit, TrackingUtilities::CDCWireHitCluster> m_wireHitClusterizer;

      /// Memory for the wire hit neighborhood in a super cluster.
      std::vector<TrackingUtilities::WeightedRelation<TrackingUtilities::CDCWireHit>> m_wireHitRelations;

      /// Wire hit neighborhood relation filter
      AWireHitRelationFilter m_wireHitRelationFilter;
    };
  }
}
