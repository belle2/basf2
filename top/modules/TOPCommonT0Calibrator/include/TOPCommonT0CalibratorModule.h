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
#include <TH1F.h>

namespace Belle2 {

  /**
   * Structure to hold value and error
   */
  struct PointWithError {
    double value = 0; /**< central value */
    double error = 0; /**< uncertainty */

    /**
     * Constructor with value and error
     * @param value central value
     * @param error uncertainty
     */
    PointWithError(double val, double err): value(val), error(err)
    {}
  };


  /**
   * On-line calibration of common T0
   */
  class TOPCommonT0CalibratorModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPCommonT0CalibratorModule();

    /**
     * Destructor
     */
    virtual ~TOPCommonT0CalibratorModule();

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
     * Return parabolic maximum
     * @param i0 index of the bin with the maximal value
     * @return position of the maximum with uncertainty
     */
    PointWithError getParabolicMaximum(unsigned i0);

    /**
     * Return parabolic maximum
     * @param yLeft bin content of left-to-maximal bin
     * @param yCenter bin content of maximal bin
     * @param yRight bin content of right-to-maximal bin
     * @return a fraction of step to be added to central bin position + error
     */
    PointWithError getParabolicMaximum(double yLeft, double yCenter, double yRight);

    int m_numBins;      /**< number of bins to which time range is divided */
    double m_timeRange; /**< time range in which to search [ns] */
    double m_minTime;   /**< lower time limit for photons [ns] */
    double m_maxTime;   /**< upper time limit for photons [ns] */
    int m_numEvents;    /**< number of events to merge */

    int m_iEvent = 0; /**< event count */
    std::vector<double> m_logLikelihood; /**< container for the sum of log likelihoods */
    std::vector<double> m_t0;  /**< container for time axis */
    double m_dt = 0;  /**< bin size */

    std::vector<TH1F*> m_histograms; /**< container for histograms */


  };

} // Belle2 namespace

