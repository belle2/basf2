/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/spacePointCreation/SpacePointHelperFunctions.h>

#include <unordered_map>

using namespace std;
// using namespace Belle2;

namespace Belle2 {


  /// stores all possible 2-Cluster-combinations
  void provideSVDClusterCombinations(const StoreArray<SVDCluster>& svdClusters,
                                     StoreArray<SpacePoint>& spacePoints,
                                     unsigned short svdClustersIndex)
  {
    unordered_map<VxdID::baseType, ClustersOnSensor> activatedSensors; // collects one entry per sensor, each entry will contain all Clusters on it TODO: better to use a sorted vector/list?
    vector<vector<SpacePoint::SVDClusterInformation> > foundCombinations; // collects all combinations of Clusters which were possible (condition: 1u+1v-Cluster on the same sensor)


    // sort Clusters by sensor. After the loop, each entry of activatedSensors contains all U and V-type clusters on that sensor
    for (unsigned int i = 0; i < uint(svdClusters.getEntries()); ++i) {
      SpacePoint::SVDClusterInformation currentCluster = {svdClusters[i], i};

      activatedSensors[svdClusters[i]->getSensorID().getID()].addCluster(currentCluster);
    }


    for (auto & aSensor : activatedSensors) {
      findPossibleCombinations(aSensor.second, foundCombinations);
    }


    for (auto & clusterCombi : foundCombinations) {
      SpacePoint* newSP = spacePoints.appendNew(clusterCombi, svdClustersIndex);
      for (auto & cluster : clusterCombi) {
        newSP->addRelationTo(cluster.first);
      }
    }
  }

}
