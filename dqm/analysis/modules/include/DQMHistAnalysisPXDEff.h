//+
// File : DQMHistAnalysisPXDEff.h
// Description : DQM module, which gives histograms showing the efficiency of PXD sensors
//
// Author: Uwe Gebauer, Bjoern Spruck
//-

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <vxd/dataobjects/VxdID.h>

#include <TEfficiency.h>
#include <TCanvas.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Efficiency */

  class DQMHistAnalysisPXDEffModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor
    DQMHistAnalysisPXDEffModule();
    //! Destructor
    ~DQMHistAnalysisPXDEffModule();
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
    //! confidence level for error bars
    double m_confidence;
    //! warn level for alarm
    double m_warnlevel;
    //! error level for alarm
    double m_errorlevel;
    /** Update entry intervall */
    int m_minEntries = 1000;
    /** use alarm level per module */
    bool m_perModuleAlarm;
    /** generate alarm from adhoc values */
    bool m_alarmAdhoc;

    //! IDs of all PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    //! Individual efficiency for each module, 2d histogram
    std::map<VxdID, TEfficiency*> m_hEffModules;
    //! Individual efficiency for each module, canvas
    std::map<VxdID, TCanvas*> m_cEffModules;

    //! One bin for each module in the geometry
    TEfficiency* m_hEffAll = nullptr;
    //! Final Canvas
    TCanvas* m_cEffAll = nullptr;
    //! TH1, last state, total
    TH1* m_hEffAllLastTotal = nullptr;
    //! TH1, last state, passed
    TH1* m_hEffAllLastPassed = nullptr;
    //! Efficiency, last state, updated
    TEfficiency* m_hEffAllUpdate = nullptr;
    //! Final Canvas for Update
    TCanvas* m_cEffAllUpdate = nullptr;


    /** TLine object for warning limit */
    TH1F* m_hWarnLine{};
    /** TLine object for error error */
    TH1F* m_hErrorLine{};
    //! warn level for alarm per module
    std::map<VxdID, double> m_warnlevelmod;
    //! error level for alarm per module
    std::map<VxdID, double> m_errorlevelmod;

    /** Monitoring Object */
    MonitoringObject* m_monObj {};

    /** flag if to export to EPICS */
    bool m_useEpics;

    /** flag if to import from EPICS only */
    bool m_useEpicsRO;

#ifdef _BELLE2_EPICS
    //! one EPICS PV
    std::vector <chid>  mychid_status;
    std::map <VxdID, chid> mychid_eff;
#endif
  };
} // end namespace Belle2

