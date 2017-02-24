/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <top/dbobjects/TOPSampleTimes.h>
#include <top/dbobjects/TOPCalTimebase.h>
#include <framework/database/DBObjPtr.h>
#include <vector>

namespace Belle2 {

  /**
   * Generator of double calibration pulses
   * Output to TOPDigits
   *
   * first version: simple generator (no waveforms)
   */
  class TOPDoublePulseGeneratorModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPDoublePulseGeneratorModule();

    /**
     * Destructor
     */
    virtual ~TOPDoublePulseGeneratorModule();

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

    // module steering parameters
    int m_moduleID = 0;          /**< slot ID */
    unsigned m_asicChannel = 0;  /**< ASIC channel with cal pulse */
    double m_timeDifference = 0; /**< time difference between first and second pulse */
    double m_timeResolution = 0; /**< sigma of time difference */
    std::vector<double> m_sampleTimeIntervals; /**< sample time intervals */
    bool m_useDatabase = false;  /**< if true, use sample times from database */

    // other
    std::vector<int> m_moduleIDs; /**< slot ID's to generate for */
    std::vector<unsigned> m_asicChannels; /**< ASIC calibration channels */
    TOPSampleTimes m_sampleTimes; /**< sample times from steering input */
    DBObjPtr<TOPCalTimebase>* m_timebase = 0; /**< sample times from database */
    int m_sampleDivisions = 0; /**< number of sample divisions (from NominalTDC) */

  };

} // Belle2 namespace

