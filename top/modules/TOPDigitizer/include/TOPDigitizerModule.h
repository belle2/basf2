/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
#include <simulation/dataobjects/SimClockState.h>

#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPSampleTimes.h>
#include <top/dbobjects/TOPCalTimebase.h>
#include <top/dbobjects/TOPCalChannelT0.h>
#include <top/dbobjects/TOPCalAsicShift.h>
#include <top/dbobjects/TOPCalModuleT0.h>
#include <top/dbobjects/TOPCalCommonT0.h>
#include <top/dbobjects/TOPCalChannelPulseHeight.h>
#include <top/dbobjects/TOPCalChannelThreshold.h>
#include <top/dbobjects/TOPCalChannelNoise.h>
#include <top/dbobjects/TOPFrontEndSetting.h>
#include <top/dbobjects/TOPCalTimeWalk.h>

#include <top/modules/TOPDigitizer/PulseHeightGenerator.h>


namespace Belle2 {

  /**
   * TOP digitizer.
   * This module takes hits form G4 simulation (TOPSimHits),
   * applies TTS, T0 jitter and does spatial and time digitization.
   * Output to TOPDigits.
   */
  class TOPDigitizerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPDigitizerModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    /**
     * Utility structure for time offset
     */
    struct TimeOffset {
      double value = 0; /**< value */
      double error = 0; /**< error squared */
      int windowShift = 0; /**< number of shifted windows */
      double timeShift = 0; /**< shift expressed in time */
      /**
       * Full constructor
       */
      TimeOffset(double v, double e, int n, double t):
        value(v), error(e), windowShift(n), timeShift(t)
      {}
    };

    /**
     * Returns a complete time offset by adding time mis-calibration to trgOffset
     * @param trgOffset trigger related time offset
     * @param moduleID slot ID
     * @param pixelID pixel ID
     * @return time offset and its error squared
     */
    TimeOffset getTimeOffset(double trgOffset, int moduleID, int pixelID);

    /**
     * Generates and returns pulse height
     * @param moduleID module ID (1-based)
     * @param pixelID pixel ID (1-based)
     * @return pulse height [ADC counts]
     */
    double generatePulseHeight(int moduleID, int pixelID) const;

    // module steering parameters
    double m_timeZeroJitter;       /**< r.m.s of T0 jitter */
    double m_electronicJitter;     /**< r.m.s of electronic jitter */
    double m_darkNoise;            /**< uniform dark noise (hits per bar) */
    double m_ADCx0; /**< pulse height distribution parameter [ADC counts] */
    double m_ADCp1; /**< pulse height distribution parameter, must be non-negative */
    double m_ADCp2; /**< pulse height distribution parameter, must be positive */
    double m_ADCmax; /**< pulse height upper bound of range [ADC counts] */
    double m_rmsNoise; /**< r.m.s of noise [ADC counts]*/
    int m_threshold; /**< pulse height threshold [ADC counts] */
    int m_hysteresis; /**< pulse height threshold hysteresis [ADC counts] */
    int m_thresholdCount; /**< minimal number of samples above threshold */
    bool m_useWaveforms; /**< if true, use full waveform digitization */
    bool m_useDatabase;  /**< if true, use calibration constants from database */
    bool m_useSampleTimeCalibration;   /**< if true, use time base calibration */
    bool m_simulateTTS; /**< if true, add TTS to simulated hits */
    bool m_allChannels; /**< if true, always make waveforms for all channels */
    double m_minWidthXheight;  /**< minimal width * height [ns * ADC counts] */
    int m_lookBackWindows;     /**< number of "look back" windows */

    // datastore objects
    StoreArray<TOPSimHit> m_simHits;        /**< collection of simuated hits */
    StoreArray<TOPSimCalPulse> m_simCalPulses; /**< collection of simuated cal pulses */
    StoreArray<MCParticle> m_mcParticles;   /**< collection of MC particles */
    StoreObjPtr<SimClockState> m_simClockState; /**< generated hardware clock state */
    StoreArray<TOPRawWaveform> m_waveforms; /**< collection of waveforms */
    StoreArray<TOPRawDigit> m_rawDigits;    /**< collection of raw digits */
    StoreArray<TOPDigit> m_digits;          /**< collection of digits */

    // constants from conditions DB
    DBObjPtr<TOPCalTimebase> m_timebases; /**< sample times from database */
    DBObjPtr<TOPCalChannelT0> m_channelT0; /**< channel T0 calibration constants */
    DBObjPtr<TOPCalAsicShift> m_asicShift; /**< ASIC shifts calibration constants */
    DBObjPtr<TOPCalModuleT0> m_moduleT0;   /**< module T0 calibration constants */
    DBObjPtr<TOPCalCommonT0> m_commonT0;   /**< common T0 calibration constants */
    DBObjPtr<TOPCalChannelPulseHeight> m_pulseHeights; /**< pulse height param. */
    DBObjPtr<TOPCalChannelThreshold> m_thresholds; /**< channel thresholds */
    DBObjPtr<TOPCalChannelNoise> m_noises; /**< channel noise levels (r.m.s) */
    OptionalDBObjPtr<TOPCalTimeWalk> m_timeWalk;   /**< time-walk calibration constants */
    DBObjPtr<TOPFrontEndSetting> m_feSetting;   /**< front-end settings */

    // default for no DB or calibration not available
    TOPSampleTimes m_sampleTimes; /**< equidistant sample times */
    TOP::PulseHeightGenerator m_pulseHeightGenerator; /**< default generator */

    // other
    double m_syncTimeBase = 0; /**< SSTin period */

  };

} // Belle2 namespace

