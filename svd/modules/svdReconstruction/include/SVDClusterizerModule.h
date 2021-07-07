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
#include <framework/database/DBObjPtr.h>

#include <svd/reconstruction/RawCluster.h>
#include <svd/reconstruction/SVDClusterTime.h>
#include <svd/reconstruction/SVDClusterCharge.h>
#include <svd/reconstruction/SVDClusterPosition.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDClustering.h>
#include <svd/dbobjects/SVDRecoConfiguration.h>

namespace Belle2 {

  namespace SVD {

    /** The SVD Clusterizer
     *
     * This module produces SVDClusters from SVDShaperDigits
     */
    class SVDClusterizerModule : public Module {

    public:

      /** Constructor defining the parameters */
      SVDClusterizerModule();

      /** Initialize the module */
      void initialize() override;

      /** configure clustering */
      void beginRun() override;

      /** does the actual clustering */
      void event() override;

      /** delete pointers */
      void endRun() override;

    protected:

      //1. Collections and relations Names
      /** Name of the collection to use for the SVDShaperDigits */
      std::string m_storeShaperDigitsName;
      /** Name of the collection to use for the SVDClusters */
      std::string m_storeClustersName;
      /** Name of the collection to use for the SVDTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the collection to use for the MCParticles */
      std::string m_storeMCParticlesName;

      /** Name of the relation between SVDShaperDigits and MCParticles */
      std::string m_relShaperDigitMCParticleName;
      /** Name of the relation between SVDClusters and MCParticles */
      std::string m_relClusterMCParticleName;
      /** Name of the relation between SVDClusters and SVDShaperDigits */
      std::string m_relClusterShaperDigitName;
      /** Name of the relation between SVDShaperDigits and SVDTrueHits */
      std::string m_relShaperDigitTrueHitName;
      /** Name of the relation between SVDClusters and SVDTrueHits */
      std::string m_relClusterTrueHitName;

      /** Collection of SVDClusters */
      StoreArray<SVDCluster> m_storeClusters;
      /** Collection of SVDShaperDigits */
      StoreArray<SVDShaperDigit> m_storeDigits;
      /** Collection of SVDTrueHits */
      StoreArray<SVDTrueHit> m_storeTrueHits;
      /** Collection of MCParticles */
      StoreArray<MCParticle> m_storeMCParticles;

      // 2. Clustering
      /** Seed cut in units of noise. DEPRECATED - useDB */
      double m_cutSeed = 5;
      /** Adjacent cut in units of noise. DEPRECATED - useDB*/
      double m_cutAdjacent = 3;
      /** Cluster cut in units of m_elNoise, not included (yet?) */
      double m_cutCluster = 0;
      /** if true takes the clusterizer cuts and reconstruction configuration from the DB objects*/
      bool m_useDB = true;

      // 3. Cluster Reconstruction Configuration:
      int m_numberOfAcquiredSamples = 0; /**< number of acquired samples, can be 6 or 3 (1 is not supported!)*/
      /** string storing the cluster time reconstruction algorithm in 6-sample DAQ mode*/
      std::string m_timeRecoWith6SamplesAlgorithm = "not set";
      /** string storing the cluster time reconstruction algorithm in 3-sample DAQ mode*/
      std::string m_timeRecoWith3SamplesAlgorithm = "not set";
      /** string storing the cluster charge reconstruction algorithm in 6-sample DAQ mode*/
      std::string m_chargeRecoWith6SamplesAlgorithm = "not set";
      /**string storing the cluster charge reconstruction algorithm in 3-sample DAQ mode*/
      std::string m_chargeRecoWith3SamplesAlgorithm = "not set";
      /** string storing the cluster position reconstruction algorithm in 6-sample DAQ mode*/
      std::string m_positionRecoWith6SamplesAlgorithm = "not set";
      /**string storing the cluster position reconstruction algorithm in 3-sample DAQ mode*/
      std::string m_positionRecoWith3SamplesAlgorithm = "not set";

      /** string storing the strip time reconstruction algorithm for cluster position reconstruction in 6-sample DAQ mode*/
      std::string m_stripTimeRecoWith6SamplesAlgorithm = "not set";
      /** string storing the strip time reconstruction algorithm for cluster position reconstruction in 3-sample DAQ mode*/
      std::string m_stripTimeRecoWith3SamplesAlgorithm = "not set";
      /** string storing the strip charge reconstruction algorithm for cluster position reconstruction in 6-sample DAQ mode*/
      std::string m_stripChargeRecoWith6SamplesAlgorithm = "not set";
      /**string storing the strip charge reconstruction algorithm for cluster reconstruction in 3-sample DAQ mode*/
      std::string m_stripChargeRecoWith3SamplesAlgorithm = "not set";

      SVDClusterTime* m_time6SampleClass = nullptr; /**< cluster time class for the 6-sample acquisition mode*/
      SVDClusterTime* m_time3SampleClass = nullptr; /**< cluster time class for the 3-sample acquisition mode*/
      SVDClusterCharge* m_charge6SampleClass = nullptr; /**< cluster charge class for the 6-sample acquisition mode*/
      SVDClusterCharge* m_charge3SampleClass = nullptr; /**< cluster charge class for the 3-sample acquisition mode*/
      SVDClusterPosition* m_position6SampleClass = nullptr; /**< cluster position class for the 6-sample acquisition mode*/
      SVDClusterPosition* m_position3SampleClass = nullptr; /**< cluster position class for the 3-sample acquisition mode*/


      // 4. Calibration Objects
      bool m_returnRawClusterTime = false; /**< if true cluster time is not calibrated, to be used for time calibration */

      DBObjPtr<SVDRecoConfiguration> m_recoConfig; /**< SVD Reconstruction Configuration payload*/
      SVDNoiseCalibrations m_NoiseCal; /**<SVDNoise calibrations db object*/
      SVDClustering m_ClusterCal; /**<SVDCluster calibrations db object*/


      /**
       * returns the position of the cluster after
       * lorentz shift correction
       */
      double applyLorentzShiftCorrection(double position, VxdID vxdID, bool isU);

      /**
       * computes charge, position and time of the raw cluster
       * and appends the new SVDCluster to the StoreArray
       */
      void finalizeCluster(Belle2::SVD::RawCluster& rawCluster);

      /**
       * writes the relations of the SVDClusters with the other StoreArrays
       */
      void writeClusterRelations(Belle2::SVD::RawCluster& rawCluster);

    };// end class declarations


  } //end SVD namespace;
} // end namespace Belle2
