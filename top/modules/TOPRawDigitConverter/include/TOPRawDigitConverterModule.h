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
#include <map>

#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPCalTimebase.h>


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

    std::string m_inputRawDigitsName;  /**< name of TOPRawDigit store array */
    std::string m_outputDigitsName;    /**< name of TOPDigit store array */
    bool m_useSampleTimeCalibration;   /**< use sample time calibration, if true */

    DBObjPtr<TOPCalTimebase> m_timebase;  /**< sample time calibration */
    TOPSampleTimes m_sampleTimes; /**< equidistant in case no calibration required */
    int m_sampleDivisions = 0; /**< number of sample divisions (from TDC subbits) */

  };

} // Belle2 namespace

