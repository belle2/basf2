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
#include <string>

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
   * Event T0 finder for global cosmic runs
   */
  class TOPCosmicT0FinderModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPCosmicT0FinderModule();

    /**
     * Destructor
     */
    virtual ~TOPCosmicT0FinderModule();

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
     * Return parabolic minimum
     * @param i0 index of the bin with the minimal value
     * @return position of the minimum with uncertainty
     */
    PointWithError getParabolicMinimum(unsigned i0);

    /**
     * Return parabolic minimum
     * @param yLeft bin content of left-to-minimal bin
     * @param yCenter bin content of minimal bin
     * @param yRight bin content of right-to-minimal bin
     * @return a fraction of step to be added to central bin position + error
     */
    PointWithError getParabolicMinimum(double yLeft, double yCenter, double yRight);

    bool m_useIncomingTrack; /**< if true use incoming track, otherwise use outcoming */
    unsigned m_minHits; /**< minimal number of hits on TOP module */
    bool m_applyT0; /**< if true, subtract T0 in TOPDigits */
    int m_numBins;      /**< number of bins to which time range is divided */
    double m_timeRange; /**< time range in which to search [ns] */
    double m_sigma;     /**< additional time spread added to PDF [ns] */
    bool m_saveHistograms; /**< flag to save histograms */
    double m_bkgPerModule; /**< average background hits per module */

    std::vector<double> m_chi2s; /**< container for chi^2 */
    std::vector<double> m_t0;  /**< container for time axis */
    double m_dt = 0;  /**< bin size */
    int m_num = 0; /**< histogram number */

  };

} // Belle2 namespace

