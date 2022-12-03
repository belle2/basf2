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
#include <TH1.h>
#include <TString.h>
#include <TLine.h>
#include <TPaveText.h>

namespace Belle2 {
  /**
   * Class for TOP histogram analysis.
   */

  class DQMHistAnalysisTOPModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisTOPModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisTOPModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;

    //! Data members
  private:
    /** Histogram from DQMInfo with run type. */
    TH1* m_RunType = nullptr;

    /** String with run type. */
    TString m_RunTypeString;

    /** Run type flag for null runs. */
    bool m_IsNullRun;

    /** Canvas for the mean of the good hits. */
    TCanvas* m_c_goodHitsMean = nullptr;
    /** Canvas for the RMS of the good hits. */
    TCanvas* m_c_goodHitsRMS = nullptr;
    /** Canvas for the mean of the bad hits. */
    TCanvas* m_c_badHitsMean = nullptr;
    /** Canvas for the RMS of the bad hits. */
    TCanvas* m_c_badHitsRMS = nullptr;

    /** Canvas for 16 good Hits XY */
    TCanvas* m_c_good_hits_xy_[17] = {};
    /** Canvas for 16 bad Hits XY */
    TCanvas* m_c_bad_hits_xy_[17] = {};
    /** Canvas for 16 good Hits ASICS */
    TCanvas* m_c_good_hits_asics_[17] = {};
    /** Canvas for 16 bad Hits ASICS */
    TCanvas* m_c_bad_hits_asics_[17] = {};

    /** Histogram for the mean of the good hits. */
    TH1F* m_h_goodHitsMean = nullptr;
    /** Histogram for the RMS of the good hits. */
    TH1F* m_h_goodHitsRMS = nullptr;
    /** Histogram for the mean of the bad hits. */
    TH1F* m_h_badHitsMean = nullptr;
    /** Histogram for the RMS of the bad hits. */
    TH1F* m_h_badHitsRMS = nullptr;

    /** The line for the upper bound of the nornal window. */
    TLine* m_line1 = nullptr;
    /** The line for the lower bound of the nornal window. */
    TLine* m_line2 = nullptr;
    /** The text for the conditions of the nornal window. */
    TPaveText* m_text1 = nullptr;
    /** The text for the conditions of the nornal window. */
    TPaveText* m_text2 = nullptr;

    /** Monitoring object. */
    MonitoringObject* m_monObj {};

  };
} // end namespace Belle2

