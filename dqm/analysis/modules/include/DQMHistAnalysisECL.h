/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * ECL Data Quality Monitor (Analysis part)                               *
 *                                                                        *
 * This module provides analysis of ECL DQM histograms                    *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitry Matvienko (d.v.matvienko@inp.nsk.su)              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>

//ROOT
#include <TCanvas.h>
#include <TLine.h>
#include <TGraphErrors.h>
#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {

  /**
   * This module is for analysis of ECL DQM histograms.
   */

  class DQMHistAnalysisECLModule : public DQMHistAnalysisModule { /**< derived from DQMHistAnalysisModule class. */

  public:

    /** Constructor. */
    DQMHistAnalysisECLModule();

    /** Destructor. */
    virtual ~DQMHistAnalysisECLModule();

    /** Initialize the module. */
    virtual void initialize() override;
    /** Call when a run begins. */
    virtual void beginRun() override;
    /** Event processor. */
    virtual void event() override;
    /** Call when a run ends. */
    virtual void endRun() override;
    /** Terminate. */
    virtual void terminate() override;

  private:

    /** Parameters for hit map histograms. */
    std::vector<double> m_HitMapThresholds{};
    /** Options for waveform histograms. */
    std::vector<std::string> m_WaveformOption{};

    /** TLine to show lower boundary for 'trigtag2_trigid' histogram. */
    TLine* m_lower_boundary_trigtag2 = nullptr;
    /** TLine to show upper boundary for 'trigtag2_trigid' histogram. */
    TLine* m_upper_boundary_trigtag2 = nullptr;

    //TLine to show lower boundary for 'crate_time_offsets' graph. */
    TLine* m_lower_boundary_time_offsets = nullptr;
    //TLine to show upper boundary for 'crate_time_offsets' graph. */
    TLine* m_upper_boundary_time_offsets = nullptr;

    /** TCanvas for time offsets. */
    TCanvas* c_crate_time_offsets = nullptr;

    /** Histogram showing signal times from ECL crates (Thr. = 1 GeV). */
    TH1* h_time_crate_Thr1GeV = nullptr;
    /** Graph for time offsets. */
    TGraphErrors* h_crate_time_offsets = nullptr;

    /** TCanvas for ECL logic summary. */
    TCanvas* c_logic_summary;
    /** Histogram for ECL logic summary. */
    TH2F* h_logic_summary;

    /** Find TCanvas by name. */
    TCanvas* findCanv(TString);
    /** Normalize hitsograms. */
    void normalize(std::string, std::string, Double_t);
  };
} // end namespace Belle2

