/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <vxd/dataobjects/VxdID.h>

#include <tracking/dataobjects/DATCONSVDSpacePoint.h>
#include <tracking/dataobjects/DATCONSimpleSVDCluster.h>

#include <string>



namespace Belle2 {

  /**
  * Imports Clusters of the SVD detector and converts them to spacePoints.
  *
  */
  class DATCONSVDSpacePointCreatorModule : public Module {

  public:

    /** Constructor */
    DATCONSVDSpacePointCreatorModule();

    /** Deconstructor */
    virtual ~DATCONSVDSpacePointCreatorModule();

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
    std::string m_storeDATCONSimpleSVDClustersName; /**< SVDCluster collection name */
    std::string m_storeDATCONSVDSpacePointsName; /**< SpacePoints collection name */
    /** Name of the collection to use for the SVDTrueHits */
    std::string m_storeTrueHitsName;
    /** Name of the collection to use for the MCParticles */
    std::string m_storeMCParticlesName;

    StoreArray<DATCONSimpleSVDCluster>
    m_storeDATCONSimpleSVDClusters;  /**< the storeArray for svdClusters as member, is faster than recreating link for each event */

    StoreArray<DATCONSVDSpacePoint>
    m_storeDATCONSVDSpacePoints; /**< the storeArray for spacePoints as member, is faster than recreating link for each event */
    /** StoreArray of the SVDTrueHits */
    StoreArray<SVDTrueHit> m_storeTrueHits;
    /** StoreArray of the MCParticles */
    StoreArray<MCParticle> m_storeMCParticles;

    // modification parameters
    std::string
    m_nameOfInstance; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */

    float m_minClusterTime; /**< clusters with time below this value are not considered to make spacePoints*/

    bool m_onlySingleClusterSpacePoints; /**< standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more. Does not affect pixel-type Clusters */

  }; // end class

} // end namespace Belle2
