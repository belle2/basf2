/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <vxd/dataobjects/VxdID.h>

#include <tracking/dataobjects/DATCONSVDSpacePoint.h>
#include <tracking/dataobjects/DATCONSimpleSVDCluster.h>

#include <unordered_map>

#include <type_traits>


namespace Belle2 {


  /** small struct for storing all clusters of the same sensor in one container.
  *
  * members should only be filled using the single addCluster-function described below.
  */
  struct ClustersOnSensor {

  public:

    /** member function to automatically add the cluster to its corresponding entry */
    inline void addCluster(const DATCONSimpleSVDCluster* entry)
    {
      vxdID = entry->getSensorID();
      if (entry->isUCluster() == true) { clustersU.push_back(entry); return; }
      clustersV.push_back(entry);
    }

    /** Id of sensor, TODO can be removed if struct is used in a map */
    VxdID vxdID;

    /** stores all SVDclusters of U type.
    *
    * Each entry stores a pointer to its DATCONSimpleSVDCluster.
    */
    std::vector<const DATCONSimpleSVDCluster*> clustersU;

    /** stores all SVDclusters of V type.
    *
    * Each entry stores a pointer to its DATCONSimpleSVDCluster.
    */
    std::vector<const DATCONSimpleSVDCluster*> clustersV;

  };


  /** simply store one spacePoint for each existing DATCONSimpleSVDCluster.
  *
  * first parameter is a storeArray containing SVDClusters.
  * second parameter is a storeArra containing SpacePoints (will be filled in the function).
  */
  void provideDATCONSVDClusterSingles(const StoreArray<DATCONSimpleSVDCluster>& DATCONSimpleSVDClusters,
                                      StoreArray<DATCONSVDSpacePoint>& spacePoints)
  {
    for (unsigned int i = 0; i < uint(DATCONSimpleSVDClusters.getEntries()); ++i) {
      const DATCONSimpleSVDCluster* currentCluster = DATCONSimpleSVDClusters[i];
      std::vector<const DATCONSimpleSVDCluster*> currentClusterCombi = { currentCluster };
      DATCONSVDSpacePoint* newSP = spacePoints.appendNew(currentClusterCombi);
      newSP->addRelationTo(currentCluster);
    }
  }



  /** stores all possible 2-Cluster-combinations.
  *
  * first parameter is a struct containing all clusters on current sensor.
  * second parameter is the container which collects all combinations found.
  *
  * for each u cluster, a v cluster is combined to a possible combination.
  * Condition which has to be fulfilled: the first entry is always an u cluster, the second always a v-cluster
  */
  inline void findPossibleCombinations(const Belle2::ClustersOnSensor& aSensor,
                                       std::vector< std::vector<const DATCONSimpleSVDCluster*> >& foundCombinations)
  {

    for (const DATCONSimpleSVDCluster* uCluster : aSensor.clustersU) {
      //       if (uCluster->getClsTime() < minClusterTime)
      //         continue;
      for (const DATCONSimpleSVDCluster* vCluster : aSensor.clustersV) {
        //         if (vCluster->getClsTime() < minClusterTime)
        //           continue;
        foundCombinations.push_back({uCluster, vCluster});

      }
    }
  }

  /** finds all possible combinations of U and V Clusters for DATCONSimpleSVDClusters.
  *
  * first parameter is a StoreArray containing DATCONSimpleSVDClusters.
  * second parameter is a StoreArray containing SpacePoints (will be filled in the function).
  * third parameter tells the spacePoint where to get the name of the storeArray containing the related clusters
  * relationweights code the type of the cluster. +1 for u and -1 for v
  */
  void provideDATCONSVDClusterCombinations(const StoreArray<DATCONSimpleSVDCluster>& DATCONSimpleSVDClusters,
                                           StoreArray<DATCONSVDSpacePoint>& spacePoints)
  {
    // collects one entry per sensor, each entry will contain all Clusters on it TODO: better to use a sorted vector/list?
    std::unordered_map<VxdID::baseType, Belle2::ClustersOnSensor> activatedSensors;

    // collects all combinations of Clusters which were possible (condition: 1u+1v-Cluster on the same sensor)
    std::vector<std::vector<const DATCONSimpleSVDCluster*> > foundCombinations;


    // sort Clusters by sensor. After the loop, each entry of activatedSensors contains all U and V-type clusters on that sensor
    for (unsigned int i = 0; i < uint(DATCONSimpleSVDClusters.getEntries()); ++i) {
      DATCONSimpleSVDCluster* currentCluster = DATCONSimpleSVDClusters[i];

      activatedSensors[currentCluster->getSensorID().getID()].addCluster(currentCluster);
    }

    for (auto& aSensor : activatedSensors) {
//       findPossibleCombinations(aSensor.second, foundCombinations, minClusterTime);
      findPossibleCombinations(aSensor.second, foundCombinations);
    }

    for (auto& clusterCombi : foundCombinations) {
      DATCONSVDSpacePoint* newSP = spacePoints.appendNew(clusterCombi);
      for (auto* cluster : clusterCombi) {
        newSP->addRelationTo(cluster, cluster->isUCluster() ? 1. : -1.);

        RelationVector<MCParticle> relatedMC = cluster->getRelationsTo<MCParticle>();
        RelationVector<SVDTrueHit> relatedSVDTrue = cluster->getRelationsTo<SVDTrueHit>();

        // Register relations to the MCParticles and SVDTrueHits
        if (relatedMC.size() > 0) {
          for (unsigned int relmcindex = 0; relmcindex < relatedMC.size(); relmcindex++) {
            newSP->addRelationTo(relatedMC[relmcindex], relatedMC.weight(relmcindex));
          }
        }
        if (relatedSVDTrue.size() > 0) {
          for (unsigned int reltruehitindex = 0; reltruehitindex < relatedSVDTrue.size(); reltruehitindex++) {
            newSP->addRelationTo(relatedSVDTrue[reltruehitindex], relatedSVDTrue.weight(reltruehitindex));
          }
        }

      }

    }
  }

} //Belle2 namespace
