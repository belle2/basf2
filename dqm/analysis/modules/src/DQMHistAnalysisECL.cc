/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * ECL Data Quality Monitor (Analysis part)                               *
 *                                                                        *
 * This module provides analysis of ECL DQM histograms                    *
 * @ 2D Histos comparitor (color alert)                                   *
 * @ Histo title renew (adc_flag)                                         *
 * @ 2D histo analysis (time_crate_%1%_Thr1GeV)                           *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitry Matvienko (d.v.matvienko@inp.nsk.su)              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//THIS MODULE
#include <dqm/analysis/modules/DQMHistAnalysisECL.h>

#include <boost/format.hpp>
#include <cmath>
#include <TROOT.h>
#include <TStyle.h>
#include <TColor.h>
#include <sstream>

using namespace Belle2;

REG_MODULE(DQMHistAnalysisECL)

DQMHistAnalysisECLModule::DQMHistAnalysisECLModule()
  : DQMHistAnalysisModule()
{
  B2DEBUG(20, "DQMHistAnalysisECL: Constructor done.");

  m_WaveformOption = {"psd", "logic", "rand", "dphy", "other"};

  addParam("HitMapThresholds", m_HitMapThresholds, "Thresholds to display hit map, MeV", std::vector<double> {0, 5, 10, 50});
  addParam("WaveformOption", m_WaveformOption, "Option (all,psd,logic,rand,dphy) to display waveform flow",
           m_WaveformOption);
  addParam("CrateTimeOffsetsMax", m_CrateTimeOffsetsMax, "Maximum boundary for crate time offsets", 20.);
  addParam("LogicTestMax", m_LogicTestMax, " Maximum of fails for logic test", 50);
}


DQMHistAnalysisECLModule::~DQMHistAnalysisECLModule() { }

void DQMHistAnalysisECLModule::initialize()
{
  gROOT->cd();
  B2DEBUG(20, "DQMHistAnalysisECL: initialized.");

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
}

void DQMHistAnalysisECLModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisECL: beginRun called.");
}

void DQMHistAnalysisECLModule::normalize(TCanvas* c, const std::string&  h_name, const Double_t& weight)
{
  c->Clear();
  c->cd();
  TH1* h = findHist(h_name);
  if (h != NULL) {
    for (unsigned short i = 0; i < h->GetNbinsX(); i++) {
      Double_t entries = h->GetBinContent(i + 1);
      Double_t error = h->GetBinError(i + 1);
      if (weight) {
        h->SetBinContent(i + 1, entries / weight);
        h->SetBinError(i + 1, error / weight);
      }
    }
    h->Draw("HIST");
  }
  c->Draw();
  c->Modified();
  c->Update();
}

void DQMHistAnalysisECLModule::event()
{
  B2DEBUG(20, "DQMHistAnalysisECL: event called");

  //gStyle requirements
  gStyle->SetPalette(1);

  //1D histos
  //quality
  c_quality_analysis->Clear();
  c_quality_analysis->cd();
  c_quality_analysis->SetLogy();
  TH1* h_quality = findHist("ECL/quality");
  if (h_quality != NULL) {
    h_quality->SetMinimum(0.1);
    h_quality->Draw();
  }
  c_quality_analysis->Draw();
  c_quality_analysis->Modified();
  c_quality_analysis->Update();

  //quality_other
  c_quality_other_analysis->Clear();
  c_quality_other_analysis->cd();
  c_quality_other_analysis->SetLogy();
  TH1* h_quality_other = findHist("ECL/quality_other");
  if (h_quality_other != NULL) {
    h_quality_other->SetMinimum(0.1);
    h_quality_other->Draw();
  }
  c_quality_other_analysis->Draw();
  c_quality_other_analysis->Modified();
  c_quality_other_analysis->Update();

  //bad_quality ,cid_Thr%1%MeV, wf_cid_%1%, wf_sh_%1%, wf_cr_%1%
  TH1* h_evtot = findHist("ECL/event");
  if (h_evtot != NULL) {
    Double_t events = h_evtot->GetBinContent(1);
    normalize(c_bad_quality_analysis, "ECL/bad_quality", events);
    for (std::size_t i = 0; i < m_HitMapThresholds.size(); ++i)
      normalize(c_cid_analysis[i],
                str(boost::format("ECL/cid_Thr%1%MeV") % m_HitMapThresholds[i]), events);
  }
  for (std::size_t i = 0; i < m_WaveformOption.size(); ++i) {
    auto val =  m_WaveformOption[i];
    if (val != "psd" && val != "other") {
      TH1* h_evtot_norm = findHist(str(boost::format("ECL/event_%1%") % val));
      if (h_evtot_norm != NULL) {
        Double_t events = h_evtot_norm->GetBinContent(1);
        normalize(c_wf_analysis[i],
                  str(boost::format("ECL/wf_cid_%1%") % val), events);
      }
    } else if (val == "psd") {
      c_wf_analysis[i]->Clear();
      c_wf_analysis[i]->cd();
      TH1* h_psd = findHist(str(boost::format("ECL/wf_cid_%1%") % val));
      TH1* h_psd_norm = findHist(str(boost::format("ECL/%1%_cid") % val));
      if (h_psd != NULL && h_psd_norm != NULL) {
        h_psd->Divide(h_psd, h_psd_norm);
        h_psd->Draw("HIST");
      }
      c_wf_analysis[i]->Draw();
      c_wf_analysis[i]->Modified();
      c_wf_analysis[i]->Update();
    }
  } //m_WaveformOption

  //trigtag1
  c_trigtag1_analysis->Clear();
  c_trigtag1_analysis->cd();
  c_trigtag1_analysis->Pad()->SetFrameFillColor(10);
  c_trigtag1_analysis->Pad()->SetFillColor(kWhite);
  c_trigtag1_analysis->SetLogy();
  TH1* h_trigtag1 = findHist("ECL/trigtag1");
  if (h_trigtag1 != NULL) {
    h_trigtag1->SetMinimum(0.1);
    h_trigtag1->Draw();
    if (h_trigtag1->GetBinContent(2)) c_trigtag1_analysis->Pad()->SetFillColor(kRed);
  }
  c_trigtag1_analysis->Draw();
  c_trigtag1_analysis->Modified();
  c_trigtag1_analysis->Update();

  //adc_hits
  c_adc_hits_analysis->Clear();
  c_adc_hits_analysis->cd();
  c_adc_hits_analysis->SetLogy();
  TH1* h_adc_hits = findHist("ECL/adc_hits");
  if (h_adc_hits != NULL) {
    h_adc_hits->SetMinimum(0.1);
    h_adc_hits->Draw();
  }
  c_adc_hits_analysis->Draw();
  c_adc_hits_analysis->Modified();
  c_adc_hits_analysis->Update();

  //ampfail_quality
  c_ampfail_quality_analysis->Clear();
  c_ampfail_quality_analysis->cd();
  c_ampfail_quality_analysis->Pad()->SetFrameFillColor(10);
  c_ampfail_quality_analysis->Pad()->SetFillColor(kWhite);
  c_ampfail_quality_analysis->SetLogy();
  TH1* h_ampfail_quality = findHist("ECL/ampfail_quality");
  if (h_ampfail_quality != NULL) {
    h_ampfail_quality->SetMinimum(0.1);
    h_ampfail_quality->Draw();
    for (unsigned short i = 1; i < 5; i++) {
      if (h_ampfail_quality->GetBinContent(i + 1)) {
        c_ampfail_quality_analysis->Pad()->SetFillColor(kRed);
        break;
      }
    }
  }
  c_ampfail_quality_analysis->Draw();
  c_ampfail_quality_analysis->Modified();
  c_ampfail_quality_analysis->Update();

  //timefail_quality
  c_timefail_quality_analysis->Clear();
  c_timefail_quality_analysis->cd();
  c_timefail_quality_analysis->Pad()->SetFrameFillColor(10);
  c_timefail_quality_analysis->Pad()->SetFillColor(kWhite);
  c_timefail_quality_analysis->SetLogy();
  TH1* h_timefail_quality = findHist("ECL/timefail_quality");
  if (h_timefail_quality != NULL) {
    h_timefail_quality->SetMinimum(0.1);
    h_timefail_quality->Draw();
    for (unsigned short i = 1; i < 5; i++) {
      if (h_timefail_quality->GetBinContent(i + 1)) {
        c_timefail_quality_analysis->Pad()->SetFillColor(kRed);
        break;
      }
    }
  }
  c_timefail_quality_analysis->Draw();
  c_timefail_quality_analysis->Modified();
  c_timefail_quality_analysis->Update();

  //2D histos
  //trigtag2_trigid
  c_trigtag2_analysis->Clear();
  c_trigtag2_analysis->cd();
  c_trigtag2_analysis->Pad()->SetFrameFillColor(10);
  c_trigtag2_analysis->Pad()->SetFillColor(kWhite);
  TH1* h_trigtag2_trigid = findHist("ECL/trigtag2_trigid");
  if (h_trigtag2_trigid != NULL) {
    h_trigtag2_trigid->Draw();
    for (unsigned short i = 0; i  < 52; i++) {
      if (h_trigtag2_trigid->GetBinContent(h_trigtag2_trigid->GetBin(i + 1, 3))) {
        c_trigtag2_analysis->Pad()->SetFillColor(kRed);
        break;
      }
    }
  }
  m_lower_boundary_trigtag2->Draw();
  m_upper_boundary_trigtag2->Draw();

  c_trigtag2_analysis->Draw();
  c_trigtag2_analysis->Modified();
  c_trigtag2_analysis->Update();

  //quality_fit_data
  c_quality_fit_data_analysis->Clear();
  c_quality_fit_data_analysis->cd();
  c_quality_fit_data_analysis->Pad()->SetFrameFillColor(10);
  c_quality_fit_data_analysis->Pad()->SetFillColor(kWhite);
  TH1* h_quality_fit_data = findHist("ECL/quality_fit_data");
  if (h_quality_fit_data != NULL) {
    h_quality_fit_data->Draw();
    for (unsigned short i = 0; i < 4; i++) {
      for (unsigned short j = 0; j < 4; j++) {
        if (h_quality_fit_data->GetBinContent(h_quality_fit_data->GetBin(i + 1, j + 1)) > 0) {
          c_quality_fit_data_analysis->Pad()->SetFillColor(kRed);
          break;
        }
      }
    }
  }
  c_quality_fit_data_analysis->Draw();
  c_quality_fit_data_analysis->Modified();
  c_quality_fit_data_analysis->Update();

  //_time_crate_%1%_Thr1GeV
  bool colRed = false;

  m_low.clear();
  h_crate_time_offsets->Set(0);

  for (unsigned short i = 0; i < 52; i++) {
    std::string h_title = "ECL/time_crate_" + std::to_string(i + 1) + "_Thr1GeV";
    h_time_crate_Thr1GeV = findHist(h_title);
    if (h_time_crate_Thr1GeV != NULL) {
      h_crate_time_offsets->SetPoint(i, i + 0.5, h_time_crate_Thr1GeV->GetMean());
      h_crate_time_offsets->SetPointError(i, 0.1, h_time_crate_Thr1GeV->GetMeanError());
      if (h_time_crate_Thr1GeV->Integral() > 100) {
        double yval = (h_time_crate_Thr1GeV->GetMean() > 0) ?
                      h_time_crate_Thr1GeV->GetMean() - 2 * h_time_crate_Thr1GeV->GetMeanError() :
                      h_time_crate_Thr1GeV->GetMean() + 2 * h_time_crate_Thr1GeV->GetMeanError();
        if (abs(yval) > m_CrateTimeOffsetsMax) colRed = true;
      } else m_low.push_back(i + 1);
    }
  }

  c_crate_time_offsets->Clear();
  c_crate_time_offsets->SetGrid();
  c_crate_time_offsets->cd();

  c_crate_time_offsets->Pad()->SetFrameFillColor(10);
  c_crate_time_offsets->Pad()->SetFillColor(kWhite);

  h_crate_time_offsets->SetMinimum(-50);
  h_crate_time_offsets->SetMaximum(50);
  h_crate_time_offsets->GetXaxis()->Set(52, 0, 52);
  for (unsigned short i = 0; i < 52; i++) h_crate_time_offsets->GetXaxis()->SetBinLabel(i + 1, std::to_string(i + 1).c_str());
  h_crate_time_offsets->GetXaxis()->LabelsOption("v");

  h_crate_time_offsets->Draw("AP");

  if (!m_low.empty()) {
    auto tg = new TLatex(5, 40, "Low statistics");
    tg->SetTextSize(.06);
    tg->SetTextAlign(12);
    tg->Draw();
    c_crate_time_offsets->Pad()->SetFillColor(kYellow);
    if (m_low.size() < 52) {
      std::ostringstream sstream;
      std::copy(m_low.begin(), m_low.end() - 1, std::ostream_iterator<short>(sstream, ","));
      sstream << m_low.back();
      std::string str = "Crates: " + sstream.str();
      auto tg1 = new TLatex(5, 30, str.c_str());
      tg1->SetTextSize(.03);
      tg1->SetTextAlign(12);
      tg1->Draw();
    } else {
      auto tg1 = new TLatex(5, 30, "All crates");
      tg1->SetTextSize(.06);
      tg1->SetTextAlign(12);
      tg1->Draw();
    }
  }

  m_lower_boundary_time_offsets->Draw();
  m_upper_boundary_time_offsets->Draw();

  if (colRed) c_crate_time_offsets->Pad()->SetFillColor(kRed);

  c_crate_time_offsets->Draw();
  c_crate_time_offsets->Modified();
  c_crate_time_offsets->Update();

  //DQM summary logic
  h_logic_summary->Reset();

  TH1* h_fail_crateid = findHist("ECL/fail_crateid");
  TH1* h_fail_shaperid = findHist("ECL/fail_shaperid");
  if (h_fail_crateid != NULL && h_fail_shaperid != NULL) {
    for (unsigned short i = 0; i < 52; i++) {
      if (h_fail_crateid->GetBinContent(i + 1)) {
        for (unsigned short j = i * 12; j < i * 12 + 12; j++) {
          if (h_fail_shaperid->GetBinContent(j + 1)) {
            unsigned short shaper = (j + 1) - 12 * i;
            h_logic_summary->SetBinContent(h_logic_summary->FindBin(i + 1, shaper), h_fail_shaperid->GetBinContent(j + 1));
          }
        }
      }
    }
    c_logic_summary->Clear();
    c_logic_summary->SetGrid();
    c_logic_summary->cd();

    c_logic_summary->Pad()->SetFrameFillColor(10);
    c_logic_summary->Pad()->SetFillColor(kWhite);

    if (h_logic_summary->GetMaximum() > 0
        && h_logic_summary->GetMaximum() < m_LogicTestMax) c_logic_summary->Pad()->SetFillColor(kYellow);
    if (h_logic_summary->GetMaximum() >= m_LogicTestMax) c_logic_summary->Pad()->SetFillColor(kRed);
    h_logic_summary->Draw("textcol");
    c_logic_summary->Draw();
    c_logic_summary->Modified();
    c_logic_summary->Update();
  }
}

void DQMHistAnalysisECLModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisECL: endRun called");
}


void DQMHistAnalysisECLModule::terminate()
{
  B2DEBUG(20, "terminate called");
  delete c_quality_analysis;
  delete c_quality_other_analysis;
  delete c_bad_quality_analysis;
  delete c_trigtag1_analysis;
  delete c_trigtag2_analysis;
  delete c_adc_hits_analysis;
  delete c_ampfail_quality_analysis;
  delete c_timefail_quality_analysis;
  delete c_quality_fit_data_analysis;
  delete m_lower_boundary_trigtag2;
  delete m_upper_boundary_trigtag2;
  delete m_lower_boundary_time_offsets;
  delete m_upper_boundary_time_offsets;
  delete c_crate_time_offsets;
  delete h_time_crate_Thr1GeV;
  delete h_crate_time_offsets;
  delete c_logic_summary;
  delete h_logic_summary;
}
