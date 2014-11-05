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

#include <testbeam/vxd/tracking/spacePointCreation/TBSpacePoint.h>

#include <string>



namespace Belle2 {
  /**
   * Imports Clusters of the silicon detectors and converts them to spacePoints.
   *
   */
  class SpacePointCreatorTELModule : public Module {

  public:


    /** Constructor */
    SpacePointCreatorTELModule();



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
    std::string m_telClustersName; /**< TelCluster collection name */

    StoreArray<TelCluster> m_telClusters; /**< the storeArray for telClusters as member, is faster than recreating link for each event */

    std::string m_spacePointsName; /**< SpacePoints collection name */

    StoreArray<TBSpacePoint> m_spacePoints; /**< the storeArray for TBspacePoints as member, is faster than recreating link for each event */


    // modification parameters
    std::string m_nameOfInstance; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */


    //counters for testing
    unsigned int m_TESTERTelClusterCtr; /**< counts total number of SVDCluster occured */

    unsigned int m_TESTERSpacePointCtr; /**< counts total number of SpacePoints occured */
  };
} // end namespace Belle2
