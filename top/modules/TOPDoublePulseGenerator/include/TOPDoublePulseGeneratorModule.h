/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <top/dbobjects/TOPSampleTimes.h>
#include <top/dbobjects/TOPCalTimebase.h>
#include <framework/database/DBObjPtr.h>
#include <vector>
#include <string>

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

    /**
     * Optionally store sample times used by the generator as root histograms
     * fileName root output file name
     */
    void storeSampleTimes(std::string fileName);

    /**
     * Save vector to histogram and write it out
     * @param vec vector of bin values
     * @param name histogram name
     * @param title histogram title
     * @param xTitle x-axis title
     * @param yTitle y-axis title
     */
    void saveAsHistogram(const std::vector<double>& vec,
                         const std::string& name,
                         const std::string& title,
                         const std::string& xTitle = "",
                         const std::string& yTitle = "") const;

    // module steering parameters

    std::vector<int> m_moduleIDs; /**< slot ID's to generate for */
    std::vector<unsigned> m_asicChannels; /**< ASIC calibration channels */
    double m_timeDifference = 0; /**< time difference between first and second pulse */
    double m_timeResolution = 0; /**< sigma of time difference */
    std::vector<double> m_sampleTimeIntervals; /**< sample time intervals */
    bool m_useDatabase = false;  /**< if true, use sample times from database */
    unsigned m_storageWindows = 0; /**< number of storage windows */
    std::string m_outputFileName; /**< if given, store sample times as root histograms */

    // other

    TOPSampleTimes m_sampleTimes; /**< sample times from steering input */
    DBObjPtr<TOPCalTimebase>* m_timebase = 0; /**< sample times from database */
    int m_sampleDivisions = 0; /**< number of sample divisions (from NominalTDC) */

  };

} // Belle2 namespace

