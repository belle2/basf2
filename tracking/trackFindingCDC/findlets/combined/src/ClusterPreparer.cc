/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/ClusterPreparer.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

using namespace Belle2;
using namespace TrackFindingCDC;

ClusterPreparer::ClusterPreparer()
{
  this->addProcessingSignalListener(&m_superClusterCreator);
  this->addProcessingSignalListener(&m_clusterRefiner);
  this->addProcessingSignalListener(&m_clusterBackgroundDetector);
}

std::string ClusterPreparer::getDescription()
{
  return "Clusters the wire hits in the CDC to form locally connected groups with two granularities.";
}

void ClusterPreparer::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_superClusterCreator.exposeParameters(moduleParamList, prefixed(prefix, "SuperCluster"));
  m_clusterRefiner.exposeParameters(moduleParamList, prefix);
  m_clusterBackgroundDetector.exposeParameters(moduleParamList, prefixed(prefix, "Cluster"));
}

void ClusterPreparer::apply(std::vector<CDCWireHit>& inputWireHits,
                            std::vector<CDCWireHitCluster>& clusters,
                            std::vector<CDCWireHitCluster>& superClusters)
{
  clusters.reserve(100);
  superClusters.reserve(50);

  m_superClusterCreator.apply(inputWireHits, superClusters);
  m_clusterRefiner.apply(superClusters, clusters);
  m_clusterBackgroundDetector.apply(clusters);
}
