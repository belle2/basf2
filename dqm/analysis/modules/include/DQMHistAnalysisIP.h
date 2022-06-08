/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisIP.h
// Description : Median for IP position with delta histogramming
//-

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <framework/core/Module.h>

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TCanvas.h>
#include <TLine.h>
#include <TH1.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisIPModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisIPModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisIPModule();

    /**
     * Initializer.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override final;

    // Data members
  private:
    /** The name of the histogram. */
    std::string m_histoname;
    /** The prefix of PV. */
    std::string  m_pvPrefix;
    /** The prefix for MonitoringObj. */
    std::string  m_monPrefix;
    /** Update entry intervall */
    int m_minEntries = 1000;

    /** The drawing canvas. */
    TCanvas* m_c1 = nullptr;
    /** The line for the mean result. */
    TLine* m_line = nullptr;
    /** The line for the median result. */
    TLine* m_line2 = nullptr;

    /** last histogram */
    TH1* m_h_last = nullptr;

    /** Monitoring Object */
    MonitoringObject* m_monObj {};

    /** flag if to export to EPICS */
    bool m_useEpics;
    /** number of EPICS PVs */
    static const int m_parameters = 3;
#ifdef _BELLE2_EPICS
    chid mychid[m_parameters];// hard limit max parameters
#endif
  };
} // end namespace Belle2

