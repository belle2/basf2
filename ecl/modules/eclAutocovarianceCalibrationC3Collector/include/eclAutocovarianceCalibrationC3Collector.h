/**************************************************************************
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>

//Calibration
#include <calibration/CalibrationCollectorModule.h>

//Root
#include <TH2F.h>

//Framework
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  class ECLDsp;
  class ECLCrystalCalib;

  /** Calibration collector module that uses delayed Bhabha to compute coveriance matrix */
  class eclAutocovarianceCalibrationC3CollectorModule : public CalibrationCollectorModule {

  public:

    /** Constructor. */
    eclAutocovarianceCalibrationC3CollectorModule();

    /** Define histograms and read payloads from DB */
    void prepare() override;

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
    std::vector<float> m_tempArray; /**< vector of thresholds obtained from DB object */
    TH2F* CovarianceMatrixInfoVsCrysID;  /**< result returned by collector that contains the coveriance matrix for each crystal  */
    const int nADCWaveformPoints = 31; /**< length of ECLDSP waveforms  */
    float m_CovarianceMatrixInfoVsCrysIDHistogram[ECLElementNumbers::c_NCrystals][32] = {}; /**< container for coveriance matrix  */

  };
} // end Belle2 namespace
