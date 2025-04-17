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
  class DQMHistAnalysisTrackingAbortModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /** Constructor */
    DQMHistAnalysisTrackingAbortModule();
    /**Destructor */
    ~DQMHistAnalysisTrackingAbortModule() {};

    /** Module function initialize */
    void initialize() override final;
    /** Module function event */
    void event() override final;
    /** Module function doing stuff at beginning of a run */
    void beginRun() override final;
    /** Module function needed to delete pointer*/
    void terminate() override final;

  private:

    bool m_printCanvas = false; /**< if true print the pdf of the canvases */

    int m_statThreshold = 1000; /**< minimal number of events to judge */
    double m_failureRateThreshold = 0.01; /**< above this rate, there is maybe a problem?*/

    /** Monitoring Object to be produced by this module, which contain defined canvases and monitoring variables */
    MonitoringObject* m_monObj = nullptr;

    //abort canvases
    TCanvas* m_cAbortRate = nullptr;  /**< canvas for the abort rate plot */
    TCanvas* m_cAbortRateIN = nullptr;  /**< canvas for the abort rate inside the active veto region */
    TCanvas* m_cAbortRateOUT = nullptr;  /**< canvas for the abort rate outside the active veto region */
    TCanvas* m_cAbortRate_BF = nullptr;  /**< canvas for the abort rate plot BEFORE FILTER*/
    TCanvas* m_cAbortRateIN_BF = nullptr;  /**< canvas for the abort rate inside the active veto region BEFORE FILTER*/
    TCanvas* m_cAbortRateOUT_BF = nullptr;  /**< canvas for the abort rate outside the active veto region BEFORE FILTER*/

    TH1F* m_hAbort = nullptr; /**< totla abort (inside + outside) active veto region */
    TH1F* m_hAbort_BF = nullptr; /**< totla abort (inside + outside) active veto region BEFORE FILTER*/

    /** scale hAverage and send bin contents to Mirabelle */
    void scaleAndSendToMirabelle(TH1F* hAverage, const int nEvents, const TString& tag);
  };
} // end namespace Belle2

