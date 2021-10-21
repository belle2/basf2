/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TCanvas.h>

namespace Belle2 {

  /** Analysis of HLT Tracking DQM plots */
  class DQMHistAnalysisTrackingHLTModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    /** Constructor */
    DQMHistAnalysisTrackingHLTModule();
    /**Destructor */
    ~DQMHistAnalysisTrackingHLTModule();

    /** Module function initialize */
    void initialize() override;
    /** Module function beginRun */
    void beginRun() override;
    /** Module function event */
    void event() override;
    /** Module function endRun */
    void endRun() override;
    /** Module function terminate */
    void terminate() override;

    // parameters
    bool m_printCanvas = false; /**< if true print the pdf of the canvases */

    int m_statThreshold = 1000; /**< minimal number of events to judge */
    double m_failureRateThreshold = 1; /**< above this rate, there is maybe a problem?*/

  private:

    /** Monitoring Object to be produced by this module, which contain defined canvases and monitoring variables */
    MonitoringObject* m_monObj = nullptr;

    TH1* m_rtype = nullptr; /**< histogram from DQMInfo with runtype */
    TString m_runtype = nullptr; /**< string with runtype: physics or cosmic */

    TCanvas* m_cAbortRate = nullptr;  /**< canvas for the abort rate plot */
  };
} // end namespace Belle2

