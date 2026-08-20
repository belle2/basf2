/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>

/* Basf2 headers. */
#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

/* ROOT headers. */
#include <TFile.h>
#include <TTree.h>

namespace Belle2 {

  /** Store information needed to calculate ECL waveform template shapes */
  class eclWaveformCalibCollectorModule : public Module {

  public:

    /** Constructor: Sets the description, the properties and the parameters of the module */
    eclWaveformCalibCollectorModule();

    /** Initializes the module. */
    virtual void initialize() override;

    /** terminate */
    virtual void terminate() override;

    /** Method is called for each event. */
    virtual void event() override;

  private:

    int m_selectCellID; /**< Root used to select specific CellID to save */

    std::string m_dataOutFileName;  /**< Root file name for saving the output */
    TTree* tree{nullptr};  /**< Root tree for saving the output */
    TFile* m_rootFile{nullptr};  /**< Root file for saving the output */

    StoreArray<ECLDsp> m_eclDSPs;  /**< StoreArray ECLDsp */

    StoreArray<ECLDigit> m_eclDigits;   /**< StoreArray ECLDigit */

    std::vector<float> m_ADCtoEnergy;  /**< calibration vector from adc to energy */

    StoreObjPtr<EventMetaData> m_EventMetaData; /**< Event metadata info */

    double m_LowEnergyThresholdGeV; /**< Low Energy Threshold in GeV. >*/

    double m_HighEnergyThresholdGeV; /**< High Energy Threshold in GeV. >*/

    bool m_includeWaveforms; /**< Flag to save ADC information. >*/

    int m_CellID = 0; /**< To read ntuple branch, waveform ECL crystal cell ID > */
    int m_runNum = 0; /**< To read ntuple branch, waveform ECL crystal cell ID > */
    int m_expNum = 0; /**< To read ntuple branch, waveform ECL crystal cell ID > */
    float m_OnlineE = 0.0; /**< To read ntuple branch, waveform energy measured online by FPGA > */
    float m_OfflineE = 0.0;  /**< To read ntuple branch, waveform energy measure offline with multi-template fit > */
    float m_OfflineHadE = 0.0; /**< To read ntuple branch, waveform hadron energy measure offline with multi-template fit > */
    float m_Baseline = 0.0; /**< To read ntuple branch, baseline of waveform > */
    float m_BaselineRMS = 0.0; /**< To read ntuple branch, RMS squared of waveform baseline > */
    float m_calibConst = 0.0; /**< To read ntuple branch, calibration from ADC to GeV > */
    float m_Chi2 = 0.0; /**< To read ntuple branch, offline fit chi2 > */
    float m_Chi2Save0 = 0.0; /**< To read ntuple branch, offline fit chi2 for fit type 0 > */
    float m_Chi2Save1 = 0.0; /**< To read ntuple branch, offline fit chi2 for fit type 1 > */
    float m_Chi2Save2 = 0.0; /**< To read ntuple branch, offline fit chi2 for fit type 2 > */
    int m_FitType = 0; /**< To read ntuple branch, offline fit type with best chi2 > */
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
}
