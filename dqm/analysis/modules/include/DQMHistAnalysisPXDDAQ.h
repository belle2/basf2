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

    //! Histogram covering all modules
    TH1F* m_hMissingDHP = nullptr;
    //! Final Canvas
    TCanvas* m_cMissingDHP = nullptr;


    /** Monitoring Object */
    MonitoringObject* m_monObj {};

#ifdef _BELLE2_EPICS
    //! one EPICS PV
    chid  mychid;
#endif
  };
} // end namespace Belle2

