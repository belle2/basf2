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

  /**
   * Findlet for clustering DATCONSVDDigits and creating SVDClusters that are used for tracking in DATCON.
   * This finldet only clusters strips on one side.
   * The created SVDClusters can be stored in the DataStore by setting m_param_saveClusterToDataStore to true.
   */
  class DATCONSVDClusterizer : public TrackFindingCDC::Findlet<DATCONSVDDigit, SVDCluster> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<DATCONSVDDigit, SVDCluster>;

    /** Simple representation of a SVD Noise Map */
    typedef std::map<int, float> simpleSVDNoiseMap;

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
    void apply(std::vector<DATCONSVDDigit>& digits, std::vector<SVDCluster>& clusters) override;

  private:
    /// fill the noise map to be used for SNR cuts
    void fillDATCONSVDNoiseMap();

    /// calculate the SNR of a DATCONSVDDigit (= one strip) in a simplified way
    /// @param digit DATCONSVDDigit for which the SNR is calculated
    /// @return SNR of the digit
    float calculateSNR(DATCONSVDDigit digit);

    /// SVD Noise payload
    SVDNoiseCalibrations m_NoiseCal;

    // Parameters
    /// Simple noise map for u-strips*/
    simpleSVDNoiseMap svdNoiseMap;

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
    const VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

    /// struct containing a cluster candidate for easier handling
    struct clusterCandidate {
      VxdID vxdID = 0; /**< VxdID of the cluster */
      std::vector<unsigned short> strips; /**< Vector containing strips (DATCONSVDDigits) that are added */
      std::vector<unsigned short> charges; /**< Vector containing the charges of the corresponding strips that are added */
      float maxSNRinClusterCandidate; /**< Maximum SNR of all the strips in the cluster candidate */

      int charge = 0; /**< Charge of the cluster */
      int seedStripIndex = 0; /**< Index of the seed strip of the cluster (0...m_Size) */
      int seedStrip = 0; /**< SVD strip (i.e. 0...511 or 0...767) that is considered the seed strip of the cluster */
      int seedCharge = 0; /**< Seed Charge of the cluster */
      float clusterPosition = 0; /**< Position of the cluster */

      /// add a new strip to the current cluster candidate if possible
      /// @param nextID VxdID of the digit to be added
      /// @param nextCharge charge (in ADU) of the digit to be added
      /// @param nextCellID strip number of the digit to be added
      /// @param nextStripSNR SNR of the digit to be added
      /// @param maxClusterSize maximum cluster size, if exceeded, start new cluster
      /// @return true if digit was successfully added to current cluster candidate, else return false
      bool add(VxdID nextID, int nextCharge, unsigned short nextCellID, float nextStripSNR, unsigned short maxClusterSize)
      {
        bool added = false;

        // do not add if you are on the wrong sensor or side
        if (vxdID != nextID) return false;

        // add if it's the first strip
        if (strips.size() == 0) added = true;

        /**add if it adjacent to the last strip added
        * (we assume that SVDRecoDigits are ordered)
        * and if cluster size would still be <= 4 strips
        */
        if (strips.size() > 0 and nextCellID == strips.back() + 1 and strips.size() < maxClusterSize)
          added  = true;

        //add it to the vector od strips, update the seed nextCharge and index:
        if (added) {
          strips.push_back(nextCellID);
          charges.push_back(nextCharge);

          if (nextCharge > seedCharge) seedCharge = nextCharge;

          if (nextStripSNR > maxSNRinClusterCandidate) {
            maxSNRinClusterCandidate = nextStripSNR;
          }
        }
        return added;

      };

      /// calculate cluster properties once a cluster is ready to be stored
      /// @param pitch strip pitch of the sensor and sensor side on which the cluster was found
      /// @param stripsInSensor number of strips of the sensor and sensor side on which the cluster was found
      void finalizeCluster(const double pitch, const int stripsInSensor)
      {
        charge = std::accumulate(charges.begin(), charges.end(), 0);
        seedStripIndex  = strips.size() / 2 + 1;
        seedStrip       = (strips.at(0) + seedStripIndex - 1);
        seedCharge      = charges.at(seedStripIndex - 1);
        clusterPosition = pitch * (seedStrip - stripsInSensor / 2);
      };
    };

  };
}
