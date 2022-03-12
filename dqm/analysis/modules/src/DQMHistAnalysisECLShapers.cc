/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//THIS MODULE
#include <dqm/analysis/modules/DQMHistAnalysisECLShapers.h>

#include <boost/format.hpp>
#include <cmath>
#include <TROOT.h>
#include <TStyle.h>
#include <TColor.h>
#include <sstream>

using namespace Belle2;

REG_MODULE(DQMHistAnalysisECLShapers)

DQMHistAnalysisECLShapersModule::DQMHistAnalysisECLShapersModule()
  : DQMHistAnalysisModule()
{
  B2DEBUG(20, "DQMHistAnalysisECLShapers: Constructor done.");

  m_WaveformOption = {"psd", "logic", "rand", "dphy", "other"};

  addParam("HitMapThresholds", m_HitMapThresholds, "Thresholds to display hit map, MeV", std::vector<double> {0, 5, 10, 50});
  addParam("WaveformOption", m_WaveformOption, "Option (all,psd,logic,rand,dphy) to display waveform flow",
           m_WaveformOption);
  addParam("CrateTimeOffsetsMax", m_CrateTimeOffsetsMax, "Maximum boundary for crate time offsets", 20.);
  addParam("LogicTestMax", m_LogicTestMax, " Maximum of fails for logic test", 50);
  addParam("useEpics", m_useEpics, "Whether to update EPICS PVs.", false);
}


DQMHistAnalysisECLShapersModule::~DQMHistAnalysisECLShapersModule()
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (ca_current_context()) ca_context_destroy();
  }
#endif
}

void DQMHistAnalysisECLShapersModule::initialize()
{
  gROOT->cd();
  B2DEBUG(20, "DQMHistAnalysisECLShapers: initialized.");

  //new canvases for existing histograms
  c_quality_analysis = new TCanvas("ECL/c_quality_analysis");
  c_quality_other_analysis = new TCanvas("ECL/c_quality_other_analysis");
  c_bad_quality_analysis = new TCanvas("ECL/c_bad_quality_analysis");
  c_trigtag1_analysis = new TCanvas("ECL/c_trigtag1_analysis");
  c_trigtag2_analysis = new TCanvas("ECL/c_trigtag2_analysis");
  c_adc_hits_analysis = new TCanvas("ECL/c_adc_hits_analysis");
  c_ampfail_quality_analysis = new TCanvas("ECL/c_ampfail_quality_analysis");
  c_timefail_quality_analysis = new TCanvas("ECL/c_timefail_quality_analysis");
  c_quality_fit_data_analysis = new TCanvas("ECL/c_quality_fit_data_analysis");

  for (const auto& id : m_HitMapThresholds) {
    std::string canvas_name = str(boost::format("ECL/c_cid_Thr%1%MeV_analysis") % id);
    TCanvas* canvas  = new TCanvas(canvas_name.c_str());
    c_cid_analysis.push_back(canvas);
  }

  for (const auto& id : m_WaveformOption) {
    if (id != "other") {
      std::string canvas_name = str(boost::format("ECL/c_wf_cid_%1%_analysis") % id);
      TCanvas* canvas  = new TCanvas(canvas_name.c_str());
      c_wf_analysis.push_back(canvas);
    }
  }

  //Boundaries for 'trigtag2_trigid' histogram
  m_lower_boundary_trigtag2 = new TLine(1, 0, 53, 0);
  m_lower_boundary_trigtag2->SetLineWidth(3);
  m_lower_boundary_trigtag2->SetLineColor(kBlue);

  m_upper_boundary_trigtag2 = new TLine(1, 1, 53, 1);
  m_upper_boundary_trigtag2->SetLineWidth(3);
  m_upper_boundary_trigtag2->SetLineColor(kBlue);
  //Boundaries for 'crate_time_offset' plot
  m_lower_boundary_time_offsets = new TLine(0, -20, 52, -20);
  m_lower_boundary_time_offsets->SetLineWidth(3);
  m_lower_boundary_time_offsets->SetLineColor(kBlue);

  m_upper_boundary_time_offsets = new TLine(0, 20, 52, 20);
  m_upper_boundary_time_offsets->SetLineWidth(3);
  m_upper_boundary_time_offsets->SetLineColor(kBlue);

  //Summary crate_time_offsets plot
  c_crate_time_offsets = new TCanvas("ECL/c_crate_time_offsets");
  h_crate_time_offsets = new TGraphErrors();
  h_crate_time_offsets->SetName("t_off");
  h_crate_time_offsets->SetTitle("Crate time offset (E > 1 GeV); Crate ID (same as ECLCollector ID); Time offset [ns]");
  h_crate_time_offsets->SetMarkerColor(kBlue);
  h_crate_time_offsets->SetMarkerStyle(20);

  //New DQM summary for logic test in CR shifter panel
  c_logic_summary = new TCanvas("ECL/c_logic_summary");
  h_logic_summary = new TH2F("logic_summary", "FPGA <-> C++ fitter inconsistencies count", 52, 1, 53, 12, 1, 13);
  h_logic_summary->SetTitle("FPGA <-> C++ fitter inconsistencies count; ECLCollector ID (same as Crate ID); Shaper ID inside the crate");
  h_logic_summary->SetCanExtend(TH1::kAllAxes);
  h_logic_summary->SetStats(0);
  for (unsigned short i = 0; i < 52; i++) h_logic_summary->GetXaxis()->SetBinLabel(i + 1, std::to_string(i + 1).c_str());
  for (unsigned short i = 0; i < 12; i++) h_logic_summary->GetYaxis()->SetBinLabel(i + 1, std::to_string(i + 1).c_str());
  h_logic_summary->LabelsOption("v");
  h_logic_summary->SetTickLength(0, "xy");

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    for (int i = 0; i < 52; i++) {
      std::string pv_name = "ECL:logic_check:crate" + std::to_string(i + 1);
      SEVCHK(ca_create_channel(pv_name.c_str(), NULL, NULL, 10, &mychid[i]), "ca_create_channel failure");
      // Read LO and HI limits from EPICS, seems this needs additional channels?
      // SEVCHK(ca_get(DBR_DOUBLE,mychid[i],(void*)&data),"ca_get failure"); // data is only valid after ca_pend_io!!
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

void DQMHistAnalysisECLShapersModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLShapers: beginRun called.");
}

void DQMHistAnalysisECLShapersModule::event()
{
  TH1* h_fail_crateid = findHist("ECL/fail_crateid");

#ifdef _BELLE2_EPICS
  if (m_useEpics && h_fail_crateid != NULL) {
    for (int i = 0; i < 52; i++) {
      int errors_count = h_fail_crateid->GetBinContent(i + 1);
      if (mychid[i]) SEVCHK(ca_put(DBR_LONG, mychid[i], (void*)&errors_count), "ca_set failure");
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

void DQMHistAnalysisECLShapersModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLShapers: endRun called");
}


void DQMHistAnalysisECLShapersModule::terminate()
{
  B2DEBUG(20, "terminate called");

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    for (int i = 0; i < 52; i++) {
      if (mychid[i]) SEVCHK(ca_clear_channel(mychid[i]), "ca_clear_channel failure");
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}
