/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/filters/wireHitRelation/WholeWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>

#include <tracking/trackingUtilities/ca/Clusterizer.h>

#include <tracking/trackingUtilities/filters/base/RelationFilterUtil.h>

#include <tracking/trackingUtilities/utilities/Algorithms.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCWireHit;
  }

  namespace TrackFindingCDC {

    /// Refines the clustering of wire hits from  clusters to clusters
    template <class AWireHitRelationFilter = WholeWireHitRelationFilter>
    class ClusterCreator : public TrackingUtilities::Findlet<TrackingUtilities::CDCWireHit&, CDCWireHitCluster> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<TrackingUtilities::CDCWireHit, CDCWireHitCluster>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      ClusterCreator()
      {
        this->addProcessingSignalListener(&m_wireHitRelationFilter);
      }

      /// Short description of the findlet
      std::string getDescription() final
      {
        return "Groups the wire hits into super by expanding the given wire hit relation";
      }

    public:
      /// Main algorithm applying the cluster refinement
      void apply(std::vector<TrackingUtilities::CDCWireHit>& inputWireHits,
                 std::vector<CDCWireHitCluster>& outputClusters) final
      {

        // Obtain the wire hits as pointers.
        const std::vector<TrackingUtilities::CDCWireHit*> wireHitPtrs = TrackingUtilities::as_pointers<TrackingUtilities::CDCWireHit>
            (inputWireHits);

        // create the neighborhood
        m_wireHitRelations.clear();
        TrackingUtilities::RelationFilterUtil::appendUsing(m_wireHitRelationFilter, wireHitPtrs, m_wireHitRelations);

        B2ASSERT("Expect wire hit neighborhood to be symmetric ",
                 TrackingUtilities::WeightedRelationUtil<TrackingUtilities::CDCWireHit>::areSymmetric(m_wireHitRelations));

        // Compose the clusters
        m_wirehitClusterizer.apply(wireHitPtrs, m_wireHitRelations, outputClusters);
      }

    private:
      /// Instance of the hit cluster generator
      TrackingUtilities::Clusterizer<TrackingUtilities::CDCWireHit, CDCWireHitCluster> m_wirehitClusterizer;

      /// Memory for the wire hit neighborhood in a cluster.
      std::vector<TrackingUtilities::WeightedRelation<TrackingUtilities::CDCWireHit>> m_wireHitRelations;

      /// Wire hit neighborhood relation filter
      AWireHitRelationFilter m_wireHitRelationFilter;
    };
  }
}
