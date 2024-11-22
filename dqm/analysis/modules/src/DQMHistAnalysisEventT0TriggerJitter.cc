/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisEventT0TriggerJitter.cc
// Description : module for trigger jitter/EventT0 DQM histogram analysis
//-


#include <dqm/analysis/modules/DQMHistAnalysisEventT0TriggerJitter.h>

#include <TROOT.h>
#include <TStyle.h>
#include <TF1.h>
#include <TMath.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisEventT0TriggerJitter);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisEventT0TriggerJitterModule::DQMHistAnalysisEventT0TriggerJitterModule()
  : DQMHistAnalysisModule()
{
  setDescription("Determining and processing EventT0s from different subdetectors (ECL, CDC, TOP, SVD) for different L1 trigger sources (ECL, CDC, and TOP) to estimate trigger jitter information for different HLT event types (hadron, BhaBha, µµ).");

  //Parameter definition
  addParam("min_nEntries", m_nEntriesMin, "Minimum number of entries to process the histogram.", m_nEntriesMin);
  addParam("prefixCanvas", m_prefixCanvas, "Prefix to be added to canvas filename when saved as pdf.", std::string("c"));
  addParam("printCanvas", m_printCanvas, "If true, prints pdf of the analysis canvas.", bool(false));
}


DQMHistAnalysisEventT0TriggerJitterModule::~DQMHistAnalysisEventT0TriggerJitterModule() { }

void DQMHistAnalysisEventT0TriggerJitterModule::initialize()
{
  gROOT->cd();

  initializeCanvases();

  m_monObj = getMonitoringObject("eventT0");
}


void DQMHistAnalysisEventT0TriggerJitterModule::beginRun()
{
  clearCanvases();
}

void DQMHistAnalysisEventT0TriggerJitterModule::event()
{
  analyseECLTRGEventT0Distributions();
  analyseCDCTRGEventT0Distributions();
  analyseTOPTRGEventT0Distributions();

  if (m_printCanvas) {
    printCanvases();
  }
}

void DQMHistAnalysisEventT0TriggerJitterModule::terminate()
{
  deleteCanvases();
}

double DQMHistAnalysisEventT0TriggerJitterModule::fDoubleGaus(double* x, double* par)
{
  double N = par[0];
  double frac = par[1];
  double mean = par[2];
  double sigma = par[3];
  double mean2 = par[4];
  double sigma2 = par[5];

  return N * frac * TMath::Gaus(x[0], mean, sigma) + N * (1 - frac) * TMath::Gaus(x[0], mean2, sigma2);
}

bool DQMHistAnalysisEventT0TriggerJitterModule::processHistogram(TH1* h,  TString tag)
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
  TF1 fitf("fit", DQMHistAnalysisEventT0TriggerJitterModule::fDoubleGaus, -50, 50, 6);
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

void DQMHistAnalysisEventT0TriggerJitterModule::analyseECLTRGEventT0Distributions()
{
  // --- ECL EventT0 plots for ECLTRG ---

  // find ECL EventT0 Hadrons ECLTRG histogram and process it
  TH1* h = findHist("EventT0/m_histEventT0_ECL_hadron_L1_ECLTRG");
  TString tag = "hadronECLTRG_ECLT0";
  m_cECLTimeHadronsECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cECLTimeHadronsECLTRG->SetFillColor(0);
    m_cECLTimeHadronsECLTRG->Modified();
    m_cECLTimeHadronsECLTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram ECL EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cECLTimeHadronsECLTRG->SetFillColor(kGray);
    m_cECLTimeHadronsECLTRG->Draw();
  }

  // find ECL EventT0 Bhabhas ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_ECL_bhabha_L1_ECLTRG");
  tag = "bhabhaECLTRG_ECLT0";
  m_cECLTimeBhaBhaECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cECLTimeBhaBhaECLTRG->SetFillColor(0);
    m_cECLTimeBhaBhaECLTRG->Modified();
    m_cECLTimeBhaBhaECLTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram ECL EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cECLTimeBhaBhaECLTRG->SetFillColor(kGray);
    m_cECLTimeBhaBhaECLTRG->Draw();
  }

  // find ECL EventT0 Mumus ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_ECL_mumu_L1_ECLTRG");
  tag = "mumuECLTRG_ECLT0";
  m_cECLTimeMuMuECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cECLTimeMuMuECLTRG->SetFillColor(0);
    m_cECLTimeMuMuECLTRG->Modified();
    m_cECLTimeMuMuECLTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram ECL EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cECLTimeMuMuECLTRG->SetFillColor(kGray);
    m_cECLTimeMuMuECLTRG->Draw();
  }


  // --- CDC EventT0 plots for ECLTRG ---

  // find CDC EventT0 Hadrons ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_CDC_hadron_L1_ECLTRG");
  tag = "hadronECLTRG_CDCT0";
  m_cCDCTimeHadronsECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cCDCTimeHadronsECLTRG->SetFillColor(0);
    m_cCDCTimeHadronsECLTRG->Modified();
    m_cCDCTimeHadronsECLTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram CDC EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cCDCTimeHadronsECLTRG->SetFillColor(kGray);
    m_cCDCTimeHadronsECLTRG->Draw();
  }

  // find CDC EventT0 Bhabhas ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_CDC_bhabha_L1_ECLTRG");
  tag = "bhabhaECLTRG_CDCT0";
  m_cCDCTimeBhaBhaECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cCDCTimeBhaBhaECLTRG->SetFillColor(0);
    m_cCDCTimeBhaBhaECLTRG->Modified();
    m_cCDCTimeBhaBhaECLTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram CDC EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cCDCTimeBhaBhaECLTRG->SetFillColor(kGray);
    m_cCDCTimeBhaBhaECLTRG->Draw();
  }

  // find CDC EventT0 Mumus ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_CDC_mumu_L1_ECLTRG");
  tag = "mumuECLTRG_CDCT0";
  m_cCDCTimeMuMuECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cCDCTimeMuMuECLTRG->SetFillColor(0);
    m_cCDCTimeMuMuECLTRG->Modified();
    m_cCDCTimeMuMuECLTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram CDC EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cCDCTimeMuMuECLTRG->SetFillColor(kGray);
    m_cCDCTimeMuMuECLTRG->Draw();
  }


  // --- TOP EventT0 plots for ECLTRG ---

  // find TOP EventT0 Hadrons ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_hadron_L1_ECLTRG");
  tag = "hadronECLTRG_TOPT0";
  m_cTOPTimeHadronsECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cTOPTimeHadronsECLTRG->SetFillColor(0);
    m_cTOPTimeHadronsECLTRG->Modified();
    m_cTOPTimeHadronsECLTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cTOPTimeHadronsECLTRG->SetFillColor(kGray);
    m_cTOPTimeHadronsECLTRG->Draw();
  }

  // find TOP EventT0 Bhabhas ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_bhabha_L1_ECLTRG");
  tag = "bhabhaECLTRG_TOPT0";
  m_cTOPTimeBhaBhaECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cTOPTimeBhaBhaECLTRG->SetFillColor(0);
    m_cTOPTimeBhaBhaECLTRG->Modified();
    m_cTOPTimeBhaBhaECLTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cTOPTimeBhaBhaECLTRG->SetFillColor(kGray);
    m_cTOPTimeBhaBhaECLTRG->Draw();
  }

  // find TOP EventT0 Mumus ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_mumu_L1_ECLTRG");
  tag = "mumuECLTRG_TOPT0";
  m_cTOPTimeMuMuECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cTOPTimeMuMuECLTRG->SetFillColor(0);
    m_cTOPTimeMuMuECLTRG->Modified();
    m_cTOPTimeMuMuECLTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cTOPTimeMuMuECLTRG->SetFillColor(kGray);
    m_cTOPTimeMuMuECLTRG->Draw();
  }


  // --- SVD EventT0 plots for ECLTRG ---

  // find SVD EventT0 Hadrons ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_hadron_L1_ECLTRG");
  tag = "hadronECLTRG_SVDT0";
  m_cSVDTimeHadronsECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cSVDTimeHadronsECLTRG->SetFillColor(0);
    m_cSVDTimeHadronsECLTRG->Modified();
    m_cSVDTimeHadronsECLTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cSVDTimeHadronsECLTRG->SetFillColor(kGray);
    m_cSVDTimeHadronsECLTRG->Draw();
  }

  // find SVD EventT0 Bhabhas ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_bhabha_L1_ECLTRG");
  tag = "bhabhaECLTRG_SVDT0";
  m_cSVDTimeBhaBhaECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cSVDTimeBhaBhaECLTRG->SetFillColor(0);
    m_cSVDTimeBhaBhaECLTRG->Modified();
    m_cSVDTimeBhaBhaECLTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cSVDTimeBhaBhaECLTRG->SetFillColor(kGray);
    m_cSVDTimeBhaBhaECLTRG->Draw();
  }

  // find SVD EventT0 Mumus ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_mumu_L1_ECLTRG");
  tag = "mumuECLTRG_SVDT0";
  m_cSVDTimeMuMuECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cSVDTimeMuMuECLTRG->SetFillColor(0);
    m_cSVDTimeMuMuECLTRG->Modified();
    m_cSVDTimeMuMuECLTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cSVDTimeMuMuECLTRG->SetFillColor(kGray);
    m_cSVDTimeMuMuECLTRG->Draw();
  }


}

void DQMHistAnalysisEventT0TriggerJitterModule::analyseCDCTRGEventT0Distributions()
{
  // --- ECL EventT0 plots for CDCTRG ---

  // find ECL EventT0 Hadrons CDCTRG histogram and process it
  TH1* h = findHist("EventT0/m_histEventT0_ECL_hadron_L1_CDCTRG");
  TString tag = "hadronCDCTRG_ECLT0";
  m_cECLTimeHadronsCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cECLTimeHadronsCDCTRG->SetFillColor(0);
    m_cECLTimeHadronsCDCTRG->Modified();
    m_cECLTimeHadronsCDCTRG->Update();
    m_cECLTimeHadronsCDCTRG->Draw();
  } else {
    B2DEBUG(29, Form("Histogram ECL EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cECLTimeHadronsCDCTRG->SetFillColor(kGray);
    m_cECLTimeHadronsCDCTRG->Draw();
  }

  // find ECL EventT0 Bhabhas CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_ECL_bhabha_L1_CDCTRG");
  tag = "bhabhaCDCTRG_ECLT0";
  m_cECLTimeBhaBhaCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cECLTimeBhaBhaCDCTRG->SetFillColor(0);
    m_cECLTimeBhaBhaCDCTRG->Modified();
    m_cECLTimeBhaBhaCDCTRG->Update();
    m_cECLTimeBhaBhaCDCTRG->Draw();
  } else {
    B2DEBUG(29, Form("Histogram ECL EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cECLTimeBhaBhaCDCTRG->SetFillColor(kGray);
    m_cECLTimeBhaBhaCDCTRG->Draw();
  }

  // find ECL EventT0 Mumus CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_ECL_mumu_L1_CDCTRG");
  tag = "mumuCDCTRG_ECLT0";
  m_cECLTimeMuMuCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cECLTimeMuMuCDCTRG->SetFillColor(0);
    m_cECLTimeMuMuCDCTRG->Modified();
    m_cECLTimeMuMuCDCTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram ECL EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cECLTimeMuMuCDCTRG->SetFillColor(kGray);
    m_cECLTimeMuMuCDCTRG->Draw();
  }


  // --- CDC EventT0 plots for CDCTRG ---

  // find CDC EventT0 Hadrons CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_CDC_hadron_L1_CDCTRG");
  tag = "hadronCDCTRG_CDCT0";
  m_cCDCTimeHadronsCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cCDCTimeHadronsCDCTRG->SetFillColor(0);
    m_cCDCTimeHadronsCDCTRG->Modified();
    m_cCDCTimeHadronsCDCTRG->Update();
    m_cCDCTimeHadronsCDCTRG->Draw();
  } else {
    B2DEBUG(29, Form("Histogram CDC EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cCDCTimeHadronsCDCTRG->SetFillColor(kGray);
    m_cCDCTimeHadronsCDCTRG->Draw();
  }

  // find CDC EventT0 Bhabhas CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_CDC_bhabha_L1_CDCTRG");
  tag = "bhabhaCDCTRG_CDCT0";
  m_cCDCTimeBhaBhaCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cCDCTimeBhaBhaCDCTRG->SetFillColor(0);
    m_cCDCTimeBhaBhaCDCTRG->Modified();
    m_cCDCTimeBhaBhaCDCTRG->Update();
    m_cCDCTimeBhaBhaCDCTRG->Draw();
  } else {
    B2DEBUG(29, Form("Histogram CDC EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cCDCTimeBhaBhaCDCTRG->SetFillColor(kGray);
    m_cCDCTimeBhaBhaCDCTRG->Draw();
  }

  // find CDC EventT0 Mumus CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_CDC_mumu_L1_CDCTRG");
  tag = "mumuCDCTRG_CDCT0";
  m_cCDCTimeMuMuCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cCDCTimeMuMuCDCTRG->SetFillColor(0);
    m_cCDCTimeMuMuCDCTRG->Modified();
    m_cCDCTimeMuMuCDCTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram CDC EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cCDCTimeMuMuCDCTRG->SetFillColor(kGray);
    m_cCDCTimeMuMuCDCTRG->Draw();
  }


  // --- TOP EventT0 plots for CDCTRG ---

  // find TOP EventT0 Hadrons CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_hadron_L1_CDCTRG");
  tag = "hadronCDCTRG_TOPT0";
  m_cTOPTimeHadronsCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cTOPTimeHadronsCDCTRG->SetFillColor(0);
    m_cTOPTimeHadronsCDCTRG->Modified();
    m_cTOPTimeHadronsCDCTRG->Update();
    m_cTOPTimeHadronsCDCTRG->Draw();
  } else {
    B2DEBUG(29, Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cTOPTimeHadronsCDCTRG->SetFillColor(kGray);
    m_cTOPTimeHadronsCDCTRG->Draw();
  }

  // find TOP EventT0 Bhabhas CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_bhabha_L1_CDCTRG");
  tag = "bhabhaCDCTRG_TOPT0";
  m_cTOPTimeBhaBhaCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cTOPTimeBhaBhaCDCTRG->SetFillColor(0);
    m_cTOPTimeBhaBhaCDCTRG->Modified();
    m_cTOPTimeBhaBhaCDCTRG->Update();
    m_cTOPTimeBhaBhaCDCTRG->Draw();
  } else {
    B2DEBUG(29, Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cTOPTimeBhaBhaCDCTRG->SetFillColor(kGray);
    m_cTOPTimeBhaBhaCDCTRG->Draw();
  }

  // find TOP EventT0 Mumus CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_mumu_L1_CDCTRG");
  tag = "mumuCDCTRG_TOPT0";
  m_cTOPTimeMuMuCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cTOPTimeMuMuCDCTRG->SetFillColor(0);
    m_cTOPTimeMuMuCDCTRG->Modified();
    m_cTOPTimeMuMuCDCTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cTOPTimeMuMuCDCTRG->SetFillColor(kGray);
    m_cTOPTimeMuMuCDCTRG->Draw();
  }


  // --- SVD EventT0 plots for CDCTRG ---

  // find SVD EventT0 Hadrons CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_hadron_L1_CDCTRG");
  tag = "hadronCDCTRG_SVDT0";
  m_cSVDTimeHadronsCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cSVDTimeHadronsCDCTRG->SetFillColor(0);
    m_cSVDTimeHadronsCDCTRG->Modified();
    m_cSVDTimeHadronsCDCTRG->Update();
    m_cSVDTimeHadronsCDCTRG->Draw();
  } else {
    B2DEBUG(29, Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cSVDTimeHadronsCDCTRG->SetFillColor(kGray);
    m_cSVDTimeHadronsCDCTRG->Draw();
  }

  // find SVD EventT0 Bhabhas CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_bhabha_L1_CDCTRG");
  tag = "bhabhaCDCTRG_SVDT0";
  m_cSVDTimeBhaBhaCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cSVDTimeBhaBhaCDCTRG->SetFillColor(0);
    m_cSVDTimeBhaBhaCDCTRG->Modified();
    m_cSVDTimeBhaBhaCDCTRG->Update();
    m_cSVDTimeBhaBhaCDCTRG->Draw();
  } else {
    B2DEBUG(29, Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cSVDTimeBhaBhaCDCTRG->SetFillColor(kGray);
    m_cSVDTimeBhaBhaCDCTRG->Draw();
  }

  // find SVD EventT0 Mumus CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_mumu_L1_CDCTRG");
  tag = "mumuCDCTRG_SVDT0";
  m_cSVDTimeMuMuCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cSVDTimeMuMuCDCTRG->SetFillColor(0);
    m_cSVDTimeMuMuCDCTRG->Modified();
    m_cSVDTimeMuMuCDCTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cSVDTimeMuMuCDCTRG->SetFillColor(kGray);
    m_cSVDTimeMuMuCDCTRG->Draw();
  }
}

void DQMHistAnalysisEventT0TriggerJitterModule::analyseTOPTRGEventT0Distributions()
{
  // --- ECL EventT0 plots for TOPTRG ---

  // find ECL EventT0 Hadrons TOPTRG histogram and process it
  TH1* h = findHist("EventT0/m_histEventT0_ECL_hadron_L1_TOPTRG");
  TString tag = "hadronTOPTRG_ECLT0";
  m_cECLTimeHadronsTOPTRG->cd();
  if (processHistogram(h, tag)) {
    m_cECLTimeHadronsTOPTRG->SetFillColor(0);
    m_cECLTimeHadronsTOPTRG->Modified();
    m_cECLTimeHadronsTOPTRG->Update();
    m_cECLTimeHadronsTOPTRG->Draw();
  } else {
    B2DEBUG(29, Form("Histogram ECL EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cECLTimeHadronsTOPTRG->SetFillColor(kGray);
    m_cECLTimeHadronsTOPTRG->Draw();
  }

  // find ECL EventT0 Bhabhas TOPTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_ECL_bhabha_L1_TOPTRG");
  tag = "bhabhaTOPTRG_ECLT0";
  m_cECLTimeBhaBhaTOPTRG->cd();
  if (processHistogram(h, tag)) {
    m_cECLTimeBhaBhaTOPTRG->SetFillColor(0);
    m_cECLTimeBhaBhaTOPTRG->Modified();
    m_cECLTimeBhaBhaTOPTRG->Update();
    m_cECLTimeBhaBhaTOPTRG->Draw();
  } else {
    B2DEBUG(29, Form("Histogram ECL EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cECLTimeBhaBhaTOPTRG->SetFillColor(kGray);
    m_cECLTimeBhaBhaTOPTRG->Draw();
  }

  // find ECL EventT0 Mumus TOPTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_ECL_mumu_L1_TOPTRG");
  tag = "mumuTOPTRG_ECLT0";
  m_cECLTimeMuMuTOPTRG->cd();
  if (processHistogram(h, tag)) {
    m_cECLTimeMuMuTOPTRG->SetFillColor(0);
    m_cECLTimeMuMuTOPTRG->Modified();
    m_cECLTimeMuMuTOPTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram ECL EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cECLTimeMuMuTOPTRG->SetFillColor(kGray);
    m_cECLTimeMuMuTOPTRG->Draw();
  }


  // --- CDC EventT0 plots for TOPTRG ---

  // find CDC EventT0 Hadrons TOPTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_CDC_hadron_L1_TOPTRG");
  tag = "hadronTOPTRG_CDCT0";
  m_cCDCTimeHadronsTOPTRG->cd();
  if (processHistogram(h, tag)) {
    m_cCDCTimeHadronsTOPTRG->SetFillColor(0);
    m_cCDCTimeHadronsTOPTRG->Modified();
    m_cCDCTimeHadronsTOPTRG->Update();
    m_cCDCTimeHadronsTOPTRG->Draw();
  } else {
    B2DEBUG(29, Form("Histogram CDC EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cCDCTimeHadronsTOPTRG->SetFillColor(kGray);
    m_cCDCTimeHadronsTOPTRG->Draw();
  }

  // find CDC EventT0 Bhabhas TOPTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_CDC_bhabha_L1_TOPTRG");
  tag = "bhabhaTOPTRG_CDCT0";
  m_cCDCTimeBhaBhaTOPTRG->cd();
  if (processHistogram(h, tag)) {
    m_cCDCTimeBhaBhaTOPTRG->SetFillColor(0);
    m_cCDCTimeBhaBhaTOPTRG->Modified();
    m_cCDCTimeBhaBhaTOPTRG->Update();
    m_cCDCTimeBhaBhaTOPTRG->Draw();
  } else {
    B2DEBUG(29, Form("Histogram CDC EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cCDCTimeBhaBhaTOPTRG->SetFillColor(kGray);
    m_cCDCTimeBhaBhaTOPTRG->Draw();
  }

  // find CDC EventT0 Mumus TOPTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_CDC_mumu_L1_TOPTRG");
  tag = "mumuTOPTRG_CDCT0";
  m_cCDCTimeMuMuTOPTRG->cd();
  if (processHistogram(h, tag)) {
    m_cCDCTimeMuMuTOPTRG->SetFillColor(0);
    m_cCDCTimeMuMuTOPTRG->Modified();
    m_cCDCTimeMuMuTOPTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram CDC EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cCDCTimeMuMuTOPTRG->SetFillColor(kGray);
    m_cCDCTimeMuMuTOPTRG->Draw();
  }


  // --- TOP EventT0 plots for TOPTRG ---

  // find TOP EventT0 Hadrons TOPTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_hadron_L1_TOPTRG");
  tag = "hadronTOPTRG_TOPT0";
  m_cTOPTimeHadronsTOPTRG->cd();
  if (processHistogram(h, tag)) {
    m_cTOPTimeHadronsTOPTRG->SetFillColor(0);
    m_cTOPTimeHadronsTOPTRG->Modified();
    m_cTOPTimeHadronsTOPTRG->Update();
    m_cTOPTimeHadronsTOPTRG->Draw();
  } else {
    B2DEBUG(29, Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cTOPTimeHadronsTOPTRG->SetFillColor(kGray);
    m_cTOPTimeHadronsTOPTRG->Draw();
  }

  // find TOP EventT0 Bhabhas TOPTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_bhabha_L1_TOPTRG");
  tag = "bhabhaTOPTRG_TOPT0";
  m_cTOPTimeBhaBhaTOPTRG->cd();
  if (processHistogram(h, tag)) {
    m_cTOPTimeBhaBhaTOPTRG->SetFillColor(0);
    m_cTOPTimeBhaBhaTOPTRG->Modified();
    m_cTOPTimeBhaBhaTOPTRG->Update();
    m_cTOPTimeBhaBhaTOPTRG->Draw();
  } else {
    B2DEBUG(29, Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cTOPTimeBhaBhaTOPTRG->SetFillColor(kGray);
    m_cTOPTimeBhaBhaTOPTRG->Draw();
  }

  // find TOP EventT0 Mumus TOPTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_mumu_L1_TOPTRG");
  tag = "mumuTOPTRG_TOPT0";
  m_cTOPTimeMuMuTOPTRG->cd();
  if (processHistogram(h, tag)) {
    m_cTOPTimeMuMuTOPTRG->SetFillColor(0);
    m_cTOPTimeMuMuTOPTRG->Modified();
    m_cTOPTimeMuMuTOPTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cTOPTimeMuMuTOPTRG->SetFillColor(kGray);
    m_cTOPTimeMuMuTOPTRG->Draw();
  }


  // --- SVD EventT0 plots for TOPTRG ---

  // find SVD EventT0 Hadrons TOPTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_hadron_L1_TOPTRG");
  tag = "hadronTOPTRG_SVDT0";
  m_cSVDTimeHadronsTOPTRG->cd();
  if (processHistogram(h, tag)) {
    m_cSVDTimeHadronsTOPTRG->SetFillColor(0);
    m_cSVDTimeHadronsTOPTRG->Modified();
    m_cSVDTimeHadronsTOPTRG->Update();
    m_cSVDTimeHadronsTOPTRG->Draw();
  } else {
    B2DEBUG(29, Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cSVDTimeHadronsTOPTRG->SetFillColor(kGray);
    m_cSVDTimeHadronsTOPTRG->Draw();
  }

  // find SVD EventT0 Bhabhas TOPTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_bhabha_L1_TOPTRG");
  tag = "bhabhaTOPTRG_SVDT0";
  m_cSVDTimeBhaBhaTOPTRG->cd();
  if (processHistogram(h, tag)) {
    m_cSVDTimeBhaBhaTOPTRG->SetFillColor(0);
    m_cSVDTimeBhaBhaTOPTRG->Modified();
    m_cSVDTimeBhaBhaTOPTRG->Update();
    m_cSVDTimeBhaBhaTOPTRG->Draw();
  } else {
    B2DEBUG(29, Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cSVDTimeBhaBhaTOPTRG->SetFillColor(kGray);
    m_cSVDTimeBhaBhaTOPTRG->Draw();
  }

  // find SVD EventT0 Mumus TOPTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_mumu_L1_TOPTRG");
  tag = "mumuTOPTRG_SVDT0";
  m_cSVDTimeMuMuTOPTRG->cd();
  if (processHistogram(h, tag)) {
    m_cSVDTimeMuMuTOPTRG->SetFillColor(0);
    m_cSVDTimeMuMuTOPTRG->Modified();
    m_cSVDTimeMuMuTOPTRG->Update();
  } else {
    B2DEBUG(29, Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cSVDTimeMuMuTOPTRG->SetFillColor(kGray);
    m_cSVDTimeMuMuTOPTRG->Draw();
  }
}

void DQMHistAnalysisEventT0TriggerJitterModule::initializeCanvases()
{
  //ECLTRG canvas
  m_cECLTimeHadronsECLTRG = new TCanvas("ECLTimeHadronsECLTRG", "ECL time hadrons ECLTRG jitter");
  m_cECLTimeBhaBhaECLTRG = new TCanvas("ECLTimeBhaBhaECLTRG", "ECL time BhaBha ECLTRG jitter");
  m_cECLTimeMuMuECLTRG = new TCanvas("ECLTimeMuMuECLTRG", "ECL time #mu#mu ECLTRG jitter");
  m_cCDCTimeHadronsECLTRG = new TCanvas("CDCTimeHadronsECLTRG", "CDC time hadrons ECLTRG jitter");
  m_cCDCTimeBhaBhaECLTRG = new TCanvas("CDCTimeBhaBhaECLTRG", "CDC time BhaBha ECLTRG jitter");
  m_cCDCTimeMuMuECLTRG = new TCanvas("CDCTimeMuMuECLTRG", "CDC time #mu#mu ECLTRG jitter");
  m_cTOPTimeHadronsECLTRG = new TCanvas("TOPTimeHadronsECLTRG", "TOP time hadrons ECLTRG jitter");
  m_cTOPTimeBhaBhaECLTRG = new TCanvas("TOPTimeBhaBhaECLTRG", "TOP time BhaBha ECLTRG jitter");
  m_cTOPTimeMuMuECLTRG = new TCanvas("TOPTimeMuMuECLTRG", "TOP time #mu#mu ECLTRG jitter");
  m_cSVDTimeHadronsECLTRG = new TCanvas("SVDTimeHadronsECLTRG", "SVD time hadrons ECLTRG jitter");
  m_cSVDTimeBhaBhaECLTRG = new TCanvas("SVDTimeBhaBhaECLTRG", "SVD time BhaBha ECLTRG jitter");
  m_cSVDTimeMuMuECLTRG = new TCanvas("SVDTimeMuMuECLTRG", "SVD time #mu#mu ECLTRG jitter");

  //CDCTRG canvas
  m_cECLTimeHadronsCDCTRG = new TCanvas("ECLTimeHadronsCDCTRG", "ECL time hadrons CDCTRG jitter");
  m_cECLTimeBhaBhaCDCTRG = new TCanvas("ECLTimeBhaBhaCDCTRG", "ECL time BhaBha CDCTRG jitter");
  m_cECLTimeMuMuCDCTRG = new TCanvas("ECLTimeMuMuCDCTRG", "ECL time #mu#mu CDCTRG jitter");
  m_cCDCTimeHadronsCDCTRG = new TCanvas("CDCTimeHadronsCDCTRG", "CDC time hadrons CDCTRG jitter");
  m_cCDCTimeBhaBhaCDCTRG = new TCanvas("CDCTimeBhaBhaCDCTRG", "CDC time BhaBha CDCTRG jitter");
  m_cCDCTimeMuMuCDCTRG = new TCanvas("CDCTimeMuMuCDCTRG", "CDC time #mu#mu CDCTRG jitter");
  m_cTOPTimeHadronsCDCTRG = new TCanvas("TOPTimeHadronsCDCTRG", "TOP time hadrons CDCTRG jitter");
  m_cTOPTimeBhaBhaCDCTRG = new TCanvas("TOPTimeBhaBhaCDCTRG", "TOP time BhaBha CDCTRG jitter");
  m_cTOPTimeMuMuCDCTRG = new TCanvas("TOPTimeMuMuCDCTRG", "TOP time #mu#mu CDCTRG jitter");
  m_cSVDTimeHadronsCDCTRG = new TCanvas("SVDTimeHadronsCDCTRG", "SVD time hadrons CDCTRG jitter");
  m_cSVDTimeBhaBhaCDCTRG = new TCanvas("SVDTimeBhaBhaCDCTRG", "SVD time BhaBha CDCTRG jitter");
  m_cSVDTimeMuMuCDCTRG = new TCanvas("SVDTimeMuMuCDCTRG", "SVD time #mu#mu CDCTRG jitter");

  //TOPTRG canvas
  m_cECLTimeHadronsTOPTRG = new TCanvas("ECLTimeHadronsTOPTRG", "ECL time hadrons TOPTRG jitter");
  m_cECLTimeBhaBhaTOPTRG = new TCanvas("ECLTimeBhaBhaTOPTRG", "ECL time BhaBha TOPTRG jitter");
  m_cECLTimeMuMuTOPTRG = new TCanvas("ECLTimeMuMuTOPTRG", "ECL time #mu#mu TOPTRG jitter");
  m_cCDCTimeHadronsTOPTRG = new TCanvas("CDCTimeHadronsTOPTRG", "CDC time hadrons TOPTRG jitter");
  m_cCDCTimeBhaBhaTOPTRG = new TCanvas("CDCTimeBhaBhaTOPTRG", "CDC time BhaBha TOPTRG jitter");
  m_cCDCTimeMuMuTOPTRG = new TCanvas("CDCTimeMuMuTOPTRG", "CDC time #mu#mu TOPTRG jitter");
  m_cTOPTimeHadronsTOPTRG = new TCanvas("TOPTimeHadronsTOPTRG", "TOP time hadrons TOPTRG jitter");
  m_cTOPTimeBhaBhaTOPTRG = new TCanvas("TOPTimeBhaBhaTOPTRG", "TOP time BhaBha TOPTRG jitter");
  m_cTOPTimeMuMuTOPTRG = new TCanvas("TOPTimeMuMuTOPTRG", "TOP time #mu#mu TOPTRG jitter");
  m_cSVDTimeHadronsTOPTRG = new TCanvas("SVDTimeHadronsTOPTRG", "SVD time hadrons TOPTRG jitter");
  m_cSVDTimeBhaBhaTOPTRG = new TCanvas("SVDTimeBhaBhaTOPTRG", "SVD time BhaBha TOPTRG jitter");
  m_cSVDTimeMuMuTOPTRG = new TCanvas("SVDTimeMuMuTOPTRG", "SVD time #mu#mu TOPTRG jitter");
}

void DQMHistAnalysisEventT0TriggerJitterModule::clearCanvases()
{
  m_cECLTimeHadronsECLTRG->Clear();
  m_cECLTimeBhaBhaECLTRG->Clear();
  m_cECLTimeMuMuECLTRG->Clear();
  m_cCDCTimeHadronsECLTRG->Clear();
  m_cCDCTimeBhaBhaECLTRG->Clear();
  m_cCDCTimeMuMuECLTRG->Clear();
  m_cTOPTimeHadronsECLTRG->Clear();
  m_cTOPTimeBhaBhaECLTRG->Clear();
  m_cTOPTimeMuMuECLTRG->Clear();
  m_cSVDTimeHadronsECLTRG->Clear();
  m_cSVDTimeBhaBhaECLTRG->Clear();
  m_cSVDTimeMuMuECLTRG->Clear();

  m_cECLTimeHadronsCDCTRG->Clear();
  m_cECLTimeBhaBhaCDCTRG->Clear();
  m_cECLTimeMuMuCDCTRG->Clear();
  m_cCDCTimeHadronsCDCTRG->Clear();
  m_cCDCTimeBhaBhaCDCTRG->Clear();
  m_cCDCTimeMuMuCDCTRG->Clear();
  m_cTOPTimeHadronsCDCTRG->Clear();
  m_cTOPTimeBhaBhaCDCTRG->Clear();
  m_cTOPTimeMuMuCDCTRG->Clear();
  m_cSVDTimeHadronsCDCTRG->Clear();
  m_cSVDTimeBhaBhaCDCTRG->Clear();
  m_cSVDTimeMuMuCDCTRG->Clear();

  m_cECLTimeHadronsTOPTRG->Clear();
  m_cECLTimeBhaBhaTOPTRG->Clear();
  m_cECLTimeMuMuTOPTRG->Clear();
  m_cCDCTimeHadronsTOPTRG->Clear();
  m_cCDCTimeBhaBhaTOPTRG->Clear();
  m_cCDCTimeMuMuTOPTRG->Clear();
  m_cTOPTimeHadronsTOPTRG->Clear();
  m_cTOPTimeBhaBhaTOPTRG->Clear();
  m_cTOPTimeMuMuTOPTRG->Clear();
  m_cSVDTimeHadronsTOPTRG->Clear();
  m_cSVDTimeBhaBhaTOPTRG->Clear();
  m_cSVDTimeMuMuTOPTRG->Clear();
}

void DQMHistAnalysisEventT0TriggerJitterModule::printCanvases()
{
  m_cECLTimeHadronsECLTRG->Print("EventT0_TriggerJitter.pdf(");
  m_cECLTimeBhaBhaECLTRG->Print("EventT0_TriggerJitter.pdf");
  m_cECLTimeMuMuECLTRG->Print("EventT0_TriggerJitter.pdf");
  m_cCDCTimeHadronsECLTRG->Print("EventT0_TriggerJitter.pdf");
  m_cCDCTimeBhaBhaECLTRG->Print("EventT0_TriggerJitter.pdf");
  m_cCDCTimeMuMuECLTRG->Print("EventT0_TriggerJitter.pdf");
  m_cTOPTimeHadronsECLTRG->Print("EventT0_TriggerJitter.pdf");
  m_cTOPTimeBhaBhaECLTRG->Print("EventT0_TriggerJitter.pdf");
  m_cTOPTimeMuMuECLTRG->Print("EventT0_TriggerJitter.pdf");
  m_cSVDTimeHadronsECLTRG->Print("EventT0_TriggerJitter.pdf");
  m_cSVDTimeBhaBhaECLTRG->Print("EventT0_TriggerJitter.pdf");
  m_cSVDTimeMuMuECLTRG->Print("EventT0_TriggerJitter.pdf");

  m_cECLTimeHadronsCDCTRG->Print("EventT0_TriggerJitter.pdf");
  m_cECLTimeBhaBhaCDCTRG->Print("EventT0_TriggerJitter.pdf");
  m_cECLTimeMuMuCDCTRG->Print("EventT0_TriggerJitter.pdf");
  m_cCDCTimeHadronsCDCTRG->Print("EventT0_TriggerJitter.pdf");
  m_cCDCTimeBhaBhaCDCTRG->Print("EventT0_TriggerJitter.pdf");
  m_cCDCTimeMuMuCDCTRG->Print("EventT0_TriggerJitter.pdf");
  m_cTOPTimeHadronsCDCTRG->Print("EventT0_TriggerJitter.pdf");
  m_cTOPTimeBhaBhaCDCTRG->Print("EventT0_TriggerJitter.pdf");
  m_cTOPTimeMuMuCDCTRG->Print("EventT0_TriggerJitter.pdf");
  m_cSVDTimeHadronsCDCTRG->Print("EventT0_TriggerJitter.pdf");
  m_cSVDTimeBhaBhaCDCTRG->Print("EventT0_TriggerJitter.pdf");
  m_cSVDTimeMuMuCDCTRG->Print("EventT0_TriggerJitter.pdf");

  m_cECLTimeHadronsTOPTRG->Print("EventT0_TriggerJitter.pdf");
  m_cECLTimeBhaBhaTOPTRG->Print("EventT0_TriggerJitter.pdf");
  m_cECLTimeMuMuTOPTRG->Print("EventT0_TriggerJitter.pdf");
  m_cCDCTimeHadronsTOPTRG->Print("EventT0_TriggerJitter.pdf");
  m_cCDCTimeBhaBhaTOPTRG->Print("EventT0_TriggerJitter.pdf");
  m_cCDCTimeMuMuTOPTRG->Print("EventT0_TriggerJitter.pdf");
  m_cTOPTimeHadronsTOPTRG->Print("EventT0_TriggerJitter.pdf");
  m_cTOPTimeBhaBhaTOPTRG->Print("EventT0_TriggerJitter.pdf");
  m_cTOPTimeMuMuTOPTRG->Print("EventT0_TriggerJitter.pdf");
  m_cSVDTimeHadronsTOPTRG->Print("EventT0_TriggerJitter.pdf");
  m_cSVDTimeBhaBhaTOPTRG->Print("EventT0_TriggerJitter.pdf");
  m_cSVDTimeMuMuTOPTRG->Print("EventT0_TriggerJitter.pdf)");
}


void DQMHistAnalysisEventT0TriggerJitterModule::deleteCanvases()
{
  if (m_cECLTimeHadronsECLTRG) delete m_cECLTimeHadronsECLTRG;
  if (m_cECLTimeBhaBhaECLTRG) delete m_cECLTimeBhaBhaECLTRG ;
  if (m_cECLTimeMuMuECLTRG) delete m_cECLTimeMuMuECLTRG;
  if (m_cCDCTimeHadronsECLTRG) delete m_cCDCTimeHadronsECLTRG;
  if (m_cCDCTimeBhaBhaECLTRG) delete m_cCDCTimeBhaBhaECLTRG ;
  if (m_cCDCTimeMuMuECLTRG) delete m_cCDCTimeMuMuECLTRG;
  if (m_cTOPTimeHadronsECLTRG) delete m_cTOPTimeHadronsECLTRG;
  if (m_cTOPTimeBhaBhaECLTRG) delete m_cTOPTimeBhaBhaECLTRG ;
  if (m_cTOPTimeMuMuECLTRG) delete m_cTOPTimeMuMuECLTRG;
  if (m_cSVDTimeHadronsECLTRG) delete m_cSVDTimeHadronsECLTRG;
  if (m_cSVDTimeBhaBhaECLTRG) delete m_cSVDTimeBhaBhaECLTRG ;
  if (m_cSVDTimeMuMuECLTRG) delete m_cSVDTimeMuMuECLTRG;

  if (m_cECLTimeHadronsCDCTRG) delete m_cECLTimeHadronsCDCTRG;
  if (m_cECLTimeBhaBhaCDCTRG) delete m_cECLTimeBhaBhaCDCTRG ;
  if (m_cECLTimeMuMuCDCTRG) delete m_cECLTimeMuMuCDCTRG;
  if (m_cCDCTimeHadronsCDCTRG) delete m_cCDCTimeHadronsCDCTRG;
  if (m_cCDCTimeBhaBhaCDCTRG) delete m_cCDCTimeBhaBhaCDCTRG ;
  if (m_cCDCTimeMuMuCDCTRG) delete m_cCDCTimeMuMuCDCTRG;
  if (m_cTOPTimeHadronsCDCTRG) delete m_cTOPTimeHadronsCDCTRG;
  if (m_cTOPTimeBhaBhaCDCTRG) delete m_cTOPTimeBhaBhaCDCTRG ;
  if (m_cTOPTimeMuMuCDCTRG) delete m_cTOPTimeMuMuCDCTRG;
  if (m_cSVDTimeHadronsCDCTRG) delete m_cSVDTimeHadronsCDCTRG;
  if (m_cSVDTimeBhaBhaCDCTRG) delete m_cSVDTimeBhaBhaCDCTRG ;
  if (m_cSVDTimeMuMuCDCTRG) delete m_cSVDTimeMuMuCDCTRG;

  if (m_cTOPTimeHadronsTOPTRG) delete m_cTOPTimeHadronsTOPTRG;
  if (m_cTOPTimeBhaBhaTOPTRG) delete m_cTOPTimeBhaBhaTOPTRG ;
  if (m_cTOPTimeMuMuTOPTRG) delete m_cTOPTimeMuMuTOPTRG;
  if (m_cECLTimeHadronsTOPTRG) delete m_cECLTimeHadronsTOPTRG;
  if (m_cECLTimeBhaBhaTOPTRG) delete m_cECLTimeBhaBhaTOPTRG ;
  if (m_cECLTimeMuMuTOPTRG) delete m_cECLTimeMuMuTOPTRG;
  if (m_cSVDTimeHadronsTOPTRG) delete m_cSVDTimeHadronsTOPTRG;
  if (m_cSVDTimeBhaBhaTOPTRG) delete m_cSVDTimeBhaBhaTOPTRG ;
  if (m_cSVDTimeMuMuTOPTRG) delete m_cSVDTimeMuMuTOPTRG;
  if (m_cCDCTimeHadronsTOPTRG) delete m_cCDCTimeHadronsTOPTRG;
  if (m_cCDCTimeBhaBhaTOPTRG) delete m_cCDCTimeBhaBhaTOPTRG ;
  if (m_cCDCTimeMuMuTOPTRG) delete m_cCDCTimeMuMuTOPTRG;
}
