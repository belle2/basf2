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

#include <ecl/dbobjects/ECLCrystalCalib.h>

//Root
#include <TH2F.h>

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

    /** Select events and crystals and accumulate histograms */
    void collect() override;

  private:

    StoreArray<ECLDigit> m_eclDigits; /**< Required input array of ECLDigits */
    StoreArray<ECLDsp> m_eclDsps; /**< Required input array of ECLDSPs */
    StoreObjPtr<EventMetaData> m_evtMetaData; /**< dataStore EventMetaData */

    TH2F* varXvsCrysID;

    std::vector<float> m_ADCtoEnergy;

    double m_MinEnergyThreshold;
    double m_MaxEnergyThreshold;
    int m_MinCellID;
    int m_MaxCellID;
    std::vector<float> m_MaxVarX;

    /** Crystal electronics. */
    DBObjPtr<ECLCrystalCalib> m_CrystalElectronics{"ECLCrystalElectronics"};

    /** Crystal energy. */
    DBObjPtr<ECLCrystalCalib> m_CrystalEnergy{"ECLCrystalEnergy"};

    DBObjPtr<ECLCrystalCalib> m_eclWaveformTemplateCalibrationC1VarSq;

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
