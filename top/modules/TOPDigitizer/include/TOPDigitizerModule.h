/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <top/dataobjects/TOPSimHit.h>
#include <top/dataobjects/TOPSimCalPulse.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRawDigit.h>
#include <top/dataobjects/TOPRawWaveform.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPSampleTimes.h>
#include <top/dbobjects/TOPCalTimebase.h>
#include <top/dbobjects/TOPCalChannelPulseHeight.h>
#include <top/dbobjects/TOPCalChannelThreshold.h>
#include <top/dbobjects/TOPCalChannelNoise.h>

#include <top/modules/TOPDigitizer/PulseHeightGenerator.h>
#include <string>


namespace Belle2 {

  /**
   * TOP digitizer.
   * This module takes hits form G4 simulation (TOPSimHits),
   * applies TTS, T0 jitter and does spatial and time digitization.
   * (QE had been moved to the simulation: applied in SensitiveBar, SensitivePMT)
   * Output to TOPDigits.
   */
  class TOPDigitizerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPDigitizerModule();

    /**
     * Destructor
     */
    virtual ~TOPDigitizerModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:

    /**
     * Generates and returns pulse height
     * @param moduleID module ID (1-based)
     * @param pixelID pixel ID (1-based)
     * @return pulse height [ADC counts]
     */
    double generatePulseHeight(int moduleID, int pixelID) const;

    // module steering parameters
    double m_timeZeroJitter = 0;       /**< r.m.s of T0 jitter */
    double m_electronicJitter = 0;     /**< r.m.s of electronic jitter */
    double m_darkNoise = 0;            /**< uniform dark noise (hits per bar) */
    double m_ADCx0 = 0; /**< pulse height distribution parameter [ADC counts] */
    double m_ADCp1 = 0; /**< pulse height distribution parameter, must be non-negative */
    double m_ADCp2 = 0; /**< pulse height distribution parameter, must be positive */
    double m_ADCmax = 0; /**< pulse height upper bound of range [ADC counts] */
    double m_rmsNoise = 0; /**< r.m.s of noise [ADC counts]*/
    int m_threshold = 0; /**< pulse height threshold [ADC counts] */
    int m_hysteresis = 0; /**< pulse height threshold hysteresis [ADC counts] */
    int m_thresholdCount = 0; /**< minimal number of samples above threshold */
    bool m_useWaveforms = false; /**< if true, use full waveform digitization */
    bool m_useDatabase = false;  /**< if true, use calibration constants from database */
    bool m_useSampleTimeCalibration = false;   /**< if true, use time base calibration */
    bool m_simulateTTS = true; /**< if true, add TTS to simulated hits */
    bool m_allChannels = false; /**< if true, always make waveforms for all channels */
    unsigned m_storageDepth = 0;           /**< ASIC analog storage depth */

    // datastore objects
    StoreArray<TOPSimHit> m_simHits;        /**< collection of simuated hits */
    StoreArray<TOPSimCalPulse> m_simCalPulses; /**< collection of simuated cal pulses */
    StoreArray<MCParticle> m_mcParticles;   /**< collection of MC particles */
    StoreArray<TOPRawWaveform> m_waveforms; /**< collection of waveforms */
    StoreArray<TOPRawDigit> m_rawDigits;    /**< collection of raw digits */
    StoreArray<TOPDigit> m_digits;          /**< collection of digits */

    // constants from conditions DB
    DBObjPtr<TOPCalTimebase>* m_timebases = 0; /**< sample times from database */
    DBObjPtr<TOPCalChannelPulseHeight>* m_pulseHeights = 0; /**< pulse height param. */
    DBObjPtr<TOPCalChannelThreshold>* m_thresholds = 0; /**< channel thresholds */
    DBObjPtr<TOPCalChannelNoise>* m_noises = 0; /**< channel noise levels (r.m.s) */

    // default for no DB or calibration not available
    TOPSampleTimes m_sampleTimes; /**< equidistant sample times */
    TOP::PulseHeightGenerator m_pulseHeightGenerator; /**< default generator */

    // other
    double m_timeMin = 0; /**< time range limit: minimal time */
    double m_timeMax = 0; /**< time range limit: maximal time */



  };

} // Belle2 namespace

