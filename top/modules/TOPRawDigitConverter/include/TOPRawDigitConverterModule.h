/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <top/dataobjects/TOPRawDigit.h>
#include <top/dataobjects/TOPProductionEventDebug.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPAsicMask.h>

#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPCalTimebase.h>
#include <top/dbobjects/TOPCalChannelT0.h>
#include <top/dbobjects/TOPCalAsicShift.h>
#include <top/dbobjects/TOPCalModuleT0.h>
#include <top/dbobjects/TOPCalCommonT0.h>
#include <top/dbobjects/TOPCalChannelNoise.h>
#include <top/dbobjects/TOPFrontEndSetting.h>
#include <top/dbobjects/TOPCalTimeWalk.h>

namespace Belle2 {

  /**
   * TOPRawDigits to TOPDigits converter. Applies also time calibration.
   */
  class TOPRawDigitConverterModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPRawDigitConverterModule();

    /**
     * Destructor
     */
    virtual ~TOPRawDigitConverterModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

  private:

    // steering parameters
    std::string m_inputRawDigitsName;  /**< name of TOPRawDigit store array */
    std::string m_outputDigitsName;    /**< name of TOPDigit store array */
    bool m_useSampleTimeCalibration;   /**< if true, use sample time calibration */
    bool m_useAsicShiftCalibration;    /**< if true, use asic shifts calibration */
    bool m_useChannelT0Calibration;    /**< if true, use channel T0 calibration */
    bool m_useModuleT0Calibration;     /**< if true, use module T0 calibration */
    bool m_useCommonT0Calibration;     /**< if true, use common T0 calibration */
    bool m_useTimeWalkCalibration;     /**< if true, use time-walk calibration */
    double m_pedestalRMS;              /**< r.m.s of pedestals [ADC counts] */
    double m_minPulseWidth;            /**< min pulse width to set digit as good [ns] */
    double m_maxPulseWidth;            /**< max pulse width to set digit as good [ns] */
    double m_minWidthXheight;          /**< minimal width * height [ns * ADC counts] */
    unsigned m_storageDepth;           /**< ASIC analog storage depth */
    int m_lookBackWindows;             /**< number of "look back" windows */
    bool m_setPhase;                   /**< if true, set phase in TOPRawDigits */

    int m_calibrationChannel;   /**< ASIC channel number with calibration pulse */
    double m_calpulseWidthMin;  /**< minimal width of calibration pulse */
    double m_calpulseWidthMax;  /**< maximal width of calibration pulse */
    int m_calpulseHeightMin;    /**< minimal height of calibration pulse */
    int m_calpulseHeightMax;    /**< maximal height of calibration pulse */
    double m_calpulseTimeMin;  /**< minimal time of calibration pulse */
    double m_calpulseTimeMax;  /**< maximal time of calibration pulse */

    bool m_addRelations = false;  /**< switch ON/OFF relations to TOPRawDigits */

    // front-end settings (lookback, storage depths etc)
    DBObjPtr<TOPFrontEndSetting> m_feSetting;   /**< front-end settings */

    // time calibration
    DBObjPtr<TOPCalTimebase> m_timebase;   /**< sample time calibration constants */
    DBObjPtr<TOPCalChannelT0> m_channelT0; /**< channel T0 calibration constants */
    DBObjPtr<TOPCalAsicShift> m_asicShift; /**< ASIC shifts calibration constants */
    DBObjPtr<TOPCalModuleT0> m_moduleT0;   /**< module T0 calibration constants */
    DBObjPtr<TOPCalCommonT0> m_commonT0;   /**< common T0 calibration constants */
    OptionalDBObjPtr<TOPCalTimeWalk> m_timeWalk;   /**< time-walk calibration constants */
    TOPSampleTimes m_sampleTimes; /**< equidistant in case no calibration required */

    // r.m.s of pedestals (noise) of individual channels
    DBObjPtr<TOPCalChannelNoise> m_noises; /**< r.m.s. of pedestals [ADC counts] */

    // collections
    StoreArray<TOPRawDigit> m_rawDigits; /**< collection of raw digits */
    StoreArray<TOPProductionEventDebug> m_eventDebugs; /**< collection of debug data */
    StoreArray<TOPDigit> m_digits;       /**< collection of digits */
    StoreObjPtr<TOPAsicMask> m_asicMask; /**< masked asics in firmware */

    // other
    double m_syncTimeBase = 0; /**< SSTin period */

  };

} // Belle2 namespace

