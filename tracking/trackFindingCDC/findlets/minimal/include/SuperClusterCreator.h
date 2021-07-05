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

#include <tracking/trackFindingCDC/ca/Clusterizer.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCWireHit;
    class CDCWireHitCluster;

    /// Refines the clustering of wire hits from  clusters to clusters
    class SuperClusterCreator : public Findlet<CDCWireHit&, CDCWireHitCluster> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit, CDCWireHitCluster>;

    public:
      /// Constructor
      SuperClusterCreator();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    public:
      /// Main algorithm applying the cluster refinement
      void apply(std::vector<CDCWireHit>& inputWireHits,
                 std::vector<CDCWireHitCluster>& outputSuperClusters) final;

    private:
      /// Parameter : Expand the super clusters over the typical gap at the apogee of the trajectory
      bool m_param_expandOverApogeeGap = false;

    private:
      /// Instance of the hit cluster generator
      Clusterizer<CDCWireHit, CDCWireHitCluster> m_wirehitClusterizer;

      /// Memory for the wire hit neighborhood in a cluster.
      std::vector<WeightedRelation<CDCWireHit>> m_wireHitRelations;

      /// Wire hit neighborhood relation filter
      WholeWireHitRelationFilter m_wireHitRelationFilter{2};
    };
  }
}
