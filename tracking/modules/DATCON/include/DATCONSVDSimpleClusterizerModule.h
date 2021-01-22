/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/MCParticle.h>

#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/geometry/SensorInfo.h>
// #include <svd/calibration/SVDFADCMaskedStrips.h>
// #include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
// #include <svd/calibration/SVDPedestalCalibrations.h>

#include <tracking/modules/DATCON/DATCONSVDSimpleClusterCandidate.h>
#include <tracking/dataobjects/DATCONSVDDigit2.h>

#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>

#include <fstream>

namespace Belle2 {

  /** Simple representation of a SVD Noise Map */
  typedef std::map<int, float> simpleSVDNoiseMap;

  /** DATCONSVDSimpleClusterizerModule:
    * This class performs a simple clusterisation and noise filtering
    * of SVD hits (DATCONSVDDigits in this case). The seed strip of the
    * cluster is (clustersize / 2 + 0.5), and the maximum cluster size
    * allowed is 5 strips. The noise filter removes hits where the strip
    * charge in ADU is not sufficiently large.
    */
  class DATCONSVDSimpleClusterizerModule : public Module {

  public:

    /** Constructor defining the parameters */
    DATCONSVDSimpleClusterizerModule();

    /** Deconstructor */
    virtual ~DATCONSVDSimpleClusterizerModule() = default;

    /** Begin Run */
    virtual void beginRun() override;

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
    StoreArray<DATCONSVDDigit2> storeDATCONSVDDigits;
    /** StoreArray of the SVDTrueHits */
    StoreArray<SVDTrueHit> storeTrueHits;
    /** StoreArray of the MCParticles */
    StoreArray<MCParticle> storeMCParticles;


    // 2. Clustering
//     void writeClusters(DATCONSVDSimpleClusterCandidate clusterCand);


    void fillDATCONSVDNoiseMap();

    /** Save cluster candidates as DATCONSimpleSVDCluster */
    void saveClusters();

    /** Perform an easy noise filtering for the given datconsvddigit */
    float calculateSNR(DATCONSVDDigit2 datconsvddigit);

//     SVDFADCMaskedStrips m_MaskedStr; /**< FADC masked strip payload*/
    SVDNoiseCalibrations m_NoiseCal; /**< noise payload*/
//     SVDPulseShapeCalibrations m_PulseShapeCal; /**< pulse shape payload*/
//     SVDPedestalCalibrations m_PedestalCal; /**< pedestal payload*/

    /** Simple noise map for u-strips*/
    simpleSVDNoiseMap svdNoiseMapU;
    /** Simple noise map for v-strips*/
    simpleSVDNoiseMap svdNoiseMapV;

    /** Use the simple noise filter during clustering? */
    bool m_useNoiseFilter;

    /** Simple way to check for noisy strips / noise signals */
    unsigned short m_NoiseLevelInADU;

    /** NoiseCut in ADU */
    unsigned short m_NoiseCutInADU;

    /** Variable indicating whether simple clustering should be used, default: true */
    bool m_useSimpleClustering;

    /** Vector containing all cluster candidates that will might be stored */
    std::vector<DATCONSVDSimpleClusterCandidate> clusterCandidates;

    /** Maximum cluster size for u in strips */
    unsigned short m_maxClusterSizeU;
    /** Maximum cluster size for v in strips */
    unsigned short m_maxClusterSizeV;

    /** Value above which u-strips are considered noisy, all other u-strips will
     * get assigned a standard noise value of m_noiseCutU */
    float m_noiseCutU;
    /** Value above which b-strips are considered noisy, all other b-strips will
     * get assigned a standard noise value of m_noiseCutV */
    float m_noiseCutV;
    /** Require a SNR for a u-strip signal to be valid. */
    float m_requiredSNRstripU;
    /** Require a SNR for a v-strip signal to be valid. */
    float m_requiredSNRstripV;
    /** Require a SNR for at least one strip in the u-cluster to make the cluster valid. */
    float m_requiredSNRclusterU;
    /** Require a SNR for at least one strip in the v-cluster to make the cluster valid. */
    float m_requiredSNRclusterV;

    /** Write the simple SVD noise maps to files? */
    bool m_writeNoiseMapsToFile;

  };//end class declaration

} // end namespace Belle2

