/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <vector>

#include <framework/datastore/StoreArray.h>
#include <top/dataobjects/TOPRawDigit.h>
#include <top/dataobjects/TOPDigit.h>

#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPCalTimebase.h>
#include <top/dbobjects/TOPCalChannelT0.h>
#include <top/dbobjects/TOPCalModuleT0.h>
#include <top/dbobjects/TOPCalCommonT0.h>


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

  private:

    // steering parameters
    std::string m_inputRawDigitsName;  /**< name of TOPRawDigit store array */
    std::string m_outputDigitsName;    /**< name of TOPDigit store array */
    bool m_useSampleTimeCalibration;   /**< if true, use sample time calibration */
    bool m_useChannelT0Calibration;    /**< if true, use channel T0 calibration */
    bool m_useModuleT0Calibration;     /**< if true, use module T0 calibration */
    bool m_useCommonT0Calibration;     /**< if true, use common T0 calibration */
    bool m_subtractOffset;             /**< if true, subtract offset of nominal TDC */
    double m_pedestalRMS;              /**< r.m.s of pedestals [ADC counts] */
    double m_maxPulseWidth;            /**< max pulse width to set digit as good [ns] */
    unsigned m_storageDepth;           /**< ASIC analog storage depth */
    int m_lookBackWindows;             /**< number of "look back" windows */
    bool m_setPhase;                   /**< if true, set phase in TOPRawDigits */

    int m_calibrationChannel;   /**< ASIC channel number with calibration pulse */
    double m_calpulseWidthMin;  /**< minimal width of calibration pulse */
    double m_calpulseWidthMax;  /**< maximal width of calibration pulse */
    int m_calpulseHeightMin;    /**< minimal height of calibration pulse */
    int m_calpulseHeightMax;    /**< maximal height of calibration pulse */

    // time calibration
    DBObjPtr<TOPCalTimebase>* m_timebase = 0;   /**< sample time calibration constants */
    DBObjPtr<TOPCalChannelT0>* m_channelT0 = 0; /**< channel T0 calibration constants */
    DBObjPtr<TOPCalModuleT0>* m_moduleT0 = 0;   /**< module T0 calibration constants */
    DBObjPtr<TOPCalCommonT0>* m_commonT0 = 0;   /**< common T0 calibration constants */
    TOPSampleTimes m_sampleTimes; /**< equidistant in case no calibration required */

    // collections
    StoreArray<TOPRawDigit> m_rawDigits; /**< collection of raw digits */
    StoreArray<TOPDigit> m_digits;       /**< collection of digits */

    // other
    std::vector<int> m_writeDepths;  /**< write depths of production debug format */
    double m_syncTimeBase = 0; /**< SSTin period */

  };

} // Belle2 namespace

