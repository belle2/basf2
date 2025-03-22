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

#include <tracking/trackingUtilities/ca/Clusterizer.h>
#include <tracking/trackingUtilities/utilities/WeightedRelation.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackingUtilities {
    class CDCWireHit;
  }

  namespace TrackFindingCDC {
    class CDCWireHitCluster;

    /// Refines the clustering of wire hits from  clusters to clusters
    class SuperClusterCreator : public TrackingUtilities::Findlet<TrackingUtilities::CDCWireHit&, CDCWireHitCluster> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<TrackingUtilities::CDCWireHit, CDCWireHitCluster>;

    public:
      /// Constructor
      SuperClusterCreator();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    public:
      /// Main algorithm applying the cluster refinement
      void apply(std::vector<TrackingUtilities::CDCWireHit>& inputWireHits,
                 std::vector<CDCWireHitCluster>& outputSuperClusters) final;

    private:
      /// Parameter : Expand the super clusters over the typical gap at the apogee of the trajectory
      bool m_param_expandOverApogeeGap = false;

    private:
      /// Instance of the hit cluster generator
      TrackingUtilities::Clusterizer<TrackingUtilities::CDCWireHit, CDCWireHitCluster> m_wirehitClusterizer;

      /// Memory for the wire hit neighborhood in a cluster.
      std::vector<TrackingUtilities::WeightedRelation<TrackingUtilities::CDCWireHit>> m_wireHitRelations;

      /// Wire hit neighborhood relation filter
      WholeWireHitRelationFilter m_wireHitRelationFilter{2};
    };
  }
}
