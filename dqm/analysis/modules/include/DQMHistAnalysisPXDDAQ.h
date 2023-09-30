/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPXDDAQ.h
// Description : DAQM Analysis for PXD Data DAQ
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <TH2F.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD DAQ */

  class DQMHistAnalysisPXDDAQModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisPXDDAQModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisPXDDAQModule();

    /**
     * Initializer.
     */
    void initialize(void) override final;

    /**
     * Called when entering a new run.
     */
    void beginRun(void) override final;

    /**
     * This method is called for each event.
     */
    void event(void) override final;

  private:

    // Data members
    //! name of histogram directory
    std::string m_histogramDirectoryName;
    /** Update entry intervall */
    int m_minEntries = 10000;

    //! Histogram covering all error types
    TH1* m_hDAQError = nullptr;
    //! Histogram covering all DHC modules
    TH2F* m_hMissingDHC = nullptr;
    //! Histogram covering all DHE modules
    TH2F* m_hMissingDHE = nullptr;
    //! Histogram covering all modules*DHP
    TH1F* m_hMissingDHP = nullptr;
    //! Histogram covering stat
    TH1D* m_hStatistic = nullptr;
    //! Final Canvas
    TCanvas* m_cDAQError = nullptr;
    //! Final Canvas
    TCanvas* m_cMissingDHC = nullptr;
    //! Final Canvas
    TCanvas* m_cMissingDHE = nullptr;
    //! Final Canvas
    TCanvas* m_cMissingDHP = nullptr;
    //! Final Canvas
    TCanvas* m_cStatistic = nullptr;
    //! Final Canvas
    TCanvas* m_cStatisticUpd = nullptr;


    /** Monitoring Object */
    MonitoringObject* m_monObj {};
  };
} // end namespace Belle2

