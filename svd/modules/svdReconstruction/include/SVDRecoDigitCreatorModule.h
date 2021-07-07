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
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>

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
      void initialize() override;

      /** configure strip reconstruction */
      void beginRun() override;

      /** does the actual strip reconstruction */
      void event() override;

      /** delete pointers */
      void endRun() override;

    protected:

      //1. Collections and relations Names
      /** Name of the collection to use for the SVDShaperDigits */
      std::string m_storeShaperDigitsName;
      /** Name of the collection to use for the SVDRecoDigits */
      std::string m_storeRecoDigitsName;
      /** Name of the collection to use for the SVDClusters */
      std::string m_storeClustersName;

      /** Collection of SVDClusters */
      StoreArray<SVDCluster> m_storeClusters;
      /** Collection of SVDShaperDigits */
      StoreArray<SVDShaperDigit> m_storeShaper;
      /** Collection of SVDRecoDigits */
      StoreArray<SVDRecoDigit> m_storeReco;


      // 2. Strip Reconstruction Configuration:

      /** if true takes the strip reconstruction configuration from the DB objects*/
      bool m_useDB = true;

      /** string storing the strip time reconstruction algorithm in 6-sample DAQ mode*/
      std::string m_timeRecoWith6SamplesAlgorithm = "not set";
      /** string storing the strip time reconstruction algorithm in 3-sample DAQ mode*/
      std::string m_timeRecoWith3SamplesAlgorithm = "not set";
      /** string storing the strip charge reconstruction algorithm in 6-sample DAQ mode*/
      std::string m_chargeRecoWith6SamplesAlgorithm = "not set";
      /**string storing the strip charge reconstruction algorithm in 3-sample DAQ mode*/
      std::string m_chargeRecoWith3SamplesAlgorithm = "not set";

      SVDClusterTime* m_time6SampleClass = nullptr; /**< strip time class for the 6-sample acquisition mode*/
      SVDClusterTime* m_time3SampleClass = nullptr; /**< strip time class for the 3-sample acquisition mode*/
      SVDClusterCharge* m_charge6SampleClass = nullptr; /**< strip charge class for the 6-sample acquisition mode*/
      SVDClusterCharge* m_charge3SampleClass = nullptr; /**< strip charge class for the 3-sample acquisition mode*/

      // 3. Calibration Objects
      DBObjPtr<SVDRecoConfiguration> m_recoConfig; /**< SVD Reconstruction Configuration payload*/
      SVDNoiseCalibrations m_NoiseCal; /**< wrapper of the noise calibrations*/

    };// end class declarations


  } //end SVD namespace;
} // end namespace Belle2
