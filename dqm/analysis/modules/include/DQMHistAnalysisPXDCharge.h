//+
// File : DQMHistAnalysisPXDCharge.h
// Description : DQM Analysis for PXD Cluster Charges
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

#include <vector>
#include <TF1.h>
#include <TCanvas.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Cluster Charge */

  class DQMHistAnalysisPXDChargeModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor
    DQMHistAnalysisPXDChargeModule();
    //! Destructor
    ~DQMHistAnalysisPXDChargeModule();
  private:

    //! Module functions to be called from main process
    void initialize(void) override final;

    //! Module functions to be called from event process
    void beginRun(void) override final;
    void event(void) override final;
    void endRun(void) override final;
    void terminate(void) override final;

    // Data members
    //! name of histogram directory
    std::string m_histogramDirectoryName;
    //! prefix for EPICS PVs
    std::string m_pvPrefix;
    //! fit range lo edge for landau
    double m_rangeLow;
    //! fit range hi edge for landau
    double m_rangeHigh;

    //! IDs of all PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    //! only one fit function for all Landaus
    TF1* m_fLandau = nullptr;
    //! Fit the Mean for all modules
    TF1* m_fMean = nullptr;
    //! Histogram covering all modules
    TH1F* m_hCharge = nullptr;
    //! Final Canvas
    TCanvas* m_cCharge = nullptr;

    /** Monitoring Object */
    MonitoringObject* m_monObj {};

    /** flag if to export to EPICS */
    bool m_useEpics;

#ifdef _BELLE2_EPICS
    //! Place for EPICS PVs, Mean and maximum deviation
    std::vector <chid> mychid;
#endif
  };
} // end namespace Belle2

