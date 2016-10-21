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

#include <tracking/trackFindingCDC/filters/wireHitRelation/SecondaryWireHitRelationFilter.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterCreator.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>

#include <vector>
#include <iterator>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Refines the clustering of wire hits from  clusters to clusters
    class SuperClusterCreator:
      public Findlet<CDCWireHit, CDCWireHitCluster> {

    private:
      /// Type of the base class
      typedef Findlet<CDCWireHit, CDCWireHitCluster> Super;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Groups the wire hits into super cluster by expanding the secondary wire neighborhood";
      }

      /// Signals the beginning of the event processing
      void initialize() override
      {
        Super::initialize();
        m_clusterCreator.initialize();
      }

      /// Signals the beginning of a new event
      void beginEvent() override
      {
        Super::beginEvent();
        m_clusterCreator.beginEvent();
      }

      /// Signals the end of the event processing
      void terminate() override
      {
        m_clusterCreator.terminate();
        Super::terminate();
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

    }; // end class
  } // end namespace TrackFindingCDC
} // end namespace Belle2
