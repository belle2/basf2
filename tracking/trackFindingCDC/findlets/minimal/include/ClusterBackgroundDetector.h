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

#include <tracking/trackFindingCDC/filters/cluster/BaseClusterFilter.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>
#include <iterator>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Marks clusters as background based on a background measure
    template<class ClusterFilter = BaseClusterFilter>
    class ClusterBackgroundDetector :
      public Findlet<CDCWireHitCluster&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHitCluster&>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      ClusterBackgroundDetector()
      {
        addProcessingSignalListener(&m_clusterFilter);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Marks clusters as background if the used filter detectes them as such";
      }

      /// Expose the parameters of the cluster filter to a module
      virtual void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final
      {
        m_clusterFilter.exposeParameters(moduleParamList, prefix);
      }

      /// Main algorithm applying the cluster background detection
      virtual void apply(std::vector<CDCWireHitCluster>& outputClusters) override final
      {
        for (CDCWireHitCluster& cluster : outputClusters) {
          CellWeight clusterWeight = m_clusterFilter(cluster);
          if (isNotACell(clusterWeight)) {
            // Cluster detected as background
            cluster.setBackgroundFlag(true);
            for (const CDCWireHit* wireHit : cluster) {
              wireHit->getAutomatonCell().setBackgroundFlag();
              wireHit->getAutomatonCell().setTakenFlag();
            }
          }
        }
      }

    private:
      /// Reference to the filter to be used to filter background
      ClusterFilter m_clusterFilter;

    };

  }
}
