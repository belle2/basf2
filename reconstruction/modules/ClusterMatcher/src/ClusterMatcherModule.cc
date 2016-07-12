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

#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>



using namespace Belle2;
using namespace std;



REG_MODULE(ClusterMatcher);

ClusterMatcherModule::ClusterMatcherModule(): Module() // constructor kan nkeine argumente nehmen
{
  setDescription("Match KLM cluster to ECL Clusters within a certain cone.");
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

}//init

void ClusterMatcherModule::event()
{
  StoreArray<ECLCluster> eclClusters;

  for (const ECLCluster& cluster : eclClusters) {

    matchClusterInCone(cluster, 0.26);// cone angle in rad

  }// for ecl cluster in clusters
} // event


/** find all KLM clusters within specified cone and match them (add a relation)*/
void ClusterMatcherModule::matchClusterInCone(const ECLCluster& eclcluster, float coneInRad)
{

  const TVector3& eclClusterPos = eclcluster.getclusterPosition();
  StoreArray<KLMCluster> klmClusters;
  float angleDist;

  for (KLMCluster& klmcluster : klmClusters) {

    const TVector3& klmClusterPos = klmcluster.getClusterPosition();

    angleDist = eclClusterPos.Angle(klmClusterPos);

    if (angleDist < (coneInRad / 2.0)) {
      // TODO add distance / 2 pi as weight??
      eclcluster.addRelationTo(&klmcluster);
    }
  }
}



