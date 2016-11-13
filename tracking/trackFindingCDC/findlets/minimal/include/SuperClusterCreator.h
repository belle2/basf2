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

#include <tracking/trackFindingCDC/findlets/minimal/ClusterCreator.h>

#include <tracking/trackFindingCDC/filters/wireHitRelation/SecondaryWireHitRelationFilter.h>

#include <vector>

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

    public:
      /// Main algorithm applying the cluster refinement
      void apply(std::vector<CDCWireHit>& inputWireHits,
                 std::vector<CDCWireHitCluster>& outputSuperClusters) final;

    private:
      /// Creator of the super clusters
      ClusterCreator<SecondaryWireHitRelationFilter> m_clusterCreator;
    };
  }
}
