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

    int m_CellID; /**< To read ntuple branch, waveform ECL crystal cell ID > */
    int m_ADC0; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC1; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC2; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC3; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC4; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC5; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC6; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC7; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC8; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC9; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC10; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC11; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC12; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC13; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC14; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC15; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC16; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC17; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC18; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC19; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC20; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC21; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC22; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC23; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC24; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC25; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC26; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC27; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC28; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC29; /**< To read ntuple branch, ith ADC value of waveform > */
    int m_ADC30; /**< To read ntuple branch, ith ADC value of waveform > */

  };
} // end Belle2 namespace
