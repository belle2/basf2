/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisEventT0.cc
// Description : module for DQM histogram analysis of SVD sensors occupancies
//
// Author : Giulia Casarosa (PI)
// Date : 20210210
//-


#include <dqm/analysis/modules/DQMHistAnalysisEventT0.h>

#include <TROOT.h>
#include <TStyle.h>
#include <TString.h>
#include <TFitResult.h>

#include <TMath.h>
#include <iostream>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisEventT0)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisEventT0Module::DQMHistAnalysisEventT0Module()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("min_nEntries", m_nEntriesMin, "minimum numeber of entries to process the histogram", m_nEntriesMin);
  addParam("prefixCanvas", m_prefixCanvas, "prefix to be added to canvas filename when saved as pdf", std::string("c"));
  addParam("printCanvas", m_printCanvas, "if True prints pdf of the analysis canvas", bool(false));
}


DQMHistAnalysisEventT0Module::~DQMHistAnalysisEventT0Module() { }

void DQMHistAnalysisEventT0Module::initialize()
{
  gROOT->cd();

  //ECLTRG canvas
  m_cECLTRG = new TCanvas("EventT0/c_ECLTRGjitter", "ECLTRG jitter", 1200, 400);
  m_pad1ECLTRG = new TPad("pad1ECLTRG", "pad1 ECLTRG", 0.03, 0.02, 0.33, 0.98);
  m_pad2ECLTRG = new TPad("pad2ECLTRG", "pad2 ECLTRG", 0.35, 0.02, 0.65, 0.98);
  m_pad3ECLTRG = new TPad("pad3ECLTRG", "pad3 ECLTRG", 0.67, 0.02, 0.97, 0.98);

  //CDCTRG canvases
  m_cCDCTRG = new TCanvas("EventT0/c_CDCTRGjitter", "CDCTRG jitter", 1200, 400);
  m_pad1CDCTRG = new TPad("pad1CDCTRG", "pad1 CDCTRG", 0.03, 0.02, 0.33, 0.98);
  m_pad2CDCTRG = new TPad("pad2CDCTRG", "pad2 CDCTRG", 0.35, 0.02, 0.65, 0.98);
  m_pad3CDCTRG = new TPad("pad3CDCTRG", "pad3 CDCTRG", 0.67, 0.02, 0.97, 0.98);

  m_monObj = getMonitoringObject("eventT0");
  m_monObj->addCanvas(m_cECLTRG);
  m_monObj->addCanvas(m_cCDCTRG);
}


void DQMHistAnalysisEventT0Module::beginRun()
{
  m_cECLTRG->Clear();
  m_cCDCTRG->Clear();
}

void DQMHistAnalysisEventT0Module::endRun()
{


  // --- ECLTRG ---

  //find TOP EventT0 Hadrons ECLTRG histogram and process it
  TH1* h = findHist("EventT0DQMdir/m_histEventT0_TOP_hadron_L1_ECLTRG");
  TString tag = "hadronECLTRG";
  m_pad1ECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_pad1ECLTRG->SetFillColor(0);
    m_pad1ECLTRG->Modified();
    m_pad1ECLTRG->Update();
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    h->Draw();
    m_pad1ECLTRG->SetFillColor(kGray);
  }

  //find TOP EventT0 Bhabhas ECLTRG histogram and process it
  h = findHist("EventT0DQMdir/m_histEventT0_TOP_bhabha_L1_ECLTRG");
  tag = "bhabhaECLTRG";
  m_pad2ECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_pad2ECLTRG->SetFillColor(0);
    m_pad2ECLTRG->Modified();
    m_pad2ECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    h->Draw();
    m_pad2ECLTRG->SetFillColor(kGray);
  }


  //find TOP EventT0 Mumus ECLTRG histogram and process it
  h = findHist("EventT0DQMdir/m_histEventT0_TOP_mumu_L1_ECLTRG");
  tag = "mumuECLTRG";
  m_pad3ECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_pad3ECLTRG->SetFillColor(0);
    m_pad3ECLTRG->Modified();
    m_pad3ECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    h->Draw();
    m_pad3ECLTRG->SetFillColor(kGray);
  }

  m_cECLTRG->cd();
  m_pad1ECLTRG->Draw();
  m_pad2ECLTRG->Draw();
  m_pad3ECLTRG->Draw();

  if (m_printCanvas)
    m_cECLTRG->Print(Form("%s_ECLTRG.pdf", m_prefixCanvas.c_str()));


  // --- CDCTRG ---

  //find TOP EventT0 Hadrons CDCTRG histogram and process it
  h = findHist("EventT0DQMdir/m_histEventT0_TOP_hadron_L1_CDCTRG");
  tag = "hadronCDCTRG";
  m_pad1CDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_pad1CDCTRG->SetFillColor(0);
    m_pad1CDCTRG->Modified();
    m_pad1CDCTRG->Update();
    m_cCDCTRG->cd();
    m_pad1CDCTRG->Draw();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    h->Draw();
    m_pad1CDCTRG->SetFillColor(kGray);
    m_cCDCTRG->cd();
    m_pad1CDCTRG->Draw();
  }

  //find TOP EventT0 Bhabhas CDCTRG histogram and process it
  h = findHist("EventT0DQMdir/m_histEventT0_TOP_bhabha_L1_CDCTRG");
  tag = "bhabhaCDCTRG";
  m_pad2CDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_pad2CDCTRG->SetFillColor(0);
    m_pad2CDCTRG->Modified();
    m_pad2CDCTRG->Update();
    m_cCDCTRG->cd();
    m_pad2CDCTRG->Draw();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    h->Draw();
    m_pad2CDCTRG->SetFillColor(kGray);
    m_cCDCTRG->cd();
    m_pad2CDCTRG->Draw();
  }


  //find TOP EventT0 Mumus CDCTRG histogram and process it
  h = findHist("EventT0DQMdir/m_histEventT0_TOP_mumu_L1_CDCTRG");
  tag = "mumuCDCTRG";
  m_pad3CDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_pad3CDCTRG->SetFillColor(0);
    m_pad3CDCTRG->Modified();
    m_pad3CDCTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    h->Draw();
    m_pad3CDCTRG->SetFillColor(kGray);
  }

  m_cCDCTRG->cd();
  m_pad1CDCTRG->Draw();
  m_pad2CDCTRG->Draw();
  m_pad3CDCTRG->Draw();

  if (m_printCanvas)
    m_cCDCTRG->Print(Form("%s_CDCTRG.pdf", m_prefixCanvas.c_str()));


}

void DQMHistAnalysisEventT0Module::terminate()
{

  delete m_cECLTRG;
  delete m_cCDCTRG;

}


double DQMHistAnalysisEventT0Module::fDoubleGaus(double* x, double* par)
{
  double N = par[0];
  double frac = par[1];
  double mean = par[2];
  double sigma = par[3];
  double mean2 = par[4];
  double sigma2 = par[5];

  return N * frac * TMath::Gaus(x[0], mean, sigma) + N * (1 - frac) * TMath::Gaus(x[0], mean2, sigma2);
}

bool DQMHistAnalysisEventT0Module::processHistogram(TH1* h,  TString tag)
{

  if (h == nullptr) {
    B2DEBUG(20, "h == nullptr");
    m_monObj->setVariable(Form("fit_%s", tag.Data()), 0);
    return  false;
  }

  int nToFit = h->GetEntries();// - h->GetBinContent(0) - h->GetBinContent(h->GetNbinsX()+1);
  if (nToFit < m_nEntriesMin) {
    B2DEBUG(20, "not enough entries");
    m_monObj->setVariable(Form("fit_%s", tag.Data()), 0);
    return false;
  }


  //scale the histogram
  h->Scale(1. / h->GetEntries());
  h->GetXaxis()->SetRangeUser(-50, 50);

  //define the fitting function
  TF1 fitf("fit", DQMHistAnalysisEventT0Module::fDoubleGaus, -50, 50, 6);
  fitf.SetParNames("N", "f_{1}", "#mu_{1}", "#sigma_{1}", "#mu_{2}", "#sigma_{2}");
  fitf.SetParameters(0.1, 0.8, 0, 5, 0, 15);
  fitf.SetParLimits(1, 0, 1); //fraction
  fitf.SetParLimits(3, 0, 100); //sigma1
  fitf.SetParLimits(5, 0, 100); //sigma2

  if (h->Fit(&fitf, "SR+") != 0) {
    B2DEBUG(20, "failed fit");
    m_monObj->setVariable(Form("fit_%s", tag.Data()), 0);
    return false;
  }

  Double_t par[6];
  fitf.GetParameters(&par[0]);
  Double_t parErr[6];
  for (int i = 0; i < 6; i++)
    parErr[i] = fitf.GetParError(i) ;


  //define gaussian components
  TF1 gauss1("gauss1", "gaus", -100, 100);
  TF1 gauss2("gauss2", "gaus", -100, 100);

  gauss1.SetLineColor(kBlue);
  gauss1.SetLineStyle(kDashed);
  gauss1.SetParameters(par[0]*par[1], par[2], par[3]);

  gauss2.SetLineColor(kBlue);
  gauss2.SetLineStyle(kDashed);
  gauss2.SetParameters(par[0] * (1 - par[1]), par[4], par[5]);

  m_monObj->setVariable(Form("fit_%s", tag.Data()), 1);
  m_monObj->setVariable(Form("N_%s", tag.Data()), h->GetEntries(), TMath::Sqrt(h->GetEntries()));
  m_monObj->setVariable(Form("f_%s", tag.Data()), par[1], parErr[1]);
  m_monObj->setVariable(Form("mean1_%s", tag.Data()), par[2], parErr[2]);
  m_monObj->setVariable(Form("sigma1_%s", tag.Data()), par[3], parErr[3]);
  m_monObj->setVariable(Form("mean2_%s", tag.Data()), par[4], parErr[4]);
  m_monObj->setVariable(Form("sigma2_%s", tag.Data()), par[5], parErr[5]);

  //SETUP gSTYLE - all plots
  gStyle->SetOptFit(1111);

  h->DrawClone();
  fitf.DrawClone("same");
  gauss1.DrawClone("same");
  gauss2.DrawClone("same");

  return true;

}

