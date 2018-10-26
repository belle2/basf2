//+
// File : DQMHistAnalysisPXDEff.h
// Description : DQM module, which gives histograms showing the efficiency of PXD sensors
//
// Modified to efficiency by Uwe Gebauer
// Based on work from: Tomoyuki Konno, Tokyo Metroplitan Univerisity
// Date : 25  - Dec - 2015 ; first commit
//-

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <framework/core/Module.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Efficiency */

  class DQMHistAnalysisPXDEffModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor
    DQMHistAnalysisPXDEffModule();
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
    //! Flag to trigger creation of additional histograms
    bool m_singleHists;
    //! u binning for 2d plots
    int m_u_bins;
    //! v binning for 2d plots
    int m_v_bins;

    //! IDs of all PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    //! Individual efficiency for each module, 2d histogram
    std::map<VxdID, TH2D*> m_hEffModules;
    //! Individual efficiency for each module, canvas
    std::map<VxdID, TCanvas*> m_cEffModules;

    //! One bin for each module in the geometry
    TH1D* m_hEffAll = nullptr;
    //! Final Canvas
    TCanvas* m_cEffAll = nullptr;

#ifdef _BELLE2_EPICS
    //! one EPICS PV
    chid  mychid;
#endif
  };
} // end namespace Belle2

