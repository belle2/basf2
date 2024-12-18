/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPeak.h
// Description : Simple Peak Analysis (Mean/Media/Width) for simple peaked distributions with delta histogramming
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>
#include <TGraph.h>
#include <TLine.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for simple peaking distributions */

  class DQMHistAnalysisPeakModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisPeakModule();

  private:

    /**
     * Initializer.
     */
    void initialize() override final;

    /**
     * Terminate.
     */
    void terminate() override final;

    /**
     * Called when entering a new run.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    // Data members
  private:
    /** The name of the histogram. */
    std::string m_histoName;
    /** The name of the histogram dir. */
    std::string m_histoDirectory;
    /** The prefix of PV. */
    std::string  m_pvPrefix;
    /** The prefix for MonitoringObj. */
    std::string  m_monPrefix;
    /** The Name for MonitoringObj. */
    std::string  m_monObjectName;
    /** Update entry interval */
    int m_minEntries = 1000;
    /** Flag: plot into canvas */
    bool m_plot = true;
    /** Zoom on peak with range +- zoom*rms ; 0 disable */
    float m_zoom = 3.0;
    /** Flag: extract mean */
    bool m_mean = true;
    /** Flag: extract median */
    bool m_median = true;
    /** Flag: extract rms */
    bool m_rms = true;
    /** Flag for plotting limits from PV */
    bool m_plotLimits = true;

    /** The drawing canvas. */
    TCanvas* m_canvas = nullptr;
    /** The line for the mean result. */
    TLine* m_lineMean = nullptr;
    /** The line for the median result. */
    TLine* m_lineMedian = nullptr;

    /** Monitoring Object */
    MonitoringObject* m_monObj {};

    /** Graph for Limit plot */
    TGraph* m_g_alarmlo{};
    /** Graph for Limit plot */
    TGraph* m_g_warnlo{};
    /** Graph for Limit plot */
    TGraph* m_g_good{};
    /** Graph for Limit plot */
    TGraph* m_g_warnhi{};
    /** Graph for Limit plot */
    TGraph* m_g_alarmhi{};

    /** warn/alarm limit */
    double m_lowarnlevel = NAN;
    /** warn/alarm limit */
    double m_hiwarnlevel = NAN;
    /** warn/alarm limit */
    double m_loerrorlevel = NAN;
    /** warn/alarm limit */
    double m_hierrorlevel = NAN;

    /** flag for valid limit/graph */
    bool m_valid_alarmlo = false;
    /** flag for valid limit/graph */
    bool m_valid_warnlo = false;
    /** flag for valid limit/graph */
    bool m_valid_good = false;
    /** flag for valid limit/graph */
    bool m_valid_warnhi = false;
    /** flag for valid limit/graph */
    bool m_valid_alarmhi = false;

  };
} // end namespace Belle2

