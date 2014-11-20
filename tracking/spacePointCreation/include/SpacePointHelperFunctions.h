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
    inline void addCluster(const SVDCluster* entry) {
      vxdID = entry->getSensorID();
      if (entry->isUCluster() == true) { clustersU.push_back(entry); return; }
      clustersV.push_back(entry);
    }

    /** Id of sensor, TODO can be removed if struct is used in a map */
    VxdID vxdID;

    /** stores all SVDclusters of U type.
     *
     * Each entry stores a pointer to its SVDCluster.
     */
    std::vector<const SVDCluster*> clustersU;

    /** stores all SVDclusters of V type.
     *
     * Each entry stores a pointer to its SVDCluster.
     */
    std::vector<const SVDCluster*> clustersV;

  };


  /** store a spacePoint for given Cluster.
   *
   * for SVDClusters use the functions provideSVDClusterSingles(.) or provideSVDClusterCombinations(.) instead.
   *
   * first parameter is a storeArray containing Clusters (e.g. PXD or Tel).
   * second parameter is a storeArra containing a version of spacePoints (will be filled in the function).
   */
  template <class ClusterType, class SpacePointType> inline void storeSingleCluster(
    const StoreArray<ClusterType>& clusters,
    StoreArray<SpacePointType>& spacePoints)
  {
    for (unsigned int i = 0; i < uint(clusters.getEntries()); ++i) {
      const ClusterType* currentCluster = clusters[i];
      SpacePointType* newSP = spacePoints.appendNew((currentCluster));
      newSP->addRelationTo(currentCluster);
    }
  }



  /** simply store one spacePoint for each existing SVDCluster.
   *
   * first parameter is a storeArray containing SVDClusters.
   * second parameter is a storeArra containing SpacePoints (will be filled in the function).
   */
  template <class SpacePointType> void provideSVDClusterSingles(const StoreArray<SVDCluster>& svdClusters,
      StoreArray<SpacePointType>& spacePoints)
  {
    for (unsigned int i = 0; i < uint(svdClusters.getEntries()); ++i) {
      const SVDCluster* currentCluster = svdClusters[i];
      std::vector<const SVDCluster*> currentClusterCombi = { currentCluster };
      SpacePointType* newSP = spacePoints.appendNew(currentClusterCombi);
      newSP->addRelationTo(currentCluster);
    }
  }



  /** stores all possible 2-Cluster-combinations.
   *
   * first parameter is a struct containing all clusters on current sensor.
   * second parameter is the container which collects all combinations found.
   *
   * for each u cluster, a v cluster is combined to a possible combination.
   */
  inline void findPossibleCombinations(const Belle2::ClustersOnSensor& aSensor,
                                       std::vector< std::vector<const SVDCluster*> >& foundCombinations)
  {
    for (const SVDCluster * uCluster : aSensor.clustersU) {
      for (const SVDCluster * vCluster : aSensor.clustersV) {
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
  template <class SpacePointType> void provideSVDClusterCombinations(const StoreArray<SVDCluster>& svdClusters,
      StoreArray<SpacePointType>& spacePoints)
  {
    std::unordered_map<VxdID::baseType, ClustersOnSensor> activatedSensors; // collects one entry per sensor, each entry will contain all Clusters on it TODO: better to use a sorted vector/list?
    std::vector<std::vector<const SVDCluster*> > foundCombinations; // collects all combinations of Clusters which were possible (condition: 1u+1v-Cluster on the same sensor)


    // sort Clusters by sensor. After the loop, each entry of activatedSensors contains all U and V-type clusters on that sensor
    for (unsigned int i = 0; i < uint(svdClusters.getEntries()); ++i) {
      SVDCluster* currentCluster = svdClusters[i];

      activatedSensors[currentCluster->getSensorID().getID()].addCluster(currentCluster);
    }


    for (auto & aSensor : activatedSensors) {
      findPossibleCombinations(aSensor.second, foundCombinations);
    }


    for (auto & clusterCombi : foundCombinations) {
      SpacePointType* newSP = spacePoints.appendNew(clusterCombi);
      for (auto * cluster : clusterCombi) {
        newSP->addRelationTo(cluster);
      }
    }
  }

} //Belle2 namespace
