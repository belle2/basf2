/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <framework/datastore/StoreArray.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/calibration/SVDNoiseCalibrations.h>

#include <string>
#include <vector>
#include <fstream>
#include <numeric>

namespace Belle2 {
  class ModuleParamList;
  class DATCONSVDDigit;
  class SVDCluster;
  struct DATCONSVDClusterCandidate;

  /**
   * Findlet for clustering DATCONSVDDigits and creating SVDClusters that are used for tracking in DATCON.
   * This finldet only clusters strips on one side.
   * The created SVDClusters can be stored in the DataStore by setting m_param_saveClusterToDataStore to true.
   */
  class DATCONSVDClusterizer : public TrackFindingCDC::Findlet<const DATCONSVDDigit, SVDCluster> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const DATCONSVDDigit, SVDCluster>;

    /** Simple representation of a SVD Noise Map */
    typedef std::map<int, float> SimpleSVDNoiseMap;

  public:
    /// Cluster SVD strips
    DATCONSVDClusterizer();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Begin Run
    void beginRun() override;

    /// Load in the DATCONSVDDigits and create SVDClusters from them
    void apply(const std::vector<DATCONSVDDigit>& digits, std::vector<SVDCluster>& clusters) override;

  private:
    /// fill the noise map to be used for SNR cuts
    void fillDATCONSVDNoiseMap();

    /// calculate the SNR of a DATCONSVDDigit (= one strip) in a simplified way
    /// @param digit DATCONSVDDigit for which the SNR is calculated
    /// @return SNR of the digit
    float calculateSNR(DATCONSVDDigit digit);

    /// save the current cluster candidate as a SVDCluster
    /// @param clusterCand cluster candidate to be stored
    /// @param clusters    vector containing all the finalized clusters
    void saveCluster(DATCONSVDClusterCandidate& clusterCand, std::vector<SVDCluster>& clusters);

    /// SVD Noise payload
    SVDNoiseCalibrations m_NoiseCal;

    // Parameters
    /// Simple noise map for u-strips*/
    SimpleSVDNoiseMap m_svdNoiseMap;

    /// File name for a file containing the noise of the strips for export to FPGA
    std::string m_param_noiseMapfileName = "noiseMap.txt";
    /// Write the noise map to file m_param_noiseMapfileName for export to FPGA?
    bool m_param_writeNoiseMapsToFile = false;
    /// Is this the finldlet for u-side or for v-side?
    bool m_param_isU = true;

    /// Save SVDCluster to DataStore for analysis?
    bool m_param_saveClusterToDataStore = false;
    /// SVDClusters StoreArray name
    std::string m_param_storeSVDClustersName = "DATCONSVDClusters";
    /// StoreArray to save the clusters to
    StoreArray<SVDCluster> m_storeSVDClusters;

    /// maximum cluster size
    unsigned short m_param_maxiClusterSize = 20;
    /// Value above which u-strips are considered noisy, all other u-strips will get assigned a standard noise value of m_noiseCut
    float m_param_noiseCut = 4;
    /// Require a SNR for a u-strip signal to be valid.
    float m_param_requiredSNRstrip = 5;
    /// Require a SNR for at least one strip in the u-cluster to make the cluster valid.
    float m_param_requiredSNRcluster = 5;

    /// instance of GeoCache to avoid creating it again for every cluster
    const VXD::GeoCache& m_geoCache = VXD::GeoCache::getInstance();

  };
}
