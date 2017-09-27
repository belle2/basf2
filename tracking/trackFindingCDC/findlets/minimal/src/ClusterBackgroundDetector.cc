/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/ClusterBackgroundDetector.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <vector>
#include <string>

using namespace Belle2;
using namespace TrackFindingCDC;

ClusterBackgroundDetector::ClusterBackgroundDetector()
{
  this->addProcessingSignalListener(&m_clusterFilter);
}

std::string ClusterBackgroundDetector::getDescription()
{
  return "Marks clusters as background if the used filter detectes them as such";
}

void ClusterBackgroundDetector::exposeParameters(ModuleParamList* moduleParamList,
                                                 const std::string& prefix)
{
  m_clusterFilter.exposeParameters(moduleParamList, prefix);
}

void ClusterBackgroundDetector::apply(std::vector<CDCWireHitCluster>& outputClusters)
{
  for (CDCWireHitCluster& cluster : outputClusters) {
    Weight clusterWeight = m_clusterFilter(cluster);
    if (std::isnan(clusterWeight)) {
      // Cluster detected as background
      cluster.setBackgroundFlag(true);
      for (const CDCWireHit* wireHit : cluster) {
        wireHit->getAutomatonCell().setBackgroundFlag();
        wireHit->getAutomatonCell().setTakenFlag();
      }
    }
  }
}
