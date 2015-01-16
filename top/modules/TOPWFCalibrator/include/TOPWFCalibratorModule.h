/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPWFCALIBRATORMODULE_H
#define TOPWFCALIBRATORMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <TProfile.h>

namespace Belle2 {

  /**
   * Calibration of waveforms (pedestals, gains, time axis)
   */
  class TOPWFCalibratorModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPWFCalibratorModule();

    /**
     * Destructor
     */
    virtual ~TOPWFCalibratorModule();

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

    enum {c_NumChannels = 512, c_NumWindows = 512};

    std::string m_outputFileName; /**< output file name (root file) */
    int m_barID; /**< ID of TOP module to calibrate */

    TProfile* m_profile[c_NumChannels][c_NumWindows]; /**< profile histograms */


  };

} // Belle2 namespace

#endif
