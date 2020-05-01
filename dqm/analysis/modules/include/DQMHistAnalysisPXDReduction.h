//+
// File : DQMHistAnalysisPXDReduction.h
// Description : DAQM Analysis for PXD Data Reduction
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
#include <vxd/dataobjects/VxdID.h>

#include <TH2F.h>
#include <TCanvas.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Reduction */

  class DQMHistAnalysisPXDReductionModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor
    DQMHistAnalysisPXDReductionModule();
    //! Destructor
    ~DQMHistAnalysisPXDReductionModule();
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

    //! IDs of all PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    //! Histogram covering all modules
    TH1F* m_hReduction = nullptr;
    //! Final Canvas
    TCanvas* m_cReduction = nullptr;


    /** Monitoring Object */
    MonitoringObject* m_monObj {};

#ifdef _BELLE2_EPICS
    //! one EPICS PV
    chid  mychid;
#endif
  };
} // end namespace Belle2

