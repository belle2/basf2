/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPXDEff.h
// Description : DQM module, which gives histograms showing the efficiency of PXD sensors
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <vxd/dataobjects/VxdID.h>

#include <TH2F.h>
#include <TEfficiency.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Efficiency */

  class DQMHistAnalysisPXDEffModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisPXDEffModule();

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

    /**
     * This method is called at the end of the event processing.
     */
    void terminate(void) override final;

  private:
    /**
     * Update bin in efficiency plots with condition on nhits
     * @param bin Bin number
     * @param nhit Number of hits
     * @param nmatch Number of matched hits
     * @param minentries Number of minimal entries required ofr update
     * @return if (delta) plot was updated
     */
    bool updateEffBins(int bin, int nhit, int nmatch, int minentries);

    /**
     * Check bin/name for error condition
     * @param bin Bin number
     * @param name Name of bin (Module, layer, ..)
     * @return If error condition was met
     */
    bool check_error_level(int bin, std::string name);

    /**
     * Check bin/name for warn condition
     * @param bin Bin number
     * @param name Name of bin (Module, layer, ..)
     * @return If warn condition was met
     */
    bool check_warn_level(int bin, std::string name);

    /**
     * Set module labels for TGraphAsymmErrors
     * @param gr the TGraphAsymmErrors to update
     */
    void setLabels(TGraphAsymmErrors* gr);

  private:

    // Data members
    //! name of histogram directory
    std::string m_histogramDirectoryName;
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
    /** Update entry interval */
    int m_minEntries = 1000;
    /** use alarm level per module */
    bool m_perModuleAlarm;
    /** generate alarm from adhoc values */
    bool m_alarmAdhoc;
    /** Indizes of excluded PXD Modules */
    std::vector<int> m_excluded;

    //! IDs of all PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    //! Individual efficiency for each module, 2d histogram
    std::map<VxdID, TEfficiency*> m_eEffModules;
    //! Individual efficiency for each module, canvas
    std::map<VxdID, TCanvas*> m_cEffModules;

    //! One bin for each module in the geometry
    TEfficiency* m_eEffAll = nullptr;
    //! Final Canvas
    TCanvas* m_cEffAll = nullptr;
    //! TH1, last state, total
    TH1* m_hEffAllLastTotal = nullptr;
    //! TH1, last state, passed
    TH1* m_hEffAllLastPassed = nullptr;
    //! Efficiency, last state, updated
    TEfficiency* m_eEffAllUpdate = nullptr;
    //! Final Canvas for Update
    TCanvas* m_cEffAllUpdate = nullptr;

    //! Full Eff Map Inner Layer
    TH2F* m_hInnerMap{};
    //! Full Eff Map Outer Layer
    TH2F* m_hOuterMap{};
    //! Full Eff Map Inner Layer
    TCanvas* m_cInnerMap{};
    //! Full Eff Map Outer Layer
    TCanvas* m_cOuterMap{};

    /** TLine object for warning limit */
    TH1F* m_hWarnLine{};
    /** TLine object for error error */
    TH1F* m_hErrorLine{};
    //! warn level for alarm per module
    std::map<std::string, double> m_warnlevelmod;
    //! error level for alarm per module
    std::map<std::string, double> m_errorlevelmod;
    //! Number of bins in efficiency plot, all modules plus layer and summary
    int m_nrxbins;

    /** Monitoring Object */
    MonitoringObject* m_monObj {};
  };
} // end namespace Belle2

