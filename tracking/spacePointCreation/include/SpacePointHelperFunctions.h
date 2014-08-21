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
#include <framework/datastore/StoreObjPtr.h>

#include <vxd/dataobjects/VxdID.h>

#include <tracking/spacePointCreation/SpacePoint.h>

#include <unordered_map>


namespace Belle2 {


  /** small struct for storing all clusters of the same sensor in one container.
   *
   * members should only be filled using the single addCluster-function described below.
   */
  struct ClustersOnSensor {

  public:

    /** member function to automatically add the cluster to its corresponding entry */
    inline void addCluster(SpacePoint::SVDClusterInformation& entry) {
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


  /** store a spacePoint for given Cluster.
   *
   * for SVDClusters use the functions provideSVDClusterSingles(.) or provideSVDClusterCombinations(.) instead.
   *
   * first parameter is a storeArray containing Clusters (e.g. PXD or Tel).
   * second parameter is a storeArra containing a version of spacePoints (will be filled in the function).
   * third parameter tels the spacePoint where to get the name of the storeArray containing the related clusters
   */
  template <class clusterTempl, class spacePointTempl> inline void storeSingleCluster(
    const StoreArray<clusterTempl>& clusters,
    StoreArray<spacePointTempl>& spacePoints,
    unsigned short clustersIndex)
  {
    for (unsigned int i = 0; i < uint(clusters.getEntries()); ++i) {
      spacePointTempl* newSP = spacePoints.appendNew((clusters[i]), i, clustersIndex);
      newSP->addRelationTo(clusters[i]);
    }
  }



  /** simply store one spacePoint for each existing SVDCluster.
   *
   * first parameter is a storeArray containing SVDClusters.
   * second parameter is a storeArra containing SpacePoints (will be filled in the function).
   * third parameter tels the spacePoint where to get the name of the storeArray containing the related clusters
   *
   */
  template <class spTemplate> void provideSVDClusterSingles(const StoreArray<SVDCluster>& svdClusters,
                                                            StoreArray<spTemplate>& spacePoints,
                                                            unsigned short svdClustersIndex)
  {
    for (unsigned int i = 0; i < uint(svdClusters.getEntries()); ++i) {
      SpacePoint::SVDClusterInformation currentCluster = {svdClusters[i], i};
      std::vector<SpacePoint::SVDClusterInformation> currentClusterCombi = { currentCluster };
      spTemplate* newSP = spacePoints.appendNew(currentClusterCombi, svdClustersIndex);
      newSP->addRelationTo(svdClusters[i]);
    }
  }



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



  /** finds all possible combinations of U and V Clusters for SVDClusters.
   *
   * first parameter is a storeArray containing SVDClusters.
   * second parameter is a storeArra containing SpacePoints (will be filled in the function).
   * third parameter tels the spacePoint where to get the name of the storeArray containing the related clusters
   */
  template <class spTemplate> void provideSVDClusterCombinations(const StoreArray<SVDCluster>& svdClusters,
      StoreArray<spTemplate>& spacePoints,
      unsigned short svdClustersIndex)
  {
    std::unordered_map<VxdID::baseType, ClustersOnSensor> activatedSensors; // collects one entry per sensor, each entry will contain all Clusters on it TODO: better to use a sorted vector/list?
    std::vector<std::vector<SpacePoint::SVDClusterInformation> > foundCombinations; // collects all combinations of Clusters which were possible (condition: 1u+1v-Cluster on the same sensor)


    // sort Clusters by sensor. After the loop, each entry of activatedSensors contains all U and V-type clusters on that sensor
    for (unsigned int i = 0; i < uint(svdClusters.getEntries()); ++i) {
      SpacePoint::SVDClusterInformation currentCluster = {svdClusters[i], i};

      activatedSensors[svdClusters[i]->getSensorID().getID()].addCluster(currentCluster);
    }


    for (auto & aSensor : activatedSensors) {
      findPossibleCombinations(aSensor.second, foundCombinations);
    }


    for (auto & clusterCombi : foundCombinations) {
      spTemplate* newSP = spacePoints.appendNew(clusterCombi, svdClustersIndex);
      for (auto & cluster : clusterCombi) {
        newSP->addRelationTo(cluster.first);
      }
    }
  }

} //Belle2 namespace
