/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>

#include <framework/datastore/StoreArray.h>

#include <vxd/dataobjects/VxdID.h>

#include <tracking/spacePointCreation/SpacePoint.h>



namespace Belle2 {


  /** small struct for storing all clusters of the same sensor in one container.
   *
   * members should only be filled using the single addCluster-function described below.
   */
  struct ClustersOnSensor {

  public:

    /** member function to automatically add the cluster to its corresponding entry */
    void addCluster(SpacePoint::SVDClusterInformation& entry) {
      vxdID = entry.first->getSensorID();
      if (entry.first->isUCluster() == true) { clustersU.push_back(entry); return; }
      clustersV.push_back(entry);
    }

    /** Id of sensor, TODO can be removed if struct is used in a map */
    VxdID vxdID;

    /**< stores all SVDclusters of U type.
     *
     * Each entry stores a pointer to its SVDCluster (.first) and its index number in the StoreArray.
     */
    std::vector<SpacePoint::SVDClusterInformation> clustersU;

    /**< stores all SVDclusters of V type.
     *
     * Each entry stores a pointer to its SVDCluster (.first) and its index number in the StoreArray.
     */
    std::vector<SpacePoint::SVDClusterInformation> clustersV;

  };


  /** simply store one spacePoint for each existing SVDCluster.
   *
   * first parameter is a storeArray containing SVDClusters.
   * second parameter is a storeArra containing SpacePoints (will be filled in the function).
   * third parameter tels the spacePoint where to get the name of the storeArray containing the related clusters
   *
   */
  void provideSVDClusterSingles(const StoreArray<SVDCluster>& svdClusters,
                                StoreArray<SpacePoint>& spacePoints,
                                unsigned short svdClustersIndex)
  {
    for (unsigned int i = 0; i < uint(svdClusters.getEntries()); ++i) {
      SpacePoint::SVDClusterInformation currentCluster = {svdClusters[i], i};
      std::vector<SpacePoint::SVDClusterInformation> currentClusterCombi = { currentCluster };
      spacePoints.appendNew(currentClusterCombi, svdClustersIndex);
      spacePoints[spacePoints.getEntries() - 1]->addRelationTo(svdClusters[i]);
    }
  }



  /** finds all possible combinations of U and V Clusters for SVDClusters.
   *
   * first parameter is a storeArray containing SVDClusters.
   * second parameter is a storeArra containing SpacePoints (will be filled in the function).
   * third parameter tels the spacePoint where to get the name of the storeArray containing the related clusters
   */
  void provideSVDClusterCombinations(const StoreArray<SVDCluster>& svdClusters,
                                     StoreArray<SpacePoint>& spacePoints,
                                     unsigned short svdClustersIndex);



  /** stores all possible 2-Cluster-combinations.
   *
   * first parameter is a struct containing all clusters on current sensor.
   * second parameter is the container which collects all combinations found.
   *
   * for each u cluster, a v cluster is combined to a possible combination.
   */
  inline void findPossibleCombinations(const ClustersOnSensor& aSensor,
                                       std::vector<std::vector<SpacePoint::SVDClusterInformation> >& foundCombinations)
  {
    for (const SpacePoint::SVDClusterInformation & uCluster : aSensor.clustersU) {
      for (const SpacePoint::SVDClusterInformation & vCluster : aSensor.clustersV) {
        foundCombinations.push_back({uCluster, vCluster});
      }
    }
  }
} //Belle2 namespace
