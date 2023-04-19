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
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

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

    int m_selectCellID;

    std::string m_dataOutFileName;  /**< Root file name for saving the output */
    TTree* tree{nullptr};  /**< Root tree for saving the output */
    TFile* m_rootFile{nullptr};  /**< Root file for saving the output */

    StoreArray<ECLDsp> m_eclDSPs;  /**< StoreArray ECLDsp */

    StoreArray<ECLDigit> m_eclDigits;   /**< StoreArray ECLDigit */

    std::vector<float> m_ADCtoEnergy;  /**< calibration vector from adc to energy*/

    StoreObjPtr<EventMetaData> m_EventMetaData;

    double m_LowEnergyThresholdGeV; /**< Low Energy Threshold in GeV. >*/

    double m_HighEnergyThresholdGeV; /**< High Energy Threshold in GeV. >*/

    bool m_includeWaveforms; /**< Flag to save ADC information. >*/

    int m_CellID; /**< To read ntuple branch, waveform ECL crystal cell ID > */
    int m_runNum; /**< To read ntuple branch, waveform ECL crystal cell ID > */
    int m_expNum; /**< To read ntuple branch, waveform ECL crystal cell ID > */
    float m_OnlineE; /**< To read ntuple branch, waveform energy measured online by FPGA > */
    float m_OfflineE;  /**< To read ntuple branch, waveform energy measure offline with multi-template fit > */
    float m_OfflineHadE; /**< To read ntuple branch, waveform hadron energy measure offline with multi-template fit > */
    float m_Baseline; /**< To read ntuple branch, baseline of waveform > */
    float m_BaselineRMS; /**< To read ntuple branch, RMS squared of waveform baseline > */
    float m_calibConst; /**< To read ntuple branch, calibration from ADC to GeV > */
    float m_Chi2; /**< To read ntuple branch, offline fit chi2 > */
    float m_Chi2Save0; /**< To read ntuple branch, offline fit chi2 for fit type 0 > */
    float m_Chi2Save1; /**< To read ntuple branch, offline fit chi2 for fit type 1 > */
    float m_Chi2Save2; /**< To read ntuple branch, offline fit chi2 for fit type 2 > */
    int m_FitType; /**< To read ntuple branch, offline fit type with best chi2 > */
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
}
