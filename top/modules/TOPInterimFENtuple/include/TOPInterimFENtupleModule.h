/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Maeda Yosuke                                             *
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
   * Module to produce ntuple from TOPDigits and TOPRawDigits
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
    float m_rawTdc[c_NMaxHit] = {0}; /**< "m_TDC" in TOPDigit, divided by 16 to make it in sample unit */
    float m_refTdc[c_NMaxHit] = {0}; /**< "m_rawTdc" in cal. channel */
    float m_height[c_NMaxHit] = {0}; /**< "m_ADC" in TOPDigit */
    float m_q[c_NMaxHit] = {0}; /**< "m_integral" in TOPDigit, but not available */
    float m_width[c_NMaxHit] = {0}; /**< "m_pulseWidth" in TOPDigit, full width at half maximum of the pulse */
    short m_nPixelInRawDigit = 0; /**< # of pixels with hits in TOPRawDigits */

    short m_nFEHeader = 0; /**< m_FEHeaders in TOPInterimFEInfo, the total # of FE headers found */
    short m_nEmptyFEHeader = 0; /**< m_emptyFEHeaders in TOPInterimFEInfo, the total # of empty FE headers */
    short m_nWaveform = 0; /**< m_waveforms in TOPInterimFEInfo, # of waveformes */
    unsigned m_errorFlag = 0; /**< m_errorFlags in TOPInterimFEInfo, defined in the TOPInterimFEInfo.h */
  };

}  //namespace Belle2
