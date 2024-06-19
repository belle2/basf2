/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <TCanvas.h>

namespace Belle2 {

  /** Analysis of HLT Tracking DQM plots */
  class DQMHistAnalysisTrackingHLTModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /** Constructor */
    DQMHistAnalysisTrackingHLTModule();
    /**Destructor */
    ~DQMHistAnalysisTrackingHLTModule() {};

    /** Module function initialize */
    void initialize() override final;
    /** Module function event */
    void event() override final;
    /** Module function doing stuff at beginning of a run */
    void beginRun() override final;

    // parameters
    bool m_printCanvas = false; /**< if true print the pdf of the canvases */

  private:

    int m_statThreshold = 1000; /**< minimal number of events to judge */

    /** Monitoring Object to be produced by this module, which contain defined canvases and monitoring variables */
    MonitoringObject* m_monObj = nullptr;

    TCanvas* m_cAbortRateHER = nullptr;  /**< canvas for the 2D abort rate plot for HER*/
    TCanvas* m_cAbortRateLER = nullptr;  /**< canvas for the 2D abort rate plot for LER*/
  };
} // end namespace Belle2

