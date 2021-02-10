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
  addParam("printCanvas", m_printCanvas, "if True prints pdf of the analysis canvas", bool(false));
}


DQMHistAnalysisEventT0Module::~DQMHistAnalysisEventT0Module() { }

void DQMHistAnalysisEventT0Module::initialize()
{
  gROOT->cd();

  //ECLTRG canvases
  m_cHadronECLTRG = new TCanvas("EventT0/c_HadronECLTRGjitter");
  m_cMumuECLTRG = new TCanvas("EventT0/c_MumuECLTRGjitter");
  m_cBhabhaECLTRG = new TCanvas("EventT0/c_BhabhaECLTRGjitter");
  //CDCTRG canvases
  m_cHadronCDCTRG = new TCanvas("EventT0/c_HadronCDCTRGjitter");
  m_cMumuCDCTRG = new TCanvas("EventT0/c_MumuCDCTRGjitter");
  m_cBhabhaCDCTRG = new TCanvas("EventT0/c_BhabhaCDCTRGjitter");

  m_monObj = getMonitoringObject("eventT0");
  m_monObj->addCanvas(m_cHadronECLTRG);
  m_monObj->addCanvas(m_cBhabhaECLTRG);
  m_monObj->addCanvas(m_cMumuECLTRG);
  m_monObj->addCanvas(m_cHadronCDCTRG);
  m_monObj->addCanvas(m_cBhabhaCDCTRG);
  m_monObj->addCanvas(m_cMumuCDCTRG);
}


void DQMHistAnalysisEventT0Module::beginRun()
{
  m_cHadronECLTRG->Clear();
  m_cMumuECLTRG->Clear();
  m_cBhabhaECLTRG->Clear();
  m_cHadronCDCTRG->Clear();
  m_cMumuCDCTRG->Clear();
  m_cBhabhaCDCTRG->Clear();
}

void DQMHistAnalysisEventT0Module::event()
{

  //SETUP gSTYLE - all plots
  gStyle->SetOptFit(1111);

  //define the fitting function
  TF1* fitf = new TF1("fit", DQMHistAnalysisEventT0Module::fDoubleGaus, -100, 100, 6);
  fitf->SetParNames("N", "f_{1}", "#mu_{1}", "#sigma_{1}", "#mu_{2}", "#sigma_{2}");
  fitf->SetParameters(0.1, 0.8, 0, 5, 0, 15);
  fitf->SetParLimits(1, 0, 1); //fraction
  fitf->SetParLimits(3, 0, 100); //sigma1
  fitf->SetParLimits(5, 0, 100); //sigma2


  // --- ECLTRG ---

  //find TOP EventT0 Hadrons ECLTRG histogram and process it
  TH1* h = findHist("EventT0DQMdir/m_histEventT0_TOP_hadron_L1_ECLTRG");
  TString tag = "hadronECLTRG";
  m_cHadronECLTRG->cd();
  if (processHistogram(h, fitf, tag)) {
    m_cHadronECLTRG->SetFillColor(0);
    m_cHadronECLTRG->Modified();
    m_cHadronECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    h->Draw();
    m_cHadronECLTRG->SetFillColor(kGray);
  }
  if (m_printCanvas)
    m_cHadronECLTRG->Print(Form("c_%s.pdf", tag.Data()));

  //find TOP EventT0 Bhabhas ECLTRG histogram and process it
  h = findHist("EventT0DQMdir/m_histEventT0_TOP_bhabha_L1_ECLTRG");
  tag = "bhabhaECLTRG";
  m_cBhabhaECLTRG->cd();
  if (processHistogram(h, fitf, tag)) {
    m_cBhabhaECLTRG->SetFillColor(0);
    m_cBhabhaECLTRG->Modified();
    m_cBhabhaECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    h->Draw();
    m_cBhabhaECLTRG->SetFillColor(kGray);
  }
  if (m_printCanvas)
    m_cBhabhaECLTRG->Print(Form("c_%s.pdf", tag.Data()));


  //find TOP EventT0 Mumus ECLTRG histogram and process it
  h = findHist("EventT0DQMdir/m_histEventT0_TOP_mumu_L1_ECLTRG");
  tag = "mumuECLTRG";
  m_cMumuECLTRG->cd();
  if (processHistogram(h, fitf, tag)) {
    m_cMumuECLTRG->SetFillColor(0);
    m_cMumuECLTRG->Modified();
    m_cMumuECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    h->Draw();
    m_cMumuECLTRG->SetFillColor(kGray);
  }
  if (m_printCanvas)
    m_cMumuECLTRG->Print(Form("c_%s.pdf", tag.Data()));

  // --- CDCTRG ---

  //find TOP EventT0 Hadrons CDCTRG histogram and process it
  h = findHist("EventT0DQMdir/m_histEventT0_TOP_hadron_L1_CDCTRG");
  tag = "hadronCDCTRG";
  m_cHadronCDCTRG->cd();
  if (processHistogram(h, fitf, tag)) {
    m_cHadronCDCTRG->SetFillColor(0);
    m_cHadronCDCTRG->Modified();
    m_cHadronCDCTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    h->Draw();
    m_cHadronCDCTRG->SetFillColor(kGray);
  }
  if (m_printCanvas)
    m_cHadronCDCTRG->Print(Form("c_%s.pdf", tag.Data()));

  //find TOP EventT0 Bhabhas CDCTRG histogram and process it
  h = findHist("EventT0DQMdir/m_histEventT0_TOP_bhabha_L1_CDCTRG");
  tag = "bhabhaCDCTRG";
  m_cBhabhaCDCTRG->cd();
  if (processHistogram(h, fitf, tag)) {
    m_cBhabhaCDCTRG->SetFillColor(0);
    m_cBhabhaCDCTRG->Modified();
    m_cBhabhaCDCTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    h->Draw();
    m_cBhabhaCDCTRG->SetFillColor(kGray);
  }
  if (m_printCanvas)
    m_cBhabhaCDCTRG->Print(Form("c_%s.pdf", tag.Data()));


  //find TOP EventT0 Mumus CDCTRG histogram and process it
  h = findHist("EventT0DQMdir/m_histEventT0_TOP_mumu_L1_CDCTRG");
  tag = "mumuCDCTRG";
  m_cMumuCDCTRG->cd();
  if (processHistogram(h, fitf, tag)) {
    m_cMumuCDCTRG->SetFillColor(0);
    m_cMumuCDCTRG->Modified();
    m_cMumuCDCTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    h->Draw();
    m_cMumuCDCTRG->SetFillColor(kGray);
  }
  if (m_printCanvas)
    m_cMumuCDCTRG->Print(Form("c_%s.pdf", tag.Data()));


}

void DQMHistAnalysisEventT0Module::terminate()
{

  delete m_cHadronECLTRG;
  delete m_cBhabhaECLTRG;
  delete m_cMumuECLTRG;
  delete m_cHadronCDCTRG;
  delete m_cBhabhaCDCTRG;
  delete m_cMumuCDCTRG;
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

bool DQMHistAnalysisEventT0Module::processHistogram(TH1* h, TF1* fitf, TString tag)
{

  if (h == NULL) {
    B2WARNING("h == NULL");
    return false;
  }

  int nToFit = h->GetEntries();// - h->GetBinContent(0) - h->GetBinContent(h->GetNbinsX()+1);
  if (nToFit < m_nEntriesMin) {
    B2WARNING("not enough entries");
    return false;
  }


  //scale the histogram
  h->Scale(1. / h->GetEntries());

  if (h->Fit(fitf, "SR+") != 0) {
    B2WARNING("failed fit");
    return false;
  }

  Double_t par[6];
  fitf->GetParameters(&par[0]);
  Double_t parErr[6];
  for (int i = 0; i < 6; i++)
    parErr[i] = fitf->GetParError(i) ;


  //define gaussian components
  TF1* g1 = new TF1("gauss1", "gaus", -100, 100);
  TF1* g2 = new TF1("gauss2", "gaus", -100, 100);

  g1->SetLineColor(kBlue);
  g1->SetLineStyle(kDashed);
  g1->SetParameters(par[0]*par[1], par[2], par[3]);

  g2->SetLineColor(kBlue);
  g2->SetLineStyle(kDashed);
  g2->SetParameters(par[0] * (1 - par[1]), par[4], par[5]);

  m_monObj->setVariable(Form("N_%s", tag.Data()), h->GetEntries());
  m_monObj->setVariable(Form("f_%s", tag.Data()), par[1]);
  m_monObj->setVariable(Form("mean1_%s", tag.Data()), par[2]);
  m_monObj->setVariable(Form("sigma1_%s", tag.Data()), par[3]);
  m_monObj->setVariable(Form("mean2_%s", tag.Data()), par[4]);
  m_monObj->setVariable(Form("sigma2_%s", tag.Data()), par[5]);
  m_monObj->setVariable(Form("dN_%s", tag.Data()), parErr[0]);
  m_monObj->setVariable(Form("df_%s", tag.Data()), parErr[1]);
  m_monObj->setVariable(Form("dmean1_%s", tag.Data()), parErr[2]);
  m_monObj->setVariable(Form("dsigma1_%s", tag.Data()), parErr[3]);
  m_monObj->setVariable(Form("dmean2_%s", tag.Data()), parErr[4]);
  m_monObj->setVariable(Form("dsigma2_%s", tag.Data()), parErr[5]);

  h->Draw();
  fitf->Draw("same");
  g1->Draw("same");
  g2->Draw("same");

  return true;

}
