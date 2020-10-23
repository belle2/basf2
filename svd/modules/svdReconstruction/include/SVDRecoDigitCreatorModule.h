/**************************************************************************
 * Basf2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>

#include <svd/reconstruction/RawCluster.h>
#include <svd/reconstruction/SVDChargeReconstruction.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>

//#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/dbobjects/SVDRecoConfiguration.h>

namespace Belle2 {

  namespace SVD {

    /** The SVD RecoDigit Creator
     *
     * This module produces SVDRecoDigits from SVDShaperDigits
     */
    class SVDRecoDigitCreatorModule : public Module {

    public:

      /** Constructor defining the parameters */
      SVDRecoDigitCreatorModule();

      /** Initialize the module */
      virtual void initialize() override;

      /** Initialize the module */
      virtual void beginRun() override;

      /** does the actual clustering */
      virtual void event() override;

    protected:

      //1. Collections and relations Names
      /** Name of the collection to use for the SVDShaperDigits */
      std::string m_storeShaperDigitsName;
      /** Name of the collection to use for the SVDRecoDigits */
      std::string m_storeRecoDigitsName;
      /** Name of the collection to use for the SVDClusters */
      std::string m_storeClustersName;

      /** Name of the relation between SVDClusters and SVDRecoDigits*/
      std::string m_relClusterToRecoName;
      /** Name of the relation between SVDRecoDigits and SVDShaperDigits */
      std::string m_relRecoToShaperName;

      /** Collection of SVDClusters */
      StoreArray<SVDCluster> m_storeClusters;
      /** Collection of SVDShaperDigits */
      StoreArray<SVDShaperDigit> m_storeShaper;
      /** Collection of SVDRecoDigits */
      StoreArray<SVDRecoDigit> m_storeReco;

// 2. Reconstruction
      /** if true takes the strip reconstruction configuration from the DB objects*/
      bool m_useDB = true;

      // 3. Cluster Reconstruction Configuration:
      int m_numberOfAcquiredSamples = 0; /**< number of acquired samples, can be 6,3 or 1*/
      /** string storing the cluster time reconstruction algorithm in 6-sample DAQ mode*/
      std::string m_timeRecoWith6SamplesAlgorithm = "not set";
      /** string storing the cluster time reconstruction algorithm in 3-sample DAQ mode*/
      std::string m_timeRecoWith3SamplesAlgorithm = "not set";
      /** string storing the cluster charge reconstruction algorithm in 6-sample DAQ mode*/
      std::string m_chargeRecoWith6SamplesAlgorithm = "not set";
      /**string storing the cluster charge reconstruction algorithm in 3-sample DAQ mode*/
      std::string m_chargeRecoWith3SamplesAlgorithm = "not set";

      // 4. Calibration Objects
      DBObjPtr<SVDRecoConfiguration> m_recoConfig; /**< SVD Reconstruction Configuration payload*/

      SVDNoiseCalibrations m_NoiseCal; /**< wrapper of the noise calibrations*/
    };// end class declarations


  } //end SVD namespace;
} // end namespace Belle2
