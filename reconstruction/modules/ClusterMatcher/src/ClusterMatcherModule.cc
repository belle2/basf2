/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * **************************************************************************/
#include <reconstruction/modules/ClusterMatcher/ClusterMatcherModule.h>

#include <framework/gearbox/Const.h>


#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Cluster.h>
#include <mdst/dataobjects/KlId.h>


using namespace Belle2;
using namespace std;


REG_MODULE(ClusterMatcher);

ClusterMatcherModule::ClusterMatcherModule(): Module() // constructor kan nkeine argumente nehmen
{
  setDescription("Match KLM cluster to ECL Clusters within a certain cone.");

  setPropertyFlags(c_ParallelProcessingCertified);
  /** Path were to find the .xml file containing the classifier trainings. */
  addParam("coneInRad",
           m_coneInRad,
           "Cone angle in rad, will be devided by 2 for the matching",
           m_coneInRad);
}

ClusterMatcherModule::~ClusterMatcherModule()
{
}

void ClusterMatcherModule::initialize()
{
  // require existence of necessary datastore obj
  m_klmClusters.isRequired();
  m_eclClusters.isRequired();
  m_eclClusters.registerRelationTo(m_klmClusters);

  m_Clusters.registerInDataStore();
  m_klmClusters.registerRelationTo(m_Clusters);
  m_eclClusters.registerRelationTo(m_Clusters);

}//init


void ClusterMatcherModule::event()
{
  float angleDist;

  for (const ECLCluster& eclCluster : m_eclClusters) {

    const TVector3& eclClusterPos = eclCluster.getClusterPosition();

    Cluster* clusterecl = m_Clusters.appendNew();
    //Once available we will have to set ECL likelihoods here as is done for KLM
    eclCluster.addRelationTo(clusterecl);

    for (KLMCluster& klmcluster : m_klmClusters) {

      const TVector3& klmClusterPos = klmcluster.getClusterPosition();

      angleDist = eclClusterPos.Angle(klmClusterPos);

      if (angleDist < (m_coneInRad / 2.0)) {

        eclCluster.addRelationTo(&klmcluster, angleDist);
        klmcluster.addRelationTo(clusterecl);
      } else {
        Cluster* clusterklm = m_Clusters.appendNew();
        clusterklm->setLogLikelihood(
          Const::KLM,
          Const::clusterKlong,
          klmcluster.getRelatedTo<KlId>()->getKlId()
        );
        klmcluster.addRelationTo(clusterklm);
      }
    }//klmcluster loop


  }// for ecl cluster in clusters
} // event


