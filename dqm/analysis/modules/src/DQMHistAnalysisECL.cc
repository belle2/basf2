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

using namespace Belle2;

REG_MODULE(DQMHistAnalysisECL)

DQMHistAnalysisECLModule::DQMHistAnalysisECLModule()
  : DQMHistAnalysisModule()
{
  B2DEBUG(20, "DQMHistAnalysisECL: Constructor done.");
}


DQMHistAnalysisECLModule::~DQMHistAnalysisECLModule() { }

void DQMHistAnalysisECLModule::initialize()
{
  gROOT->cd();
  B2DEBUG(20, "DQMHistAnalysisECL: initialized.");

  //Boarder for trigtag2_trigid histogram.
  m_line1 = new TLine(1, 0, 53, 0);
  m_line2 = new TLine(1, 1, 53, 1); //boarder for trigtag2_trigid
  m_line1->SetLineWidth(3);
  m_line2->SetLineWidth(3);
  m_line1->SetLineColor(kBlue);
  m_line2->SetLineColor(kBlue);

  //New TCanvas & TH1F's for time offsets
  c_crate_time_offsets = new TCanvas("ECL/c_crate_time_offsets");

  h_crate_time_offsets = new TH1F("t_off", "Crate time offsets (Thr = 1 GeV)", 52, 0.5, 52.5);
  h_crate_time_offsets->SetMarkerColor(kBlue);
  h_crate_time_offsets->SetMarkerStyle(21);
  h_crate_time_offsets->SetLineColor(kBlue);

  h_crate_time_offsets_ref = new TH1F("t_off_ref", "Crate time offsets (Thr = 1 GeV)", 52, 0.5, 52.5);
  h_crate_time_offsets_ref->SetMarkerColor(kGreen);
  h_crate_time_offsets_ref->SetMarkerStyle(20);
  h_crate_time_offsets_ref->SetLineColor(kGreen);

  //New THStack for time offsets
  hs = new THStack("hs_off", "Crate time offsets (Thr = 1 GeV), [ns]");

  //New TLegend for time offsets
  m_leg = new TLegend(0.8, 0.8, 0.95, 0.95);
  m_leg->SetTextFont(42);

  box1 = new TWbox(0, 0, 1, 1, kWhite, 1, 1);
  box1->SetFillColor(kRed);
  box2 = new TWbox(0, 0, 1, 1, kWhite, 1, 1);
  box2->SetFillColor(kYellow);
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

void DQMHistAnalysisECLModule::timeCrate(TH1F* h, Int_t* st)
{

  for (int i = 0; i < 52; i++) {
    st[i] = 0;
    std::string h_title = "ECL/time_crate_" + std::to_string(i + 1) + "_Thr1GeV";
    h_time_crate_Thr1GeV = findHist(h_title);
    if (h_time_crate_Thr1GeV != NULL) {
      TH1* hclone = h_time_crate_Thr1GeV->Rebin(2, "hclone");
      hclone->GetXaxis()->SetRange(2020, 2100);
      if (hclone->GetEntries() < 100) st[i] = 1;
      h->SetBinContent(i + 1, hclone->GetMean());
      h->SetBinError(i + 1, hclone->GetMeanError());
      delete hclone;
    } else {
      st[i] = 1;
      h->SetBinContent(i + 1, 0);
      h->SetBinError(i + 1, 0);
    }
  }
}


void DQMHistAnalysisECLModule::event()
{
  B2DEBUG(20, "DQMHistAnalysisECL: event called");

  //adc_flag histo title renewal
  TCanvas* c_adc_flag = findCanv("ECL/c_adc_flag");
  TH1* h_adc_flag = findHist("ECL/adc_flag");
  if (c_adc_flag != NULL && h_adc_flag != NULL) {
    std::string h_adc_flag_title = str(boost::format("Flag of ADC samples (%1%, %2%)") % (h_adc_flag->GetBinContent(
                                         2) / h_adc_flag->GetBinContent(1)) %
                                       (h_adc_flag->GetBinContent(3) / h_adc_flag->GetBinContent(1)));
    h_adc_flag->SetTitle(h_adc_flag_title.c_str());
    c_adc_flag->Clear();
    c_adc_flag->cd();
    h_adc_flag->Draw();
    c_adc_flag->Modified();
    c_adc_flag->Update();
  }

  //2D histos (Shifter plots only) color alert
  //trigtag2_trigid
  TCanvas* c_trigtag2_trigid = findCanv("ECL/c_trigtag2_trigid");
  TH1* h_trigtag2_trigid = findHist("ECL/trigtag2_trigid");
  if (c_trigtag2_trigid != NULL && h_trigtag2_trigid != NULL) {
    c_trigtag2_trigid->cd();
    for (int i = 0; i  < 52; i++) {
      if (h_trigtag2_trigid->GetBinContent(h_trigtag2_trigid->GetBin(i + 1, 3))) {
        c_trigtag2_trigid->Pad()->SetFillColor(kRed);
        break;
      }
    }
    m_line1->Draw();
    m_line2->Draw();
  }

  //quality_fit_data
  TCanvas* c_quality_fit_data = findCanv("ECL/c_quality_fit_data");
  TH1* h_quality_fit_data = findHist("ECL/quality_fit_data");
  if (c_quality_fit_data != NULL && h_quality_fit_data != NULL) {
    c_quality_fit_data->cd();
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        if ((i == 0 && j == 3) || (i == 3 && j == 0)) break; //to exclude color alert for known problems
        if (h_quality_fit_data->GetBinContent(h_quality_fit_data->GetBin(i + 1, j + 1) > 0)) {
          c_quality_fit_data->Pad()->SetFillColor(kRed); //to initiate color alert for unknown errors!
          break;
        }
      }
    }
  }

  //_time_crate_%1%_Thr1GeV
  timeCrate(h_crate_time_offsets, stat);

  hs->RecursiveRemove(h_crate_time_offsets);
  if (h_crate_time_offsets_ref->GetEntries()) hs->RecursiveRemove(h_crate_time_offsets_ref);

  hs->Add(h_crate_time_offsets);
  if (h_crate_time_offsets_ref->GetEntries()) hs->Add(h_crate_time_offsets_ref);

  c_crate_time_offsets->Clear();
  c_crate_time_offsets->cd();
  hs->Draw("nostack,e1p");
  c_crate_time_offsets->Modified();
  c_crate_time_offsets->Update();

  for (int i = 0; i < 52; i++) {
    Int_t bin_index = hs->GetXaxis()->FindBin((Double_t)i + 1);
    std::string label = std::to_string(i + 1);
    hs->GetXaxis()->SetBinLabel(bin_index, label.c_str());
    hs->GetXaxis()->SetLabelSize(0.03);
  }

  if (h_crate_time_offsets_ref->GetEntries()) {
    Double_t ymin = gPad->GetUymin();
    Double_t ymax = gPad->GetUymax();
    for (int i = 0; i < 52; i++) {
      TWbox* box = new TWbox(i + 0.5, ymin, i + 1.5, ymax, 0, 1, -1);
      combined_err[i] = sqrt(TMath::Power(h_crate_time_offsets->GetBinError(i + 1),
                                          2) + TMath::Power(h_crate_time_offsets_ref->GetBinError(i + 1), 2));
      if (combined_err[i]) {
        nsigmas[i] = (h_crate_time_offsets->GetBinContent(i + 1) - h_crate_time_offsets_ref->GetBinContent(i + 1)) / combined_err[i];
        if (nsigmas[i] < 0) nsigmas[i] = -nsigmas[i];
        if (nsigmas[i] > m_level) box->SetFillColor(kRed);
      } else box->SetFillColor(kRed);
      if (stat[i] == 1 || stat_ref[i] == 1) box->SetFillColor(kYellow);
      box->Draw();
    }
    hs->Draw("nostack,e1p,same");
  }

  m_leg->AddEntry(h_crate_time_offsets, "Current run" , "P");
  if (h_crate_time_offsets_ref->GetEntries()) {
    m_leg->AddEntry(h_crate_time_offsets_ref, "Previous run (reference)" , "P");
    m_leg->AddEntry(box1, "Large offset", "F");
    m_leg->AddEntry(box2, "Low statistics", "F");
  }
  m_leg->Draw();

  hs->GetXaxis()->SetTitle("Crate ID");

  if (h_crate_time_offsets_ref->GetEntries()) {
    TFrame* frame = gPad->GetFrame();
    frame->SetLineColor(kRed);
  }

  c_crate_time_offsets->Modified();
  c_crate_time_offsets->Update();
}

void DQMHistAnalysisECLModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisECL: endRun called");
  timeCrate(h_crate_time_offsets_ref, stat_ref);
}


void DQMHistAnalysisECLModule::terminate()
{
  B2DEBUG(20, "terminate called");
}

