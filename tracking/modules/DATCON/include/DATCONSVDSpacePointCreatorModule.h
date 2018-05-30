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

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>

#include <vxd/dataobjects/VxdID.h>

#include <tracking/dataobjects/DATCONSVDSpacePoint.h>


namespace Belle2 {

  /**
    * This module creates DATCONSVDSpacePoints from the
    * DATCONSVDCluster. These space points are a simplified
    * version of the global SpacePoint class, not containing
    * as many information as the original.
    */
  class DATCONSVDSpacePointCreatorModule : public Module {

  public:

    /** Constructor */
    DATCONSVDSpacePointCreatorModule();

    /** Deconstructor */
    virtual ~DATCONSVDSpacePointCreatorModule() = default;

    /** Init the module.
    *
    * prepares all store- and relationArrays.
    */
    virtual void initialize();

    /** eventWise jobs (e.g. storing spacepoints */
    virtual void event();


  protected:

    // Data members
    /** DATCONSimpleSVDCluster StoreArray name */
    std::string m_storeDATCONSVDClustersName;
    /** DATCONSVDSpacePoints StoreArray name */
    std::string m_storeDATCONSVDSpacePointsName;
    /** SVDTrueHits StoreArray name */
    std::string m_storeTrueHitsName;
    /** MCParticles StoreArray name */
    std::string m_storeMCParticlesName;

    /** StoreArray for the DATCONSimpleSVDCluster that serve as input for the space point creation. */
    StoreArray<SVDCluster> storeDATCONSVDClusters;
    /** StoreArray for DATCONSVDSpacePoints that are created in this module. */
    StoreArray<DATCONSVDSpacePoint> storeDATCONSVDSpacePoints;
    /** StoreArray of the SVDTrueHits */
    StoreArray<SVDTrueHit> storeTrueHits;
    /** StoreArray of the MCParticles */
    StoreArray<MCParticle> storeMCParticles;

    // modification parameters
    /** standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more. Does not affect pixel-type Clusters */
    bool m_onlySingleClusterSpacePoints;

  }; // end class

} // end namespace Belle2
