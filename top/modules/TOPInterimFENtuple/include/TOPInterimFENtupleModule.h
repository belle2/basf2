/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Maeda Yosuke                                             *
 *               Umberto Tamponi                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/core/HistoModule.h>
#include <framework/gearbox/Const.h>

#include <TTree.h>

namespace Belle2 {

  /**
   * Module to produce an ntuple of smaller size respect to the data starting from TOPDigits and TOPRawDigits.
   * Most of the braches of the tree are actual copies of the most relevant data members of TOPDigits and TOPRawDigits,
   * except for few values with are slighty modified to make the analysis more immediate and easy (see the documentation of the single data members for more informations).
   * The module is intended to provide a fast way to assest the quality of the TOP data before the calibration.
   */
  class TOPInterimFENtupleModule : public HistoModule {

  public:

    /**
     * Constructor
     */
    TOPInterimFENtupleModule();

    /**
     * Destructor
     */
    virtual ~TOPInterimFENtupleModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

    /**
     * Module funcions to define histograms
     */
    virtual void defineHisto();

  private:

    /**
     * enum for maximum number of array elements (# of hits per event)
     */
    enum { c_NMaxHit = 2000 };

    TTree* m_tree = 0; /**< ntuple */
    unsigned m_calibrationChannel; /**< asic channel number where the calibration pulses are routed */

    int m_nHit = 0; /**< number of hits for the event */
    short m_slotNum[c_NMaxHit] = {0}; /**< "m_moduleID" in TOPDigit, slot number */
    short m_pixelId[c_NMaxHit] = {0}; /**< "m_pixelID" in TOPDigit */
    bool m_isCalCh[c_NMaxHit] = {0}; /**< true if the hit is in the calibration channel */
    unsigned m_eventNum[c_NMaxHit] = {0}; /**< event number taken from EventMetaData */
    short m_winNum[c_NMaxHit] = {0}; /**< "m_firstWindow" in TOPDigit */
    float m_time[c_NMaxHit] = {0}; /**< "m_time" in TOPDigit, hit time after time base correction (thus in ns unit), but not yet available */

    float m_rawTdc[c_NMaxHit] = {0}; /**< "m_rawTime" in TOPDigit, sample unit */
    float m_refTdc[c_NMaxHit] = {0}; /**< "m_rawTdc" in cal. channel */
    unsigned short m_quality[c_NMaxHit] = {0}; /**< hit quality, from TOPDigit: c_Junk = 0, c_Good = 1, c_ChargeShare = 2, c_CrossTalk = 3, c_CalPulse = 4 */
    unsigned short m_sample[c_NMaxHit] = {0}; /**< Actual 50% crossing sample in the whole waveform, calculated as (int(digit.getRawTime()) + digit.getFirstWindow() * 64) % 256. */

    float m_height[c_NMaxHit] = {0}; /**< "m_ADC" in TOPDigit */
    float m_q[c_NMaxHit] = {0}; /**< "m_integral" in TOPDigit, but not available */
    float m_width[c_NMaxHit] = {0}; /**< "m_pulseWidth" in TOPDigit, full width at half maximum of the pulse */
    short m_nPixelInRawDigit = 0; /**< # of pixels with hits in TOPRawDigits */

    unsigned short m_carrier[c_NMaxHit] = {0}; /**< carrier board number, copy from TOPRawDigit */
    unsigned short m_asic[c_NMaxHit] = {0};    /**< ASIC number, copy from TOPRawDigit */
    unsigned short m_channel[c_NMaxHit] = {0};  /**< ASIC channel number,  copy from TOPRawDigit. */
    unsigned short m_TFine[c_NMaxHit] = {0}; /**< fine timing for 50% CFD (within two samples), copied from TOPRawDigit */
    unsigned short m_sampleRise[c_NMaxHit] = {0};  /**< sample number just before 50% CFD crossing, copied from in TOPRawDigit */
    unsigned short m_samplePeak[c_NMaxHit] = {0}; /**< sample number of the peak position, calculated as TOPRawDigits.m_sampleRise + TOPRawDigits.m_dSamplePeak */
    unsigned short m_sampleFall[c_NMaxHit] = {0}; /**< sample number of the peak position, calculated as TOPRawDigits.m_sampleRise + TOPRawDigits.m_dSampleFall */




    short m_nFEHeader = 0; /**< m_FEHeaders in TOPInterimFEInfo, the total # of FE headers found */
    short m_nEmptyFEHeader = 0; /**< m_emptyFEHeaders in TOPInterimFEInfo, the total # of empty FE headers */
    short m_nWaveform = 0; /**< m_waveforms in TOPInterimFEInfo, # of waveformes */
    unsigned m_errorFlag = 0; /**< m_errorFlags in TOPInterimFEInfo, defined in the TOPInterimFEInfo.h */

  };

}  //namespace Belle2
