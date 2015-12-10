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
#include <tracking/trackFindingCDC/filters/wirehit_relation/WholeWireHitRelationFilter.h>

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
      /// Signals the beginning of the event processing
      void initialize() override
      {
        Super::initialize();
        m_wireHitRelationFilter.initialize();
      }

      /// Signals the beginning of a new event
      void beginEvent() override
      {
        Super::beginEvent();
        m_wireHitRelationFilter.beginEvent();
      }

      /// Signals the end of the event processing
      void terminate() override
      {
        m_wireHitRelationFilter.terminate();
        Super::terminate();
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

        m_wirehitClusterizer.createFromPointers(inputWireHits, m_wirehitNeighborhood, outputClusters);
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
