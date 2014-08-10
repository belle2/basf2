/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <vxd/dataobjects/VxdID.h>

#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  /**
   * Imports Clusters of the silicon detectors and converts them to spacePoints.
   *
   */
  class SpacePointCreatorModule : public Module {

  public:


    /** small struct for storing all clusters of the same sensor in one container */
    struct ClustersOnSensor {

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



    /** Constructor */
    SpacePointCreatorModule();



    /** Init the module.
    *
    * prepares all store- and relationArrays.
    */
    virtual void initialize();


    /** eventWise jobs (e.g. storing spacepoints */
    virtual void event();


    /** final output with mini-feedback */
    virtual void terminate();



    /** simply store one spacePoint for each existing SVDCluster.
     *
     * first parameter is a storeArray containing SVDClusters.
     * second parameter is a storeArra containing SpacePoints (will be filled in the function).
     *
     * TODO: currently not used, but will probably be needed for Single-Cluster-SpacePoint-studies, should probably be steered by parameter.
     */
    void provideSVDClusterSingles(const StoreArray<SVDCluster>& svdClusters,
                                  StoreArray<SpacePoint>& spacePoints);



    /** finds all possible combinations of U and V Clusters for SVDClusters.
     *
     * first parameter is a storeArray containing SVDClusters.
     * second parameter is a storeArra containing SpacePoints (will be filled in the function).
     */
    void provideSVDClusterCombinations(const StoreArray<SVDCluster>& svdClusters,
                                       StoreArray<SpacePoint>& spacePoints);



    /** stores all possible 2-Cluster-combinations.
     *
     * first parameter is a struct containing all clusters on current sensor.
     * second parameter is the container which collects all combinations found.
     *
     * for each u cluster, a v cluster is combined to a possible combination.
     */
    void findPossibleCombinations(const ClustersOnSensor& aSensor,
                                  std::vector<std::vector<SpacePoint::SVDClusterInformation> >& foundCombinations) {
      for (const SpacePoint::SVDClusterInformation & uCluster : aSensor.clustersU) {
        for (const SpacePoint::SVDClusterInformation & vCluster : aSensor.clustersV) {
          foundCombinations.push_back({uCluster, vCluster});
        }
      }
    }



  protected:


    // Data members
    std::string m_pxdClustersName; /**< PXDCluster collection name */
    std::string m_svdClustersName; /**< SVDCluster collection name */
    std::string m_spacePointsName; /**< SpacePoints collection name */
    std::string m_relSpacePointsPXDClustersName; /**< SpacePoints <-> PXDClusters relation name */
    std::string m_relSpacePointsSVDClustersName; /**< SpacePoints <-> SVDClusters relation name */


    // modification parameters
    std::string m_nameOfInstance; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */
    bool m_onlySingleClusterSpacePoints; /**< standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more. Does not affect pixel-type Clusters */


    //counters for testing
    unsigned int m_TESTERPXDClusterCtr; /**< counts total number of PXDClusters occured */
    unsigned int m_TESTERSVDClusterCtr; /**< counts total number of SVDCluster occured */
    unsigned int m_TESTERSpacePointCtr; /**< counts total number of SpacePoints occured */


  };
} // end namespace Belle2
