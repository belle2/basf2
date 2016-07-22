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

#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPCalTimebase.h>
#include <top/dbobjects/TOPCalChannelT0.h>
#include <topcaf/dataobjects/TOPCAFDigit.h>

namespace Belle2 {

  /**
   * TOPCAFDigits to TOPDigits converter. Applies also time calibration.
   */
  class TOPCAFtoTOPDigitModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPCAFtoTOPDigitModule();

    /**
     * Destructor
     */
    virtual ~TOPCAFtoTOPDigitModule();

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

    /**
     * Select topcaf digit according to selection criteria
     * @return true, when selected
     */
    bool selectDigit(const TOPCAFDigit& cafDigit);

    bool m_useSampleTimeCalibration;   /**< if true, use sample time calibration */
    bool m_useChannelT0Calibration;    /**< if true, use channel T0 calibration */
    bool m_selectGood;   /**< if true, select only hits with two cal pulses found */

    DBObjPtr<TOPCalTimebase>* m_timebase = 0;   /**< sample time calibration constants */
    DBObjPtr<TOPCalChannelT0>* m_channelT0 = 0; /**< channel T0 calibration constants */
    TOPSampleTimes m_sampleTimes; /**< equidistant in case no calibration required */
    int m_sampleDivisions = 0; /**< number of sample divisions (from TDC subbits) */

  };

} // Belle2 namespace

