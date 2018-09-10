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

    //! Constructor / Destructor
    DQMHistAnalysisPXDEffModule();
  private:

    //! Module functions to be called from main process
    void initialize(void) override final;

    //! Module functions to be called from event process
    void beginRun(void) override final;
    void event(void) override final;
    void terminate(void) override final;

    // Data members
    int m_u_bins;
    int m_v_bins;
    std::string m_histogramDirectoryName;
    std::string m_pvPrefix;
    bool m_singleHists;

    //IDs of all PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    //Individual efficiency for each module
    std::map<VxdID, TH2D*> m_hEffModules;
    std::map<VxdID, TCanvas*> m_cEffModules;

    //Make four summary plots for each module type
    std::map<std::string, TH2D*> m_hEffMerge;
    std::map<std::string, TCanvas*> m_cEffMerge;

    //One bin for each module in the geometry, one histogram for each layer
    TH1D* m_hEffAll;
    TCanvas* m_cEffAll;

    TH1* GetHisto(TString a);

#ifdef _BELLE2_EPICS
    chid  mychid;
#endif
  };
} // end namespace Belle2

