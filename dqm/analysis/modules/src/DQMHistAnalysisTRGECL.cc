/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisTRGECL.cc
// Description : DQM analysis for ECL trigger
//
#include <dqm/analysis/modules/DQMHistAnalysisTRGECL.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TString.h>
#include <TFitResult.h>
#include <TMath.h>
#include <TF1.h>
#include <iomanip>
#include <iostream>

using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisTRGECL);
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
DQMHistAnalysisTRGECLModule::DQMHistAnalysisTRGECLModule()
  : DQMHistAnalysisModule()
{
  setDescription("Module for DQM histogram analysis of ECL trigger Event T0 DQM histograms");

  addParam("MinEntryForFit",
           m_MinEntryForFit,
           "set minimum number of entries for fit (default=200)",
           m_MinEntryForFit);
}

DQMHistAnalysisTRGECLModule::~DQMHistAnalysisTRGECLModule() { }

void DQMHistAnalysisTRGECLModule::initialize()
{

  gROOT->cd();

  // canvas for event timing related histograms
  c_TCEFraction =
    new TCanvas("TRGECLEventTiming/c_a_EventTimingFraction",
                "Event timing fraction",
                300, 300);
  c_EventT0Mean =
    new TCanvas("TRGECLEventTiming/c_a_EventT0Mean",
                "EventT0 mean",
                300, 300);
  c_EventT0Width =
    new TCanvas("TRGECLEventTiming/c_a_EventT0Width",
                "EventT0 width",
                300, 300);

  // TGraph for fraction of event timing
  h_EventTimingEnergyFraction = new TGraph();
  h_EventTimingEnergyFraction->SetTitle("[TRGECL] Fraction of event timing");
  h_EventTimingEnergyFraction->GetXaxis()->SetTitle("max TC Energy (ADC)");
  h_EventTimingEnergyFraction->GetYaxis()->SetTitle("N(max TC E > X) / N(all)");
  h_EventTimingEnergyFraction->SetMarkerStyle(20);
  h_EventTimingEnergyFraction->SetMarkerSize(0.5);
  h_EventTimingEnergyFraction->SetMarkerColor(4);
  h_EventTimingEnergyFraction->SetLineStyle(1);
  h_EventTimingEnergyFraction->SetLineWidth(1);
  h_EventTimingEnergyFraction->SetLineColor(4);
  h_EventTimingEnergyFraction->SetMinimum(0);

  // set EventT0 histogram names
  s_histNameEventT0 = std::vector<std::string>(15, "");
  for (int iii = 0; iii < 15; iii++) {
    std::stringstream ss1, ss2;
    ss1 << std::setfill('0') << std::setw(3) << std::to_string(10 + 20 * iii);
    ss2 << std::setfill('0') << std::setw(3) << std::to_string(10 + 20 * (iii + 1));
    std::string s_EnergyRange = ss1.str() + "to" + ss2.str();
    s_histNameEventT0[iii] = "TRGECLEventTiming/h_EventT0_MaxTCE_" + s_EnergyRange;
  }

  // EventT0 mean
  h_EventT0Mean = new TGraphErrors();
  h_EventT0Mean->SetTitle("[TRGECL] Event T0 mean");
  h_EventT0Mean->GetXaxis()->SetTitle("max TC Energy (ADC)");
  h_EventT0Mean->GetYaxis()->SetTitle("EventT0 mean (ns)");
  h_EventT0Mean->SetMarkerStyle(20);
  h_EventT0Mean->SetMarkerSize(0.5);
  h_EventT0Mean->SetMarkerColor(4);
  h_EventT0Mean->SetLineStyle(1);
  h_EventT0Mean->SetLineWidth(1);
  h_EventT0Mean->SetLineColor(4);

  // EventT0 width
  h_EventT0Width = new TGraphErrors();
  h_EventT0Width->SetTitle("[TRGECL] Event T0 width");
  h_EventT0Width->GetXaxis()->SetTitle("max TC Energy (ADC)");
  h_EventT0Width->GetYaxis()->SetTitle("EventT0 width (ns)");
  h_EventT0Width->SetMarkerStyle(20);
  h_EventT0Width->SetMarkerSize(0.5);
  h_EventT0Width->SetMarkerColor(4);
  h_EventT0Width->SetLineStyle(1);
  h_EventT0Width->SetLineWidth(1);
  h_EventT0Width->SetLineColor(4);
  h_EventT0Width->SetMinimum(0);

}

void DQMHistAnalysisTRGECLModule::beginRun()
{
  c_TCEFraction->Clear();
  c_EventT0Mean->Clear();
  c_EventT0Width->Clear();
}

void DQMHistAnalysisTRGECLModule::endRun()
{
}

void DQMHistAnalysisTRGECLModule::event()
{

  auto hhh = (TH1F*) findHist("TRGECLEventTiming/h_MaxTCE");
  if (hhh != nullptr) {
    // calculate fraction of event timing with max TC E threshold
    int n_bin = hhh->GetNbinsX();
    float n_entry_all = (float) hhh->GetEffectiveEntries();
    float n_entry_bin_sum[140] = {0};
    float ratio[140] = {0};
    for (int iii = 0; iii < n_bin; iii++) {
      for (int jjj = iii; jjj < n_bin; jjj++) {
        n_entry_bin_sum[iii] += (float) hhh->GetBinContent(jjj + 1);
      }
      ratio[iii] = n_entry_bin_sum[iii] / n_entry_all;
    }
    // fill value to TGraph
    for (int iii = 0; iii <= 30; iii++) {
      h_EventTimingEnergyFraction->SetPoint(iii, iii * 10, ratio[iii]);
    }
    c_TCEFraction->cd();
    c_TCEFraction->Clear();
    h_EventTimingEnergyFraction->Draw("AP");
    c_TCEFraction->Modified();
    c_TCEFraction->Update();
    UpdateCanvas(c_TCEFraction);
  }

  // EventT0 mean canvas
  c_EventT0Mean->cd();
  c_EventT0Mean->Clear();

  // set TGraphErrors for EventT0 mean and width
  getEventT0(s_histNameEventT0,
             h_EventT0Mean,
             h_EventT0Width);

  // EventT0 mean
  h_EventT0Mean->Draw("AP");
  c_EventT0Mean->Modified();
  c_EventT0Mean->Update();
  UpdateCanvas(c_EventT0Mean);

  // EventT0 width canvas
  c_EventT0Width->cd();
  c_EventT0Width->Clear();

  // EventT0 width
  h_EventT0Width->Draw("AP");
  c_EventT0Width->Modified();
  c_EventT0Width->Update();
  UpdateCanvas(c_EventT0Width);


}

void DQMHistAnalysisTRGECLModule::terminate()
{
  delete c_TCEFraction;
  delete c_EventT0Mean;
  delete c_EventT0Width;
  delete h_EventT0Mean;
  delete h_EventT0Width;
}

double DQMHistAnalysisTRGECLModule::fGaus(double* x, double* par)
{
  double yield = par[0];
  double mean  = par[1];
  double sigma = par[2];
  return yield * TMath::Gaus(x[0], mean, sigma);
}

void DQMHistAnalysisTRGECLModule::getEventT0(std::vector<std::string> s_HistName,
                                             TGraphErrors* h_tge_mean,
                                             TGraphErrors* h_tge_width)
{

  // loop for fits on 15 sets of EventT0
  for (int iii = 0; iii < 15; iii++) {

    TH1* hhh = findHist(s_HistName[iii]);

    if (hhh != nullptr) {

      // fit 6 parameters (yield, yieled err, mean, mean err, sigma, sigma err)
      std::vector<double> par_fit(6, 0.0);

      // check the number of entry in histogram to fit
      int nToFit = hhh->GetEffectiveEntries();
      if (nToFit > m_MinEntryForFit &&
          nToFit > 200) {

        // perform fit on EventT0 and get peak position and width
        fitEventT0(hhh, par_fit);

      }

      // set mean parameter to TGraphErrors
      h_tge_mean->SetPoint(iii, (iii + 1) * 20, par_fit[2]);
      h_tge_mean->SetPointError(iii, 10, par_fit[3]);
      // set width parameter to TGraphErrors
      h_tge_width->SetPoint(iii, (iii + 1) * 20, par_fit[4]);
      h_tge_width->SetPointError(iii, 10, par_fit[5]);

    }
  }
}

void DQMHistAnalysisTRGECLModule::fitEventT0(TH1* hist,
                                             std::vector<double>& fit_par)
{

  // initial parameters to fit on EventT0 from histogram statistics
  float v_mean = hist->GetMean();
  float v_rms  = hist->GetRMS();
  float v_norm = hist->GetEffectiveEntries() / v_rms;
  float x_min = v_mean - 3 * v_rms;
  float x_max = v_mean + 3 * v_rms;

  // Gaussian function for fit on EventT0
  TF1* f1 = new TF1("f1", DQMHistAnalysisTRGECLModule::fGaus,
                    x_min,
                    x_max,
                    3);
  // set initial parameters of Gaussian
  f1->SetParameters(v_norm, v_mean, v_rms);

  // perform fit
  hist->Fit("f1", "Q", "", x_min, x_max);

  //
  if (hist->GetFunction("f1") == NULL) {
    B2DEBUG(20, "Fit failed");
    return;
  }

  //
  if (f1->GetParameter(2) <= 0) {
    B2DEBUG(20, "Fit failed");
    return;
  }

  // get fit parameters
  fit_par[0] = f1->GetParameter(0);
  fit_par[1] = f1->GetParError(0);
  fit_par[2] = f1->GetParameter(1);
  fit_par[3] = f1->GetParError(1);
  fit_par[4] = f1->GetParameter(2);
  fit_par[5] = f1->GetParError(2);

  // delete fit function
  delete f1;
}
