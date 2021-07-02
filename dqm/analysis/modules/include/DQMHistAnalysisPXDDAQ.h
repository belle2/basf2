//+
// File : DQMHistAnalysisPXDDAQ.h
// Description : DAQM Analysis for PXD Data DAQ
//
// Author : Bjoern Spruck, University Mainz
// Date : 2018
//-

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD DAQ */

  class DQMHistAnalysisPXDDAQModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor
    DQMHistAnalysisPXDDAQModule();
    //! Destructor
    ~DQMHistAnalysisPXDDAQModule();
  private:

    //! Module functions to be called from main process
    void initialize(void) override final;

    //! Module functions to be called from event process
    void beginRun(void) override final;
    void event(void) override final;
    void terminate(void) override final;

    // Data members
    //! name of histogram directory
    std::string m_histogramDirectoryName;
    //! prefix for EPICS PVs
    std::string m_pvPrefix;

    //! Histogram covering all error types
    TH1* m_hDAQError = nullptr;
    //! Histogram covering all DHC modules
    TH2F* m_hMissingDHC = nullptr;
    //! Histogram covering all DHE modules
    TH2F* m_hMissingDHE = nullptr;
    //! Histogram covering all modules*DHP
    TH1F* m_hMissingDHP = nullptr;
    //! Histogram covering stat
    TH1F* m_hStatistic = nullptr;
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


    /** Monitoring Object */
    MonitoringObject* m_monObj {};

    /** flag if to export to EPICS */
    bool m_useEpics;

#ifdef _BELLE2_EPICS
    //! one EPICS PV
    std::vector <chid> mychid;
#endif
  };
} // end namespace Belle2

