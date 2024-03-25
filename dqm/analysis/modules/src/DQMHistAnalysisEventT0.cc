/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisEventT0.cc
// Description : module for trigger jitter/EventT0 DQM histogram analysis
//-


#include <dqm/analysis/modules/DQMHistAnalysisEventT0.h>

#include <TROOT.h>
#include <TGraphAsymmErrors.h>
#include <TStyle.h>
#include <TF1.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisEventT0);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisEventT0Module::DQMHistAnalysisEventT0Module()
  : DQMHistAnalysisModule()
{
  setDescription("Determining and processing EventT0s from different subdetector (TOP and SVD for now) for different L1 trigger sources (ECL and CDC for now) to estimate trigger jitter information.");

  //Parameter definition
  addParam("min_nEntries", m_nEntriesMin, "Minimum number of entries to process the histogram.", m_nEntriesMin);
  addParam("prefixCanvas", m_prefixCanvas, "Prefix to be added to canvas filename when saved as pdf.", std::string("c"));
  addParam("printCanvas", m_printCanvas, "If true, prints pdf of the analysis canvas.", bool(false));
}


DQMHistAnalysisEventT0Module::~DQMHistAnalysisEventT0Module() { }

void DQMHistAnalysisEventT0Module::initialize()
{
  gROOT->cd();

  //ECLTRG canvas
  m_cTOPTimeForECLTRG = new TCanvas("EventT0/c_TOPTimeECLTRGjitter", "ECLTRG jitter based on TOP time", 1200, 400);
  m_topPad1ECLTRG = new TPad("topPad1ECLTRG", "TOP pad1 ECLTRG", 0.03, 0.02, 0.33, 0.98);
  m_topPad2ECLTRG = new TPad("topPad2ECLTRG", "TOP pad2 ECLTRG", 0.35, 0.02, 0.65, 0.98);
  m_topPad3ECLTRG = new TPad("topPad3ECLTRG", "TOP pad3 ECLTRG", 0.67, 0.02, 0.97, 0.98);

  //CDCTRG canvases
  m_cTOPTimeForCDCTRG = new TCanvas("EventT0/c_TOPTimeCDCTRGjitter", "CDCTRG jitter based on TOP time", 1200, 400);
  m_topPad1CDCTRG = new TPad("topPad1CDCTRG", "TOP pad1 CDCTRG", 0.03, 0.02, 0.33, 0.98);
  m_topPad2CDCTRG = new TPad("topPad2CDCTRG", "TOP pad2 CDCTRG", 0.35, 0.02, 0.65, 0.98);
  m_topPad3CDCTRG = new TPad("topPad3CDCTRG", "TOP pad3 CDCTRG", 0.67, 0.02, 0.97, 0.98);

  //SVD canvases
  //ECLTRG canvas
  m_cSVDTimeForECLTRG = new TCanvas("EventT0/c_SVDTimeECLTRGjitter", "ECLTRG jitter based on SVD time", 1200, 400);
  m_svdPad1ECLTRG = new TPad("svdPad1ECLTRG", "SVD pad1 ECLTRG", 0.03, 0.02, 0.33, 0.98);
  m_svdPad2ECLTRG = new TPad("svdPad2ECLTRG", "SVD pad2 ECLTRG", 0.35, 0.02, 0.65, 0.98);
  m_svdPad3ECLTRG = new TPad("svdPad3ECLTRG", "SVD pad3 ECLTRG", 0.67, 0.02, 0.97, 0.98);

  // CDC TRG canvas
  m_cSVDTimeForCDCTRG = new TCanvas("EventT0/c_SVDTimeCDCTRGjitter", "CDCTRG jitter based on SVD time", 1200, 400);
  m_svdPad1CDCTRG = new TPad("svdPad1CDCTRG", "SVD pad1 CDCTRG", 0.03, 0.02, 0.33, 0.98);
  m_svdPad2CDCTRG = new TPad("svdPad2CDCTRG", "SVD pad2 CDCTRG", 0.35, 0.02, 0.65, 0.98);
  m_svdPad3CDCTRG = new TPad("svdPad3CDCTRG", "SVD pad3 CDCTRG", 0.67, 0.02, 0.97, 0.98);

  m_eAlgorithmSourceFractionsHadronL1ECLTRG =
    new TEfficiency("effAlgorithmSourceFractionsHadronL1ECLTRG",
                    "Fraction of events with EventT0 from each algorithm for hadronic events triggerd by ECL;Algorithm;Fraction #epsilon",
                    5, 0, 5);
  m_eAlgorithmSourceFractionsHadronL1CDCTRG =
    new TEfficiency("effAlgorithmSourceFractionsHadronL1CDCTRG",
                    "Fraction of events with EventT0 from each algorithm for hadronic events triggerd by CDC;Algorithm;Fraction #epsilon",
                    5, 0, 5);
  m_eAlgorithmSourceFractionsHadronL1TOPTRG =
    new TEfficiency("effAlgorithmSourceFractionsHadronL1TOPTRG",
                    "Fraction of events with EventT0 from each algorithm for hadronic events triggerd by TOP;Algorithm;Fraction #epsilon",
                    5, 0, 5);
  m_eAlgorithmSourceFractionsBhaBhaL1ECLTRG =
    new TEfficiency("effAlgorithmSourceFractionsBhaBhaL1ECLTRG",
                    "Fraction of events with EventT0 from each algorithm for Bhabha events triggerd by ECL;Algorithm;Fraction #epsilon",
                    5, 0, 5);
  m_eAlgorithmSourceFractionsBhaBhaL1CDCTRG =
    new TEfficiency("effAlgorithmSourceFractionsBhaBhaL1ECLTRG",
                    "Fraction of events with EventT0 from each algorithm for Bhabha events triggerd by ECL;Algorithm;Fraction #epsilon",
                    5, 0, 5);
  m_eAlgorithmSourceFractionsBhaBhaL1TOPTRG =
    new TEfficiency("effAlgorithmSourceFractionsBhaBhaL1TOPTRG",
                    "Fraction of events with EventT0 from each algorithm for Bhabha events triggerd by TOP;Algorithm;Fraction #epsilon",
                    5, 0, 5);
  m_eAlgorithmSourceFractionsMuMuL1ECLTRG =
    new TEfficiency("effAlgorithmSourceFractionsMuMuL1ECLTRG",
                    "Fraction of events with EventT0 from each algorithm for #mu#mu events triggerd by ECL;Algorithm;Fraction #epsilon",
                    5, 0, 5);
  m_eAlgorithmSourceFractionsMuMuL1CDCTRG =
    new TEfficiency("effAlgorithmSourceFractionsMuMuL1CDCTRG",
                    "Fraction of events with EventT0 from each algorithm for #mu#mu events triggerd by CDC;Algorithm;Fraction #epsilon",
                    5, 0, 5);
  m_eAlgorithmSourceFractionsMuMuL1TOPTRG =
    new TEfficiency("effAlgorithmSourceFractionsMuMuL1TOPTRG",
                    "Fraction of events with EventT0 from each algorithm for #mu#mu events triggerd by TOP;Algorithm;Fraction #epsilon",
                    5, 0, 5);

  m_monObj = getMonitoringObject("eventT0");
  m_monObj->addCanvas(m_cTOPTimeForECLTRG);
  m_monObj->addCanvas(m_cTOPTimeForCDCTRG);
  m_monObj->addCanvas(m_cSVDTimeForECLTRG);
  m_monObj->addCanvas(m_cSVDTimeForCDCTRG);
}


void DQMHistAnalysisEventT0Module::beginRun()
{
  m_cTOPTimeForECLTRG->Clear();
  m_cTOPTimeForCDCTRG->Clear();
  m_cSVDTimeForECLTRG->Clear();
  m_cSVDTimeForCDCTRG->Clear();
}

void DQMHistAnalysisEventT0Module::event()
{
  FillEfficiencyHistogram("AlgorithmSourceFractionsHadronL1ECLTRG", m_eAlgorithmSourceFractionsHadronL1ECLTRG);
  FillEfficiencyHistogram("AlgorithmSourceFractionsHadronL1CDCTRG", m_eAlgorithmSourceFractionsHadronL1CDCTRG);
  FillEfficiencyHistogram("AlgorithmSourceFractionsHadronL1TOPTRG", m_eAlgorithmSourceFractionsHadronL1TOPTRG);
  FillEfficiencyHistogram("AlgorithmSourceFractionsBhaBhaL1ECLTRG", m_eAlgorithmSourceFractionsBhaBhaL1ECLTRG);
  FillEfficiencyHistogram("AlgorithmSourceFractionsBhaBhaL1CDCTRG", m_eAlgorithmSourceFractionsBhaBhaL1CDCTRG);
  FillEfficiencyHistogram("AlgorithmSourceFractionsBhaBhaL1TOPTRG", m_eAlgorithmSourceFractionsBhaBhaL1TOPTRG);
  FillEfficiencyHistogram("AlgorithmSourceFractionsMuMuL1ECLTRG",   m_eAlgorithmSourceFractionsMuMuL1ECLTRG);
  FillEfficiencyHistogram("AlgorithmSourceFractionsMuMuL1CDCTRG",   m_eAlgorithmSourceFractionsMuMuL1CDCTRG);
  FillEfficiencyHistogram("AlgorithmSourceFractionsMuMuL1TOPTRG",   m_eAlgorithmSourceFractionsMuMuL1TOPTRG);
}

void DQMHistAnalysisEventT0Module::endRun()
{
  // --- TOP EventT0 plots for ECLTRG ---

  // find TOP EventT0 Hadrons ECLTRG histogram and process it
  TH1* h = findHist("EventT0/m_histEventT0_TOP_hadron_L1_ECLTRG");
  TString tag = "hadronECLTRG";
  m_topPad1ECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_topPad1ECLTRG->SetFillColor(0);
    m_topPad1ECLTRG->Modified();
    m_topPad1ECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_topPad1ECLTRG->SetFillColor(kGray);
  }

  // find TOP EventT0 Bhabhas ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_bhabha_L1_ECLTRG");
  tag = "bhabhaECLTRG";
  m_topPad2ECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_topPad2ECLTRG->SetFillColor(0);
    m_topPad2ECLTRG->Modified();
    m_topPad2ECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_topPad2ECLTRG->SetFillColor(kGray);
  }

  // find TOP EventT0 Mumus ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_mumu_L1_ECLTRG");
  tag = "mumuECLTRG";
  m_topPad3ECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_topPad3ECLTRG->SetFillColor(0);
    m_topPad3ECLTRG->Modified();
    m_topPad3ECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_topPad3ECLTRG->SetFillColor(kGray);
  }

  m_cTOPTimeForECLTRG->cd();
  m_topPad1ECLTRG->Draw();
  m_topPad2ECLTRG->Draw();
  m_topPad3ECLTRG->Draw();

  if (m_printCanvas)
    m_cTOPTimeForECLTRG->Print(Form("%s_ECLTRG.pdf", m_prefixCanvas.c_str()));


  // --- TOP EventT0 plots for CDCTRG ---

  // find TOP EventT0 Hadrons CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_hadron_L1_CDCTRG");
  tag = "hadronCDCTRG";
  m_topPad1CDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_topPad1CDCTRG->SetFillColor(0);
    m_topPad1CDCTRG->Modified();
    m_topPad1CDCTRG->Update();
    m_cTOPTimeForCDCTRG->cd();
    m_topPad1CDCTRG->Draw();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_topPad1CDCTRG->SetFillColor(kGray);
    m_cTOPTimeForCDCTRG->cd();
    m_topPad1CDCTRG->Draw();
  }

  // find TOP EventT0 Bhabhas CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_bhabha_L1_CDCTRG");
  tag = "bhabhaCDCTRG";
  m_topPad2CDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_topPad2CDCTRG->SetFillColor(0);
    m_topPad2CDCTRG->Modified();
    m_topPad2CDCTRG->Update();
    m_cTOPTimeForCDCTRG->cd();
    m_topPad2CDCTRG->Draw();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_topPad2CDCTRG->SetFillColor(kGray);
    m_cTOPTimeForCDCTRG->cd();
    m_topPad2CDCTRG->Draw();
  }

  // find TOP EventT0 Mumus CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_mumu_L1_CDCTRG");
  tag = "mumuCDCTRG";
  m_topPad3CDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_topPad3CDCTRG->SetFillColor(0);
    m_topPad3CDCTRG->Modified();
    m_topPad3CDCTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_topPad3CDCTRG->SetFillColor(kGray);
  }

  m_cTOPTimeForCDCTRG->cd();
  m_topPad1CDCTRG->Draw();
  m_topPad2CDCTRG->Draw();
  m_topPad3CDCTRG->Draw();

  if (m_printCanvas)
    m_cTOPTimeForCDCTRG->Print(Form("%s_CDCTRG.pdf", m_prefixCanvas.c_str()));



  // --- SVD EventT0 plots for ECLTRG ---

  // find SVD EventT0 Hadrons ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_hadron_L1_ECLTRG");
  tag = "hadronECLTRG";
  m_svdPad1ECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_svdPad1ECLTRG->SetFillColor(0);
    m_svdPad1ECLTRG->Modified();
    m_svdPad1ECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_svdPad1ECLTRG->SetFillColor(kGray);
  }

  // find SVD EventT0 Bhabhas ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_bhabha_L1_ECLTRG");
  tag = "bhabhaECLTRG";
  m_svdPad2ECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_svdPad2ECLTRG->SetFillColor(0);
    m_svdPad2ECLTRG->Modified();
    m_svdPad2ECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_svdPad2ECLTRG->SetFillColor(kGray);
  }

  // find SVD EventT0 Mumus ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_mumu_L1_ECLTRG");
  tag = "mumuECLTRG";
  m_svdPad3ECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_svdPad3ECLTRG->SetFillColor(0);
    m_svdPad3ECLTRG->Modified();
    m_svdPad3ECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_svdPad3ECLTRG->SetFillColor(kGray);
  }

  m_cSVDTimeForECLTRG->cd();
  m_svdPad1ECLTRG->Draw();
  m_svdPad2ECLTRG->Draw();
  m_svdPad3ECLTRG->Draw();

  if (m_printCanvas)
    m_cSVDTimeForECLTRG->Print(Form("%s_SVDECLTRG.pdf", m_prefixCanvas.c_str()));


  // --- SVD EventT0 plots for CDCTRG ---

  // find SVD EventT0 Hadrons CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_hadron_L1_CDCTRG");
  tag = "hadronCDCTRG";
  m_svdPad1CDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_svdPad1CDCTRG->SetFillColor(0);
    m_svdPad1CDCTRG->Modified();
    m_svdPad1CDCTRG->Update();
    m_cSVDTimeForCDCTRG->cd();
    m_svdPad1CDCTRG->Draw();
  } else {
    B2WARNING(Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_svdPad1CDCTRG->SetFillColor(kGray);
    m_cSVDTimeForCDCTRG->cd();
    m_svdPad1CDCTRG->Draw();
  }

  // find SVD EventT0 Bhabhas CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_bhabha_L1_CDCTRG");
  tag = "bhabhaCDCTRG";
  m_svdPad2CDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_svdPad2CDCTRG->SetFillColor(0);
    m_svdPad2CDCTRG->Modified();
    m_svdPad2CDCTRG->Update();
    m_cSVDTimeForCDCTRG->cd();
    m_svdPad2CDCTRG->Draw();
  } else {
    B2WARNING(Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_svdPad2CDCTRG->SetFillColor(kGray);
    m_cSVDTimeForCDCTRG->cd();
    m_svdPad2CDCTRG->Draw();
  }


  // find SVD EventT0 Mumus CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_mumu_L1_CDCTRG");
  tag = "mumuCDCTRG";
  m_svdPad3CDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_svdPad3CDCTRG->SetFillColor(0);
    m_svdPad3CDCTRG->Modified();
    m_svdPad3CDCTRG->Update();
  } else {
    B2WARNING(Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_svdPad3CDCTRG->SetFillColor(kGray);
  }

  m_cSVDTimeForCDCTRG->cd();
  m_svdPad1CDCTRG->Draw();
  m_svdPad2CDCTRG->Draw();
  m_svdPad3CDCTRG->Draw();

  if (m_printCanvas)
    m_cSVDTimeForCDCTRG->Print(Form("%s_SVDCDCTRG.pdf", m_prefixCanvas.c_str()));

}

void DQMHistAnalysisEventT0Module::terminate()
{

  delete m_cTOPTimeForECLTRG;
  delete m_cTOPTimeForCDCTRG;
  delete m_cSVDTimeForECLTRG;
  delete m_cSVDTimeForCDCTRG;

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

  // The default value for the EventT0 value is -1000, but bins start at -100, so we might mostly fill the underflow bin if
  // EventT0 for a detector is not present. And also the nominal EventT0 might be too big or too small. Only use the content
  // of the actually useful bins to decide whether or not to fit the histogram.
  auto nValidEntries = h->GetEntries() - h->GetBinContent(0) - h->GetBinContent(h->GetNbinsX() + 1);
  if (static_cast<uint>(nValidEntries) < m_nEntriesMin) {
    B2DEBUG(20, "not enough entries");
    m_monObj->setVariable(Form("fit_%s", tag.Data()), 0);
    return false;
  }


  //scale the histogram only with content of valid bins, ignore over and underflow bins
  h->Scale(1. / nValidEntries);
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

  gauss2.SetLineColor(kRed);
  gauss2.SetLineStyle(kDashed);
  gauss2.SetParameters(par[0] * (1 - par[1]), par[4], par[5]);

  m_monObj->setVariable(Form("fit_%s", tag.Data()), 1);
  m_monObj->setVariable(Form("N_%s", tag.Data()), nValidEntries, TMath::Sqrt(nValidEntries));
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

void DQMHistAnalysisEventT0Module::FillEfficiencyHistogram(const std::string& histname, TEfficiency* eff)
{
  TH1* h = findHist("EventT0/" + histname);
  if (not h) {
    return;
  }

  // Admittedly quite a hacky way to obtain the normalisation values: Create a new histogram and fill each of the bins with
  // the bin content of the -1 bin of h which is used for bin counting, and at the same time set the corresponding bin label.
  const auto totalEntries = h->GetBinContent(-1);
  TH1D* totalHist = new TH1D("total", "total;Algorithm;Fraction #epsilon", 5, 0, 5);
  for (uint i = 0; i < 5; i++) {
    totalHist->SetBinContent(i + 1, totalEntries);
    totalHist->GetXaxis()->SetBinLabel(i + 1, c_eventT0Algorithms[i]);
  }
  eff->SetPassedHistogram(*h, "f");
  eff->SetTotalHistogram(*totalHist, "f");

  eff->Paint("AP");

  TGraphAsymmErrors* graph = eff->GetPaintedGraph();
  if (not graph) {
    return;
  }

  auto ax = graph->GetXaxis();
  if (not ax) {
    return;
  }
  // Print x-axis bin labels horizontally
  ax->LabelsOption("h");

  graph->Draw("AP");
}

