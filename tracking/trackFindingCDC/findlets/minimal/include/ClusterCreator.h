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

#include <tracking/trackFindingCDC/filters/wireHitRelation/WholeWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/ca/Clusterizer.h>

#include <tracking/trackFindingCDC/filters/base/RelationFilterUtil.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Refines the clustering of wire hits from  clusters to clusters
    template <class AWireHitRelationFilter = WholeWireHitRelationFilter>
    class ClusterCreator : public Findlet<CDCWireHit&, CDCWireHitCluster> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit, CDCWireHitCluster>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      ClusterCreator()
      {
        this->addProcessingSignalListener(&m_wireHitRelationFilter);
      }

      /// Short description of the findlet
      std::string getDescription() final {
        return "Groups the wire hits into super by expanding the given wire hit relation";
      }

    public:
      /// Main algorithm applying the cluster refinement
      void apply(std::vector<CDCWireHit>& inputWireHits,
                 std::vector<CDCWireHitCluster>& outputClusters) final {

        // Obtain the wire hits as pointers.
        const std::vector<CDCWireHit*> wireHitPtrs = as_pointers<CDCWireHit>(inputWireHits);

        // create the neighborhood
        m_wireHitRelations.clear();
        RelationFilterUtil::appendUsing(m_wireHitRelationFilter, wireHitPtrs, m_wireHitRelations);

        B2ASSERT("Expect wire hit neighborhood to be symmetric ",
        WeightedRelationUtil<CDCWireHit>::areSymmetric(m_wireHitRelations));

        // Compose the clusters
        m_wirehitClusterizer.apply(wireHitPtrs, m_wireHitRelations, outputClusters);
      }

    private:
      /// Instance of the hit cluster generator
      Clusterizer<CDCWireHit, CDCWireHitCluster> m_wirehitClusterizer;

      /// Memory for the wire hit neighborhood in a cluster.
      std::vector<WeightedRelation<CDCWireHit>> m_wireHitRelations;

      /// Wire hit neighborhood relation filter
      AWireHitRelationFilter m_wireHitRelationFilter;
    };
  }
}
