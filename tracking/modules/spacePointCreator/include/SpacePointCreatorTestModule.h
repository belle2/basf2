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
   * Tester module for the validity of the SpacePointCreatorModule.
   *
   * Info Jakob (Aug 24, 2014)
   * TODO: at the moment, the genfit-output can only verified visually
   * (by checking, whether the detector types match the number of dimensions stored in the trackPoint)!
   * when full reco chain is working, this testerModule should be extended!
   * -> verification that input cluster(s) is/are converted to genfit-stuff shall be resilient!
   */
  class SpacePointCreatorTestModule : public Module {

  public:



    /** Constructor */
    SpacePointCreatorTestModule();



    /** Init the module.
    *
    * prepares all store- and relationArrays.
    */
    void initialize() override;


    /** eventWise jobs (e.g. storing spacepoints */
    void event() override;


    /** final output with mini-feedback */
    void terminate() override {}


    /** initialize variables in constructor to avoid nondeterministic behavior */
    void InitializeCounters();


  protected:


    // Data members
    std::string m_pxdClustersName; /**< PXDCluster collection name */

    StoreArray<PXDCluster>
    m_pxdClusters; /**< the storeArray for pxdClusters as member, is faster than recreating link for each event */

    std::string m_svdClustersName; /**< SVDCluster collection name */

    StoreArray<SVDCluster>
    m_svdClusters; /**< the storeArray for svdClusters as member, is faster than recreating link for each event */

    std::vector< StoreArray<SpacePoint> > m_allSpacePointStoreArrays; /**< a vector full of StoreArray carrying spacePoints. */

    std::vector< std::string > m_containerSpacePointsName; /**< intermediate storage for the names of the loaded storeArrays. */

    // modification parameters
    std::string
    m_nameOfInstance; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */

  };
} // end namespace Belle2
