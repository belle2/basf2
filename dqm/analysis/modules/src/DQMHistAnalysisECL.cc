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
  m_line3 = new TLine(0, 2000, 8736, 2000);
  m_line4 = new TLine(0, 7000, 8736, 7000);
  m_line1->SetLineWidth(3);
  m_line2->SetLineWidth(3);
  m_line1->SetLineColor(kBlue);
  m_line2->SetLineColor(kBlue);
  m_line3->SetLineWidth(3);
  m_line4->SetLineWidth(3);
  m_line3->SetLineColor(kBlue);
  m_line4->SetLineColor(kBlue);


  //New TCanvas for adc_flag
  c_adc_flag_title = new TCanvas("ECL/c_adc_flag_title");

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
  m_leg->AddEntry(h_crate_time_offsets, "Current run" , "P");
  if (h_crate_time_offsets_ref->GetEntries()) {
    hs->RecursiveRemove(h_crate_time_offsets_ref);
    hs->Add(h_crate_time_offsets_ref);
    m_leg->AddEntry(h_crate_time_offsets_ref, "Reference run " , "P");
  }
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

  //adc_flag
  TH1* h_adc_flag = findHist("ECL/adc_flag");
  if (h_adc_flag != NULL) {
    std::string h_adc_flag_title = str(boost::format("Flag of ADC samples (%1%, %2%)") % (h_adc_flag->GetBinContent(
                                         2) / h_adc_flag->GetBinContent(1)) %
                                       (h_adc_flag->GetBinContent(3) / h_adc_flag->GetBinContent(1)));
    h_adc_flag->SetTitle(h_adc_flag_title.c_str());
    c_adc_flag_title->Clear();
    c_adc_flag_title->cd();
    h_adc_flag->Draw();
    c_adc_flag_title->Modified();
    c_adc_flag_title->Update();
  }

  //1D histos color alert
  //trigtag1
  TCanvas* c_trigtag1 = findCanv("ECL/c_trigtag1");
  c_trigtag1->cd();
  c_trigtag1->Pad()->SetFillColor(kWhite);
  TH1* h_trigtag1 = findHist("ECL/trigtag1");
  if (h_trigtag1 != NULL) {
    if (h_trigtag1->GetBinContent(2)) c_trigtag1->Pad()->SetFillColor(kRed);
  }
  c_trigtag1->Draw();
  c_trigtag1->Modified();
  c_trigtag1->Update();

  //adc_hits
  TCanvas* c_adc_hits = findCanv("ECL/c_adc_hits");
  c_adc_hits->cd();
  c_adc_hits->Pad()->SetFillColor(kWhite);
  TH1* h_adc_hits = findHist("ECL/adc_hits");
  if (h_adc_hits != NULL) {
    for (int i = 50; i < 250; i++) {
      if (h_adc_hits->GetBinContent(i + 1)) {
        c_adc_hits->Pad()->SetFillColor(kRed);
        break;
      }
    }
  }
  c_adc_hits->Draw();
  c_adc_hits->Modified();
  c_adc_hits->Update();


  //2D histos color alert
  //trigtag2_trigid
  TCanvas* c_trigtag2_trigid = findCanv("ECL/c_trigtag2_trigid");
  c_trigtag2_trigid->cd();
  c_trigtag2_trigid->Pad()->SetFillColor(kWhite);
  TH1* h_trigtag2_trigid = findHist("ECL/trigtag2_trigid");
  if (h_trigtag2_trigid != NULL) {
    for (int i = 0; i  < 52; i++) {
      if (h_trigtag2_trigid->GetBinContent(h_trigtag2_trigid->GetBin(i + 1, 3))) {
        c_trigtag2_trigid->Pad()->SetFillColor(kRed);
        break;
      }
    }
  }
  m_line1->Draw();
  m_line2->Draw();
  c_trigtag2_trigid->Draw();
  c_trigtag2_trigid->Modified();
  c_trigtag2_trigid->Update();

  //quality_fit_data
  TCanvas* c_quality_fit_data = findCanv("ECL/c_quality_fit_data");
  c_quality_fit_data->cd();
  c_quality_fit_data->Pad()->SetFillColor(kWhite);
  TH1* h_quality_fit_data = findHist("ECL/quality_fit_data");
  if (h_quality_fit_data != NULL) {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        if ((i == 0 && j == 3) || (i == 3 && j == 0)) break; //to exclude color alert for known problems
        if (h_quality_fit_data->GetBinContent(h_quality_fit_data->GetBin(i + 1, j + 1)) > 0) {
          c_quality_fit_data->Pad()->SetFillColor(kRed); //to initiate color alert for unknown errors!
          break;
        }
      }
    }
  }
  c_quality_fit_data->Draw();
  c_quality_fit_data->Modified();
  c_quality_fit_data->Update();

  //pedmean_cellid
  TCanvas* c_pedmean_cellid = findCanv("ECL/c_pedmean_cellid");
  c_pedmean_cellid->cd();
  c_pedmean_cellid->Pad()->SetFillColor(kWhite);
  TH1* h_pedmean_cellid = findHist("ECL/pedmean_cellid");
  double ymin = h_pedmean_cellid->GetYaxis()->GetXmin();
  double ymax = h_pedmean_cellid->GetYaxis()->GetXmax();
  double binwidth = (ymax - ymin) / 200.;
  if (h_pedmean_cellid != NULL) {
    for (int i = 0; i < 8736; i++) {
      for (int j = 0; j < 200; j++) {
        if ((j > (7000. - ymin) / binwidth || j < (2000. - ymin) / binwidth)
            && h_pedmean_cellid->GetBinContent(h_pedmean_cellid->GetBin(i + 1, j + 1)) > 0) {
          c_pedmean_cellid->Pad()->SetFillColor(kRed); //to initiate color alert for unknown errors!
          break;
        }
      }
    }
  }
  m_line3->Draw();
  m_line4->Draw();
  c_pedmean_cellid->Draw();
  c_pedmean_cellid->Modified();
  c_pedmean_cellid->Update();

  //_time_crate_%1%_Thr1GeV
  hs->RecursiveRemove(h_crate_time_offsets);
  timeCrate(h_crate_time_offsets, stat);
  hs->Add(h_crate_time_offsets);

  c_crate_time_offsets->Clear();
  c_crate_time_offsets->cd();
  hs->Draw("nostack,e1p");

  for (int i = 0; i < 52; i++) {
    Int_t bin_index = hs->GetXaxis()->FindBin((Double_t)i + 1);
    std::string label = std::to_string(i + 1);
    hs->GetXaxis()->SetBinLabel(bin_index, label.c_str());
    hs->GetXaxis()->SetLabelSize(0.03);
  }

  hs->GetXaxis()->SetTitle("Crate ID");

  m_leg->Draw();

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
  delete m_line1;
  delete m_line2;
  delete m_line3;
  delete m_line4;
  delete c_adc_flag_title;
  delete c_crate_time_offsets;
  delete hs;
  delete m_leg;
  delete box1;
  delete box2;
  delete h_time_crate_Thr1GeV;
  delete h_crate_time_offsets;
  delete h_crate_time_offsets_ref;
}
