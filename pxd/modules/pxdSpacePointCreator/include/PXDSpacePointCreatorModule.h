/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>

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
    virtual void initialize() override;


    /** eventWise jobs (e.g. storing spacepoints */
    virtual void event() override;


    /** final output with mini-feedback */
    virtual void terminate() override;


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

    StoreObjPtr<EventLevelTrackingInfo>
    m_eventLevelTrackingInfo; /**< StoreObject to access the event level tracking information */

    // modification parameters
    std::string
    m_nameOfInstance; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */


    //counters for testing
    unsigned int m_TESTERPXDClusterCtr; /**< counts total number of PXDClusters occured */

    unsigned int m_TESTERSpacePointCtr; /**< counts total number of SpacePoints occured */
  };
} // end namespace Belle2
