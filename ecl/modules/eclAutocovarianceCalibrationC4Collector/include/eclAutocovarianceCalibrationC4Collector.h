/**************************************************************************
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//Calibration
#include <calibration/CalibrationCollectorModule.h>

#include <ecl/dbobjects/ECLAutoCovariance.h>

//Framework
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

//Root
#include <TH2F.h>
#include <TMatrixDSym.h>

namespace Belle2 {

  class ECLDsp;
  class ECLCrystalCalib;
  class ECLAutoCovariance;

  /** Calibration collector module that uses delayed Bhabha to compute coveriance matrix */
  class eclAutocovarianceCalibrationC4CollectorModule : public CalibrationCollectorModule {

  public:

    /** Constructor.
     */
    eclAutocovarianceCalibrationC4CollectorModule();

    /** Define histograms and read payloads from DB */
    void prepare() override;

    /** Select events and crystals and accumulate histograms */
    void collect() override;

    void closeRun() override;

  private:

    StoreArray<ECLDsp> m_eclDsps; /**< Required input array of ECLDSPs */
    StoreObjPtr<EventMetaData> m_evtMetaData; /**< dataStore EventMetaData */

    DBObjPtr<ECLCrystalCalib> m_ECLAutocovarianceCalibrationC1Threshold;
    std::vector<float> m_PeakToPeakThresholds; /**< vector of thresholds obtained from DB object */
    DBObjPtr<ECLCrystalCalib> m_ECLAutocovarianceCalibrationC2Baseline;
    std::vector<float> m_Baselines; /**< vector of thresholds obtained from DB object */
    DBObjPtr<ECLAutoCovariance> m_ECLAutocovarianceCalibrationC3Autocovariances;

    TH2F* Chi2VsCrysID;

    std::vector<TMatrixDSym> m_NoiseMatrix;

  };
} // end Belle2 namespace
