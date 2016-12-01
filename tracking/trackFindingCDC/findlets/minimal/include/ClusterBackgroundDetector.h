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

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Marks clusters as background based on a background measure
    template <class ClusterFilter>
    class ClusterBackgroundDetector : public Findlet<CDCWireHitCluster&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHitCluster&>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      ClusterBackgroundDetector()
      {
        this->addProcessingSignalListener(&m_clusterFilter);
      }

      /// Short description of the findlet
      std::string getDescription() final {
        return "Marks clusters as background if the used filter detectes them as such";
      }

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final {
        m_clusterFilter.exposeParameters(moduleParamList, prefix);
      }

      /// Main algorithm applying the cluster background detection
      void apply(std::vector<CDCWireHitCluster>& outputClusters) final {
        for (CDCWireHitCluster& cluster : outputClusters)
        {
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
