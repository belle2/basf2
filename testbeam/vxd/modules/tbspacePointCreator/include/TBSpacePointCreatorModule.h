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

#include <testbeam/vxd/tracking/spacePointCreation/TBSpacePoint.h>

#include <string>



namespace Belle2 {
  /**
   * Imports Clusters of the silicon detectors and converts them to spacePoints.
   *
   */
  class TBSpacePointCreatorModule : public Module {

  public:


    /** Constructor */
    TBSpacePointCreatorModule();



    /** Init the module.
    *
    * prepares all store- and relationArrays.
    */
    virtual void initialize();


    /** eventWise jobs (e.g. storing spacepoints */
    virtual void event();


    /** final output with mini-feedback */
    virtual void terminate();



  protected:


    // Data members
    std::string m_pxdClustersName; /**< PXDCluster collection name */
    std::string m_svdClustersName; /**< SVDCluster collection name */
    std::string m_telClustersName; /**< TelCluster collection name */
    std::string m_spacePointsName; /**< SpacePoints collection name */
    std::string m_relSpacePointsPXDClustersName; /**< SpacePoints <-> PXDClusters relation name */
    std::string m_relSpacePointsSVDClustersName; /**< SpacePoints <-> SVDClusters relation name */
    std::string m_relSpacePointsTelClustersName; /**< SpacePoints <-> TelClusters relation name */


    // modification parameters
    std::string m_nameOfInstance; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */
    bool m_onlySingleClusterSpacePoints; /**< standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more. Does not affect pixel-type Clusters */


    //counters for testing
    unsigned int m_TESTERPXDClusterCtr; /**< counts total number of PXDClusters occured */
    unsigned int m_TESTERSVDClusterCtr; /**< counts total number of SVDCluster occured */
    unsigned int m_TESTERTelClusterCtr; /**< counts total number of SVDCluster occured */
    unsigned int m_TESTERSpacePointCtr; /**< counts total number of SpacePoints occured */


  };
} // end namespace Belle2
