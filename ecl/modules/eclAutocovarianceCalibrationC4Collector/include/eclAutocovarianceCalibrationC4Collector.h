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

    /** Load run-dep payloads */
    void startRun() override;

    /** Select events and crystals and accumulate histograms */
    void collect() override;

    /** Transfer fom array container to ROOT histogram */
    void closeRun() override;

  private:

    StoreArray<ECLDsp> m_eclDsps; /**< Required input array of ECLDSPs */
    StoreObjPtr<EventMetaData> m_evtMetaData; /**< dataStore EventMetaData */

    DBObjPtr<ECLCrystalCalib> m_ECLAutocovarianceCalibrationC1Threshold;  /**< thresholds obtained from C1 stage */
    std::vector<float> m_PeakToPeakThresholds; /**< vector of thresholds obtained from DB object */
    DBObjPtr<ECLCrystalCalib> m_ECLAutocovarianceCalibrationC2Baseline; /**< baselines obtained from C2 stage */
    std::vector<float> m_Baselines; /**< vector of thresholds obtained from DB object */
    DBObjPtr<ECLAutoCovariance> m_ECLAutocovarianceCalibrationC3Autocovariances; /**< Autocovariances obtained from C3 stage */

    TH2F* Chi2VsCrysID; /**< Store fit chi2 vs. crystal ID for validation */

    std::vector<TMatrixDSym> m_NoiseMatrix; /**< Stores noise matrix derived from  the input Autocovariances */

    static constexpr int m_numberofADCPoints = 31; /**< length of ECLDsp waveform */

    int m_upperThresholdForChi2Histogram = 1000; /**< Upper Threshold For Chi2 Histogram */
    int m_NbinsForChi2Histogram = 1000; /**< Number of bins For Chi2 Histogram */

  };
} // end Belle2 namespace
