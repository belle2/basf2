/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
