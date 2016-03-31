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
#include <top/geometry/TOPGeometryPar.h>
#include <string>
#include <TProfile.h>


namespace Belle2 {

  /**
   * Time base calibrator (under development)
   */
  class TOPTimeBaseCalibratorModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPTimeBaseCalibratorModule();

    /**
     * Destructor
     */
    virtual ~TOPTimeBaseCalibratorModule();

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
     * number modules, channels per module, samples per channel
     */
    enum {c_NumModules = 16,
          c_NumChannels = 512,
          c_NumSamples = 256
         };

    std::string m_histogramFileName; /**< output file name for histograms */
    double m_timeCutLow; /**< time cut: low */
    double m_timeCutHigh; /**< time cut: high */
    double m_adcCutLow; /**< pulse-height (or Q) cut: low */
    double m_adcCutHigh; /**< pulse-height (or Q) cut: high */
    int m_calChannel; /**< calibration channel */

    TProfile* m_profile[c_NumModules][c_NumChannels]; /**< profile histograms */

    /** geometry parameters */
    TOP::TOPGeometryPar* m_topgp = TOP::TOPGeometryPar::Instance();


  };

} // Belle2 namespace

