/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/SuperClusterCreator.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

SuperClusterCreator::SuperClusterCreator()
{
  this->addProcessingSignalListener(&m_clusterCreator);
}

std::string SuperClusterCreator::getDescription()
{
  return "Groups the wire hits into super cluster by expanding the secondary wire "
         "neighborhood";
}

void SuperClusterCreator::apply(std::vector<CDCWireHit>& inputWireHits,
                                std::vector<CDCWireHitCluster>& outputSuperClusters)
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
