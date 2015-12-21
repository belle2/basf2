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
#include <tracking/trackFindingCDC/filters/wireHitRelation/WholeWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>
#include <iterator>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Refines the clustering of wire hits from  clusters to clusters
    template<class WireHitRelationFilter>
    class ClusterCreator:
      public Findlet<CDCWireHit, CDCWireHitCluster> {

    private:
      /// Type of the base class
      typedef Findlet<CDCWireHit, CDCWireHitCluster> Super;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      ClusterCreator()
      {
        addProcessingSignalListener(&m_wireHitRelationFilter);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Groups the wire hits into super by expanding the given wire hit relation";
      }

    public:
      /// Main algorithm applying the cluster refinement
      virtual void apply(std::vector<CDCWireHit>& inputWireHits,
                         std::vector<CDCWireHitCluster>& outputClusters) override final
      {
        // create the neighborhood
        B2DEBUG(100, "Creating the CDCWireHit neighborhood");
        m_wirehitNeighborhood.clear();
        m_wirehitNeighborhood.appendUsing(m_wireHitRelationFilter, inputWireHits);
        B2ASSERT("Expect wire hit neighborhood to be symmetric ", m_wirehitNeighborhood.isSymmetric());
        B2DEBUG(100, "  wirehitNeighborhood.size() = " << m_wirehitNeighborhood.size());

        m_wirehitClusterizer.create(inputWireHits, m_wirehitNeighborhood, outputClusters);
      }

    private:
      /// Instance of the hit cluster generator
      Clusterizer<CDCWireHit, CDCWireHitCluster> m_wirehitClusterizer;

      /// Memory for the wire hit neighborhood in a cluster.
      WeightedNeighborhood<CDCWireHit> m_wirehitNeighborhood;

      /// Wire hit neighborhood relation filter
      WireHitRelationFilter m_wireHitRelationFilter;

    }; // end class
  } // end namespace TrackFindingCDC
} // end namespace Belle2
