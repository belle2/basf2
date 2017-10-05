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
#include <framework/datastore/StoreObjPtr.h>

#include <vxd/dataobjects/VxdID.h>

#include <tracking/spacePointCreation/SpacePoint.h>

#include <string>



namespace Belle2 {
  /**
   * Imports Clusters of the pxd detector and converts them to spacePoints.
   *
   */
  class PXDSpacePointCreatorModule : public Module {

  public:



    /** Constructor */
    PXDSpacePointCreatorModule();



    /** Init the module.
    *
    * prepares all store- and relationArrays.
    */
    virtual void initialize();


    /** eventWise jobs (e.g. storing spacepoints */
    virtual void event();


    /** final output with mini-feedback */
    virtual void terminate();


    /** initialize variables in constructor to avoid nondeterministic behavior */
    void InitializeCounters();


  protected:


    // Data members
    std::string m_pxdClustersName; /**< PXDCluster collection name */

    StoreArray<PXDCluster>
    m_pxdClusters; /**< the storeArray for pxdClusters as member, is faster than recreating link for each event */

    std::string m_spacePointsName; /**< SpacePoints collection name */

    StoreArray<SpacePoint>
    m_spacePoints; /**< the storeArray for spacePoints as member, is faster than recreating link for each event */


    // modification parameters
    std::string
    m_nameOfInstance; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */


    //counters for testing
    unsigned int m_TESTERPXDClusterCtr; /**< counts total number of PXDClusters occured */

    unsigned int m_TESTERSpacePointCtr; /**< counts total number of SpacePoints occured */
  };
} // end namespace Belle2
