/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//Calibration
#include <calibration/CalibrationCollectorModule.h>

//Framework
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

//ECL
#include <ecl/dbobjects/ECLCrystalCalib.h>

namespace Belle2 {

  class ECLDigit;
  class ECLDsp;

  /** Calibration collector module that uses delayed Bhabha to compute coveriance matrix */
  class eclWaveformTemplateCalibrationC2CollectorModule : public CalibrationCollectorModule {

  public:

    /** Constructor.
     */
    eclWaveformTemplateCalibrationC2CollectorModule();

    /** Define histograms and read payloads from DB */
    void prepare() override;

    /** Load run-dep payloads */
    void startRun() override;

    /** Select events and crystals and accumulate histograms */
    void collect() override;

  private:
    StoreArray<ECLDigit> m_eclDigits; /**< Required input array of ECLDigits */
    StoreArray<ECLDsp> m_eclDsps; /**< Required input array of ECLDSPs */
    StoreObjPtr<EventMetaData> m_evtMetaData; /**< dataStore EventMetaData */

    std::vector<float> m_ADCtoEnergy; /**< Crystal energy calibration constants */

    double m_MinEnergyThreshold; /**< Minimum energy threshold  */
    double m_MaxEnergyThreshold; /**< Maximum energy threshold  */
    int m_MinCellID; /**< Minimum Cell ID  */
    int m_MaxCellID; /**< Maximum Cell ID  */

    int m_ADCFloorThreshold; /**< Used to determine if waveform hit ADC floor */
    int m_baselineLimit; /**< Number of points to compute baseline */
    const int m_numberofADCPoints = 31; /**< Number of ADC points in waveform */

    /** Crystal electronics. */
    DBObjPtr<ECLCrystalCalib> m_CrystalElectronics{"ECLCrystalElectronics"};

    /** Crystal energy. */
    DBObjPtr<ECLCrystalCalib> m_CrystalEnergy{"ECLCrystalEnergy"};

    /** Baseline noise thresholds compute in stage C1. */
    DBObjPtr<ECLCrystalCalib> m_eclWaveformTemplateCalibrationC1MaxResLimit;

    /** Vector to store baseline noise thresholds compute in stage C1. */
    std::vector<float> m_maxResLimit;

    int m_CellID = 0; /**< To read ntuple branch, waveform ECL crystal cell ID > */
    int m_ADC0 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC1 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC2 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC3 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC4 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC5 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC6 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC7 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC8 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC9 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC10 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC11 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC12 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC13 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC14 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC15 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC16 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC17 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC18 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC19 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC20 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC21 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC22 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC23 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC24 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC25 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC26 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC27 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC28 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC29 = 0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC30 = 0; /**< To read ntuple branch, ith ADC value of waveform > */

  };
} // end Belle2 namespace
