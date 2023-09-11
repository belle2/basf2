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

//Framework
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

//Root
#include <TH2F.h>

namespace Belle2 {

  class ECLDsp;
  class ECLCrystalCalib;

  /** Calibration collector module that uses delayed Bhabha to compute coveriance matrix */
  class eclAutocovarianceCalibrationC2CollectorModule : public CalibrationCollectorModule {

  public:

    /** Constructor.
     */
    eclAutocovarianceCalibrationC2CollectorModule();

    /** Define histograms and read payloads from DB */
    void prepare() override;

    /** Load run-dep payloads */
    void startRun() override;

    /** Select events and crystals and accumulate histograms */
    void collect() override;

    /** save histograms */
    void closeRun() override;

  private:

    StoreArray<ECLDsp> m_eclDsps; /**< Required input array of ECLDSPs */
    StoreObjPtr<EventMetaData> m_evtMetaData; /**< dataStore EventMetaData */

    DBObjPtr<ECLCrystalCalib> m_ECLAutocovarianceCalibrationC1Threshold; /**< peak to peak noise threshold computed from step C1 */
    std::vector<float> m_PeakToPeakThresholds; /**< vector of thresholds obtained from DB object */

    double m_sumOfSamples[ECLElementNumbers::c_NCrystals] = {}; /**< sum of 31 samples for selected waveforms */
    int m_nSelectedWaveforms[ECLElementNumbers::c_NCrystals] = {}; /**< number of selected waveforms for each crystal */

    TH1D* m_BaselineVsCrysID; /**< final histogram returned by collector contains baseline */
    TH1D* m_CounterVsCrysID; /**< final histogram returned by collector contains counter */

    const int m_numberofADCPoints = 31; /**< length of ECLDsp waveform */

  };
} // end Belle2 namespace
