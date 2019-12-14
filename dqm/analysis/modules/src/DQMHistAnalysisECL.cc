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
#include<iostream>

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
}


DQMHistAnalysisECLModule::~DQMHistAnalysisECLModule() { }

void DQMHistAnalysisECLModule::initialize()
{
  gROOT->cd();
  B2DEBUG(20, "DQMHistAnalysisECL: initialized.");

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
  h_crate_time_offsets->SetTitle("Crate time offsets (Thr = 1 GeV); Crate ID; Time offsets [nsec]");
  h_crate_time_offsets->SetMarkerColor(kBlue);
  h_crate_time_offsets->SetMarkerStyle(20);

  //New DQM summary for logic test in CR shifter panel
  c_logic_summary = new TCanvas("ECL/c_logic_summary");
  h_logic_summary = new TH2F("logic_summary", "ECL logic summary", 52, 1, 53, 12, 1, 13);
  h_logic_summary->SetTitle("ECL logic summary; Collector ID; Shaper ID inside the crate");
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


TCanvas* DQMHistAnalysisECLModule::findCanv(TString canvas_name)
{
  TIter nextkey(gROOT->GetListOfCanvases());
  TObject* obj = NULL;

  while ((obj = (TObject*)nextkey())) {
    if (obj->IsA()->InheritsFrom("TCanvas")) {
      if (obj->GetName() == canvas_name) return (TCanvas*)obj;
    }
  }
  return NULL;
}

void DQMHistAnalysisECLModule::normalize(std::string c_name, std::string h_name, Double_t weight)
{
  TCanvas* c = findCanv(c_name);
  c->cd();
  TH1* h = findHist(h_name);
  for (unsigned short i = 0; i < h->GetNbinsX(); i++) {
    Double_t entries = h->GetBinContent(i + 1);
    h->SetBinContent(i + 1, entries / weight);
  }
  h->Draw();
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
  TCanvas* c_quality = findCanv("ECL/c_quality");
  c_quality->cd();
  c_quality->SetLogy();
  TH1* h_quality = findHist("ECL/quality");
  if (h_quality != NULL) h_quality->SetMinimum(0.1);
  c_quality->Draw();
  c_quality->Modified();
  c_quality->Update();

  //cid_Thr%1%MeV, wf_cid_%1%, wf_sh_%1%, wf_cr_%1%
  TH1* h_evtot = findHist("ECL/event");
  if (h_evtot != NULL) {
    Double_t events = h_evtot->GetBinContent(1);
    for (const auto& id : m_HitMapThresholds)
      normalize(str(boost::format("ECL/c_cid_Thr%1%MeV") % id),
                str(boost::format("ECL/cid_Thr%1%MeV") % id), events);
    for (const auto& id : m_WaveformOption) {
      normalize(str(boost::format("ECL/c_wf_cid_%1%") % id),
                str(boost::format("ECL/wf_cid_%1%") % id), events);
      normalize(str(boost::format("ECL/c_wf_sh_%1%") % id),
                str(boost::format("ECL/wf_sh_%1%") % id), events);
      normalize(str(boost::format("ECL/c_wf_cr_%1%") % id),
                str(boost::format("ECL/wf_cr_%1%") % id), events);
    }
  }

  //trigtag1
  TCanvas* c_trigtag1 = findCanv("ECL/c_trigtag1");
  c_trigtag1->cd();
  c_trigtag1->Pad()->SetFrameFillColor(10);
  c_trigtag1->Pad()->SetFillColor(kWhite);
  c_trigtag1->SetLogy();
  TH1* h_trigtag1 = findHist("ECL/trigtag1");
  if (h_trigtag1 != NULL) {
    h_trigtag1->SetMinimum(0.1);
    if (h_trigtag1->GetBinContent(2)) c_trigtag1->Pad()->SetFillColor(kRed);
  }
  c_trigtag1->Draw();
  c_trigtag1->Modified();
  c_trigtag1->Update();

  //adc_hits
  TCanvas* c_adc_hits = findCanv("ECL/c_adc_hits");
  c_adc_hits->cd();
  c_adc_hits->Pad()->SetFrameFillColor(10);
  c_adc_hits->Pad()->SetFillColor(kWhite);
  c_adc_hits->SetLogy();
  TH1* h_adc_hits = findHist("ECL/adc_hits");
  if (h_adc_hits != NULL) {
    h_adc_hits->SetMinimum(0.1);
    for (unsigned short i = 50; i < 250; i++) {
      if (h_adc_hits->GetBinContent(i + 1)) {
        c_adc_hits->Pad()->SetFillColor(kRed);
        break;
      }
    }
  }
  c_adc_hits->Draw();
  c_adc_hits->Modified();
  c_adc_hits->Update();


  //ampfail_quality
  TCanvas* c_ampfail_quality = findCanv("ECL/c_ampfail_quality");
  c_ampfail_quality->cd();
  c_ampfail_quality->Pad()->SetFrameFillColor(10);
  c_ampfail_quality->Pad()->SetFillColor(kWhite);
  c_ampfail_quality->SetLogy();
  TH1* h_ampfail_quality = findHist("ECL/ampfail_quality");
  if (h_ampfail_quality != NULL) {
    h_ampfail_quality->SetMinimum(0.1);
    for (unsigned short i = 1; i < 5; i++) {
      if (h_ampfail_quality->GetBinContent(i + 1)) {
        c_ampfail_quality->Pad()->SetFillColor(kRed);
        break;
      }
    }
  }
  c_ampfail_quality->Draw();
  c_ampfail_quality->Modified();
  c_ampfail_quality->Update();

  //timefail_quality
  TCanvas* c_timefail_quality = findCanv("ECL/c_timefail_quality");
  c_timefail_quality->cd();
  c_timefail_quality->Pad()->SetFrameFillColor(10);
  c_timefail_quality->Pad()->SetFillColor(kWhite);
  c_timefail_quality->SetLogy();
  TH1* h_timefail_quality = findHist("ECL/timefail_quality");
  if (h_timefail_quality != NULL) {
    h_timefail_quality->SetMinimum(0.1);
    for (unsigned short i = 1; i < 5; i++) {
      if (h_timefail_quality->GetBinContent(i + 1)) {
        c_timefail_quality->Pad()->SetFillColor(kRed);
        break;
      }
    }
  }
  c_timefail_quality->Draw();
  c_timefail_quality->Modified();
  c_timefail_quality->Update();

  //2D histos
  //trigtag2_trigid
  TCanvas* c_trigtag2_trigid = findCanv("ECL/c_trigtag2_trigid");
  c_trigtag2_trigid->cd();
  c_trigtag2_trigid->Pad()->SetFrameFillColor(10);
  c_trigtag2_trigid->Pad()->SetFillColor(kWhite);
  TH1* h_trigtag2_trigid = findHist("ECL/trigtag2_trigid");
  if (h_trigtag2_trigid != NULL) {
    for (unsigned short i = 0; i  < 52; i++) {
      if (h_trigtag2_trigid->GetBinContent(h_trigtag2_trigid->GetBin(i + 1, 3))) {
        c_trigtag2_trigid->Pad()->SetFillColor(kRed);
        break;
      }
    }
  }
  m_lower_boundary_trigtag2->Draw();
  m_upper_boundary_trigtag2->Draw();

  c_trigtag2_trigid->Draw();
  c_trigtag2_trigid->Modified();
  c_trigtag2_trigid->Update();

  //quality_fit_data
  TCanvas* c_quality_fit_data = findCanv("ECL/c_quality_fit_data");
  c_quality_fit_data->cd();
  c_quality_fit_data->Pad()->SetFrameFillColor(10);
  c_quality_fit_data->Pad()->SetFillColor(kWhite);
  TH1* h_quality_fit_data = findHist("ECL/quality_fit_data");
  if (h_quality_fit_data != NULL) {
    for (unsigned short i = 0; i < 4; i++) {
      for (unsigned short j = 0; j < 4; j++) {
        if (h_quality_fit_data->GetBinContent(h_quality_fit_data->GetBin(i + 1, j + 1)) > 0) {
          c_quality_fit_data->Pad()->SetFillColor(kRed);
          break;
        }
      }
    }
  }
  c_quality_fit_data->Draw();
  c_quality_fit_data->Modified();
  c_quality_fit_data->Update();

  //_time_crate_%1%_Thr1GeV
  h_crate_time_offsets->Set(0);

  for (unsigned short i = 0; i < 52; i++) {
    std::string h_title = "ECL/time_crate_" + std::to_string(i + 1) + "_Thr1GeV";
    h_time_crate_Thr1GeV = findHist(h_title);
    if (h_time_crate_Thr1GeV != NULL) {
      h_crate_time_offsets->SetPoint(i, i + 0.5, h_time_crate_Thr1GeV->GetMean());
      h_crate_time_offsets->SetPointError(i, 0.1, h_time_crate_Thr1GeV->GetMeanError());
    }
  }

  c_crate_time_offsets->Clear();
  c_crate_time_offsets->cd();

  c_crate_time_offsets->Pad()->SetFrameFillColor(10);
  c_crate_time_offsets->Pad()->SetFillColor(kWhite);

  h_crate_time_offsets->SetMinimum(-50);
  h_crate_time_offsets->SetMaximum(50);
  h_crate_time_offsets->GetXaxis()->Set(52, 0, 52);
  for (unsigned short i = 0; i < 52; i++) h_crate_time_offsets->GetXaxis()->SetBinLabel(i + 1, std::to_string(i + 1).c_str());
  h_crate_time_offsets->GetXaxis()->LabelsOption("v");

  h_crate_time_offsets->Draw("AP");

  m_lower_boundary_time_offsets->Draw();
  m_upper_boundary_time_offsets->Draw();

  Double_t* yarray = h_crate_time_offsets->GetY();
  std::vector<double> yvec(yarray, yarray + 51);

  for (auto& yval : yvec) {
    if (abs(yval) > 20) {
      c_crate_time_offsets->Pad()->SetFillColor(kRed);
      break;
    }
  }

  c_crate_time_offsets->Draw();
  c_crate_time_offsets->Modified();
  c_crate_time_offsets->Update();

  //DQM summary logic
  TH1* h_fail_crateid = findHist("ECL/fail_crateid");
  TH1* h_fail_shaperid = findHist("ECL/fail_shaperid");
  unsigned short shaper = 0;
  if (h_fail_crateid != NULL && h_fail_shaperid != NULL) {
    for (unsigned short i = 0; i < 52; i++) {
      if (h_fail_crateid->GetBinContent(i + 1)) {
        for (unsigned short j = i * 12; j < i * 12 + 12; j++) {
          if (h_fail_shaperid->GetBinContent(j + 1)) {
            shaper = (j + 1) - 12 * i;
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
    if (h_logic_summary->GetMaximum() > 0 && h_logic_summary->GetMaximum() < 10) c_logic_summary->Pad()->SetFillColor(kYellow);
    if (h_logic_summary->GetMaximum() >= 10) c_logic_summary->Pad()->SetFillColor(kRed);
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
