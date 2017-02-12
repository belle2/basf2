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

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>


#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Cluster.h>
#include <reconstruction/dataobjects/KlId.h>


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
  StoreArray<KLMCluster>::required();
  StoreArray<ECLCluster>::required();

  StoreArray<ECLCluster> eclClusters;
  StoreArray<KLMCluster> klmClusters;
  eclClusters.registerRelationTo(klmClusters);

  StoreArray<Cluster>::registerPersistent();//Transient
  StoreArray<Cluster> Clusters;
  klmClusters.registerRelationTo(Clusters);
  eclClusters.registerRelationTo(Clusters);

}//init


void ClusterMatcherModule::event()
{
  StoreArray<ECLCluster> eclClusters;
  StoreArray<KLMCluster> klmClusters;
  StoreArray<Cluster> Clusters;

  float angleDist;

  for (const ECLCluster& eclCluster : eclClusters) {

    const TVector3& eclClusterPos = eclCluster.getClusterPosition();


    Cluster* clusterecl = Clusters.appendNew();
    clusterecl->setLogLikelihood(
      Const::ECL,
      Const::clusterKlong,
      eclCluster.getRelatedTo<KlId>()->getKlId()
    );
    eclCluster.addRelationTo(clusterecl);

    for (KLMCluster& klmcluster : klmClusters) {

      const TVector3& klmClusterPos = klmcluster.getClusterPosition();

      angleDist = eclClusterPos.Angle(klmClusterPos);

      if (angleDist < (m_coneInRad / 2.0)) {

        eclCluster.addRelationTo(&klmcluster, angleDist);
        klmcluster.addRelationTo(clusterecl);
      } else {
        Cluster* clusterklm = Clusters.appendNew();
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


