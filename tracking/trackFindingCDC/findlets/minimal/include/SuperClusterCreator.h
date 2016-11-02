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
#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>

#include <vector>
#include <iterator>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Refines the clustering of wire hits from  clusters to clusters
    class SuperClusterCreator : public Findlet<CDCWireHit&, CDCWireHitCluster> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit, CDCWireHitCluster>;

    public:
      /// Constructor
      SuperClusterCreator()
      {
        addProcessingSignalListener(&m_clusterCreator);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Groups the wire hits into super cluster by expanding the secondary wire "
               "neighborhood";
      }

    public:
      /// Main algorithm applying the cluster refinement
      virtual void apply(std::vector<CDCWireHit>& inputWireHits,
                         std::vector<CDCWireHitCluster>& outputSuperClusters) override final
      {
        m_clusterCreator.apply(inputWireHits, outputSuperClusters);

        int iSuperCluster = -1;
        for (CDCWireHitCluster& superCluster : outputSuperClusters) {
          ++iSuperCluster;
          superCluster.setISuperCluster(iSuperCluster);
          for (CDCWireHit* wireHit : superCluster) {
            wireHit->setISuperCluster(iSuperCluster);
          }
          std::sort(superCluster.begin(), superCluster.end());
        }
      }

    private:
      /// Creator of the super clusters
      ClusterCreator<SecondaryWireHitRelationFilter> m_clusterCreator;

    };
  }
}
