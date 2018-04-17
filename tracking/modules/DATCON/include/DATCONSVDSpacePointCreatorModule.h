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
    /** DATCONSimpleSVDCluster StoreArray name */
    std::string m_storeDATCONSimpleSVDClustersName;
    /** DATCONSVDSpacePoints StoreArray name */
    std::string m_storeDATCONSVDSpacePointsName;
    /** SVDTrueHits StoreArray name */
    std::string m_storeTrueHitsName;
    /** MCParticles StoreArray name */
    std::string m_storeMCParticlesName;

    /** StoreArray for the DATCONSimpleSVDCluster that serve as input for the space point creation. */
    StoreArray<DATCONSimpleSVDCluster> m_storeDATCONSimpleSVDClusters;
    /** StoreArray for DATCONSVDSpacePoints that are created in this module. */
    StoreArray<DATCONSVDSpacePoint> m_storeDATCONSVDSpacePoints;
    /** StoreArray of the SVDTrueHits */
    StoreArray<SVDTrueHit> m_storeTrueHits;
    /** StoreArray of the MCParticles */
    StoreArray<MCParticle> m_storeMCParticles;

    // modification parameters
    /** allows the user to set an identifier for this module. Useful if one wants to use several instances of that module */
    std::string  m_nameOfInstance;

    /** clusters with time below this value are not considered to make spacePoints*/
    float m_minClusterTime;

    /** standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more. Does not affect pixel-type Clusters */
    bool m_onlySingleClusterSpacePoints;

  }; // end class

} // end namespace Belle2
