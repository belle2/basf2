/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/ClusterMatcher/ClusterMatcherModule.h>

#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Cluster.h>

#include <Math/VectorUtil.h>

using namespace Belle2;

REG_MODULE(ClusterMatcher);

ClusterMatcherModule::ClusterMatcherModule(): Module()
{
  setDescription("Match KLM cluster to ECL Clusters within a certain cone.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("coneInRad",
           m_coneInRad,
           "Cone angle in rad, will be devided by 2 internally for the matching",
           m_coneInRad);
}

void ClusterMatcherModule::initialize()
{
  m_klmClusters.isRequired();
  m_eclClusters.isRequired();
  m_eclClusters.registerRelationTo(m_klmClusters);
  m_Clusters.registerInDataStore();
  m_klmClusters.registerRelationTo(m_Clusters);
  m_eclClusters.registerRelationTo(m_Clusters);
}


void ClusterMatcherModule::event()
{
  for (const ECLCluster& eclCluster : m_eclClusters) {
    const ROOT::Math::XYZVector& eclClusterPos = eclCluster.getClusterPosition();
    const Cluster* clusterecl = m_Clusters.appendNew();
    eclCluster.addRelationTo(clusterecl);
    for (KLMCluster& klmcluster : m_klmClusters) {
      const ROOT::Math::XYZVector& klmClusterPos = klmcluster.getClusterPosition();
      const float angleDist = ROOT::Math::VectorUtil::Angle(eclClusterPos, klmClusterPos);
      if (angleDist < (m_coneInRad / 2.0)) {
        eclCluster.addRelationTo(&klmcluster, angleDist);
        klmcluster.addRelationTo(clusterecl);
      } else {
        const Cluster* clusterklm = m_Clusters.appendNew();
        klmcluster.addRelationTo(clusterklm);
      }
    } // KLMClusters loop
  } // ECLClusters loop
}


