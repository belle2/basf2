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

//FRAMEWORK
#include <framework/core/Module.h>

//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>

//ROOT
#include <TCanvas.h>
#include <TString.h>
#include <TLine.h>
#include <TPaveText.h>
#include <TFile.h>
#include <TLegend.h>
#include <THStack.h>
#include <TWbox.h>

class TH1F;
class TH2F;


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

    /** TLines showing acceptable boundaries for 'trigtag2_trigid' histogram. */
    TLine* m_line1 = nullptr;
    TLine* m_line2 = nullptr;

    /** Statistics control flag for time crate histograms from current run. */
    Int_t stat[52] = {};
    /** Statistics control flag for time crate histograms from reference run. */
    Int_t stat_ref[52] = {};

    /** Combined error for time offsets from current and reference runs. */
    Double_t combined_err[52] = {};
    /** Distance in terms of 'sigmas' between time offsets from current and reference runs. */
    Double_t nsigmas[52] = {};
    /** Sigma level which determines 'large time offsets'. */
    Double_t m_level = 5.;

    /** TCanvas for time offsets. */
    TCanvas* c_crate_time_offsets = nullptr;

    /** THStack for time offsets. */
    THStack* hs = nullptr;
    /** TLegend for time offsets. */
    TLegend* m_leg = nullptr;

    /**TWboxes for time offsets TLegend. */
    TWbox* box1 = nullptr;
    TWbox* box2 = nullptr;

    /** Histogram showing signal times from ECL crates (Thr. = 1 GeV). */
    TH1* h_time_crate_Thr1GeV = nullptr;
    /** Histogram for time offsets (current run). */
    TH1F* h_crate_time_offsets = nullptr;
    /** Histogram for time offsets (reference run). */
    TH1F* h_crate_time_offsets_ref = nullptr;

    /** Find TCanvas by name. */
    TCanvas* findCanv(TString);
    /** Fill time offsets histogram and statistics control flag. */
    void timeCrate(TH1F*, Int_t*);

  };
} // end namespace Belle2

