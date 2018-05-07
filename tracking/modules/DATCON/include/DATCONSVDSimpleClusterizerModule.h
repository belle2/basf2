/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/MCParticle.h>

#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/geometry/SensorInfo.h>

#include <tracking/modules/DATCON/DATCONSVDSimpleClusterCandidate.h>
#include <tracking/dataobjects/DATCONSVDDigit.h>

#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>

#include <cstring>
#include <iomanip>
#include <sstream>
#include <fstream>

namespace Belle2 {

  /** DATCONSVDSimpleClusterizerModule: The SVD SimpleClusterizer.
   *
   * This module produces clusters from SVDRecoDigits (strips fitted with basic fitter and calibrated)
   */
  class DATCONSVDSimpleClusterizerModule : public Module {

  public:

    /** Constructor defining the parameters */
    DATCONSVDSimpleClusterizerModule();

    /** Deconstructor */
    virtual ~DATCONSVDSimpleClusterizerModule() = default;

    /** Initialize the module */
    virtual void initialize() override;

    /** do the clustering */
    virtual void event() override;


  protected:

    // Data members
    //1. Collections and relations
    /** Name of the collection to use for the SVDRecoDigits */
    std::string m_storeDATCONSVDDigitsListName;
    /** Name of the collection to use for the SVDClusters */
    std::string m_storeDATCONSVDClustersName;
    /** Name of the collection to use for the SVDTrueHits */
    std::string m_storeTrueHitsName;
    /** Name of the collection to use for the MCParticles */
    std::string m_storeMCParticlesName;

    /** StoreArray of the DATCONSimpleSVDClusters */
    StoreArray<SVDCluster> storeDATCONSVDCluster;
    /** StoreArray of the DATCONSVDDigits */
    StoreArray<DATCONSVDDigit> storeDATCONSVDDigits;
    /** StoreArray of the SVDTrueHits */
    StoreArray<SVDTrueHit> storeTrueHits;
    /** StoreArray of the MCParticles */
    StoreArray<MCParticle> storeMCParticles;


    // 2. Clustering
//     void writeClusters(DATCONSVDSimpleClusterCandidate clusterCand);

    /** Save cluster candidates as DATCONSimpleSVDCluster */
    void saveClusters();

    /** Perform an easy noise filtering for the given datconsvddigit */
    bool noiseFilter(DATCONSVDDigit datconsvddigit);

    /** Simple way to check for noisy strips / noise signals */
    unsigned short m_NoiseLevelInADU;

    /** NoiseCut in ADU */
    unsigned short m_NoiseCutInADU;

    /** Variable indicating whether simple clustering should be used, default: true */
    bool m_useSimpleClustering;

    /** Vector containing all cluster candidates that will might be stored */
    std::vector<DATCONSVDSimpleClusterCandidate> clusterCandidates;

    /** Maximum cluster size in strips */
    unsigned short m_maxClusterSize;

  };//end class declaration

} // end namespace Belle2

