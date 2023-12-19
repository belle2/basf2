/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//DQM
#include <dqm/core/DQMHistAnalysis.h>

//ROOT
#include <TCanvas.h>
#include <TLine.h>
#include <TGraphErrors.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TLatex.h>

namespace Belle2 {

  /**
   * This module is for analysis of ECL DQM histograms.
   */

  class DQMHistAnalysisECLModule final : public DQMHistAnalysisModule { /**< derived from DQMHistAnalysisModule class. */

  public:

    /** Constructor. */
    DQMHistAnalysisECLModule();

    /** Destructor. */
    ~DQMHistAnalysisECLModule();

    /** Initialize the module. */
    void initialize() override final;
    /** Call when a run begins. */
    void beginRun() override final;
    /** Event processor. */
    void event() override final;
    /** Call when a run ends. */
    void endRun() override final;
    /** Terminate. */
    void terminate() override final;

  private:

    /** Parameters for hit map histograms. */
    std::vector<double> m_HitMapThresholds{};
    /** Options for waveform histograms. */
    std::vector<std::string> m_WaveformOption{};
    /** Maximum boundary for crate time offsets. */
    double m_CrateTimeOffsetsMax;
    /** Maximum of fails for logic test. */
    int m_LogicTestMax;
    /** Prefix to use for PVs registered by this module */
    std::string m_pvPrefix;

    /** Vector for crates IDs w/ low statistics. */
    std::vector<short> m_low{};

    /** Values of crate time offsets */
    double m_crate_time_offsets[52] = {};
    /** Minimum fraction of saved waveforms for each waveform type */
    std::map<std::string, double> m_wf_fraction;

    /** TLine to show lower boundary for 'trigtag2_trigid' histogram. */
    TLine* m_lower_boundary_trigtag2 = nullptr;
    /** TLine to show upper boundary for 'trigtag2_trigid' histogram. */
    TLine* m_upper_boundary_trigtag2 = nullptr;

    /** TLine to show lower boundary for 'crate_time_offsets' graph. */
    TLine* m_lower_boundary_time_offsets = nullptr;
    /** TLine to show upper boundary for 'crate_time_offsets' graph. */
    TLine* m_upper_boundary_time_offsets = nullptr;

    /** TCanvas for time offsets. */
    TCanvas* c_crate_time_offsets = nullptr;

    /** Histogram showing signal times from ECL crates (Thr. = 1 GeV). */
    TH1* h_time_crate_Thr1GeV = nullptr;
    /** Graph for time offsets. */
    TGraphErrors* h_crate_time_offsets = nullptr;

    /** TCanvas for ECL logic summary. */
    TCanvas* c_logic_summary = nullptr;
    /** Histogram for ECL logic summary. */
    TH2F* h_logic_summary = nullptr;

    /** TCanvas for quality .*/
    TCanvas* c_quality_analysis = nullptr;
    /** TCanvas for quality_other .*/
    TCanvas* c_quality_other_analysis = nullptr;
    /**TCanvas for bad_quality .*/
    TCanvas* c_bad_quality_analysis = nullptr;
    /** TCanvas for trigtag1 .*/
    TCanvas* c_trigtag1_analysis = nullptr;
    /** TCanvas for trigtag2 .*/
    TCanvas* c_trigtag2_analysis = nullptr;
    /** TCanvas for adc_hits .*/
    TCanvas* c_adc_hits_analysis = nullptr;
    /** TCanvas for ampfail_quality .*/
    TCanvas* c_ampfail_quality_analysis = nullptr;
    /** TCanvas for timefail_quality .*/
    TCanvas* c_timefail_quality_analysis = nullptr;
    /** TCanvas for quality_fit_data .*/
    TCanvas* c_quality_fit_data_analysis = nullptr;

    /** Vector of TCanvases for hit map .*/
    std::vector<TCanvas*> c_cid_analysis{};

    /**Vector of TCanvases for waveforms .*/
    std::vector<TCanvas*> c_wf_analysis{};

    MonitoringObject* m_monObj = nullptr; /**< MiraBelle monitoring object */

    /** Normalize histograms. */
    void normalize(TCanvas*, const std::string&, const Double_t&);
  };
} // end namespace Belle2

