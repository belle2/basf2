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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/filters/wireHitRelation/BridgingWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/ca/Clusterizer.h>
#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>
#include <tracking/trackFindingCDC/ca/WeightedRelation.h>

#include <boost/range/adaptor/indirected.hpp>

#include <vector>
#include <string>
#include <algorithm>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Refines the clustering of wire hits from super clusters to clustexrs
    template <class AWireHitRelationFilter = BridgingWireHitRelationFilter>
    class ClusterRefiner : public Findlet<const CDCWireHitCluster, CDCWireHitCluster> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHitCluster, CDCWireHitCluster>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      ClusterRefiner()
      {
        this->addProcessingSignalListener(&m_wireHitRelationFilter);
      }

      /// Short description of the findlet
      std::string getDescription() final {
        return "Breaks bigger wire hit super cluster into smaller wire hit clusters";
      }

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final {
        m_wireHitRelationFilter.exposeParameters(moduleParamList, prefix);
      }

    public:
      /// Main algorithm applying the cluster refinement
      void apply(const std::vector<CDCWireHitCluster>& inputSuperClusters,
                 std::vector<CDCWireHitCluster>& outputClusters) final {
        for (const CDCWireHitCluster& superCluster : inputSuperClusters)
        {
          B2ASSERT("Expect the clusters to be sorted", std::is_sorted(superCluster.begin(),
          superCluster.end()));

          m_wireHitRelations.clear();
          auto wireHits = superCluster | boost::adaptors::indirected;
          WeightedNeighborhood<CDCWireHit>::appendUsing(m_wireHitRelationFilter, wireHits, m_wireHitRelations);
          WeightedNeighborhood<CDCWireHit> wireHitNeighborhood(m_wireHitRelations);

          const std::size_t nClustersBefore = outputClusters.size();
          m_wireHitClusterizer.createFromPointers(superCluster, wireHitNeighborhood, outputClusters);
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
      Clusterizer<CDCWireHit, CDCWireHitCluster> m_wireHitClusterizer;

      /// Memory for the wire hit neighborhood in a super cluster.
      std::vector<WeightedRelation<CDCWireHit> > m_wireHitRelations;

      /// Wire hit neighborhood relation filter
      AWireHitRelationFilter m_wireHitRelationFilter;
    };
  }
}
