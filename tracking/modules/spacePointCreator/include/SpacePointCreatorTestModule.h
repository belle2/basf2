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
    virtual void initialize();


    /** eventWise jobs (e.g. storing spacepoints */
    virtual void event();


    /** final output with mini-feedback */
    virtual void terminate() {}


    /** initialize variables in constructor to avoid nondeterministic behavior */
    void InitializeInConstructor();


  protected:


    // Data members
    std::string m_pxdClustersName; /**< PXDCluster collection name */
    unsigned short m_pxdClustersIndex; /**< PXDCluster collection index number - created for SpacePointMetaInfo */
    StoreArray<PXDCluster> m_pxdClusters; /**< the storeArray for pxdClusters as member, is faster than recreating link for each event */
    std::string m_svdClustersName; /**< SVDCluster collection name */
    unsigned short m_svdClustersIndex; /**< SVDCluster collection index number - created for SpacePointMetaInfo */
    StoreArray<SVDCluster> m_svdClusters; /**< the storeArray for svdClusters as member, is faster than recreating link for each event */
    std::string m_spacePointsName; /**< SpacePoints collection name */
    StoreArray<SpacePoint> m_spacePoints; /**< the storeArray for spacePoints as member, is faster than recreating link for each event */
    StoreObjPtr<SpacePointMetaInfo> m_spMetaInfo; /**< collects meta info relevant for all (TB)SpacePoints */


    // modification parameters
    std::string m_nameOfInstance; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */

  };
} // end namespace Belle2
