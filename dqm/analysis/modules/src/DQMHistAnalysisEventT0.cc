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

  // EventT0 source fractions
  m_cT0FractionsForHadrons = new TCanvas("EventT0/c_T0FractionsForHadrons", "EventT0 source fractions for hadron events", 1200, 400);
  m_cHadronECLTRG = new TCanvas("EventT0/c_HadronECLTRG", "Fractions ECLTRG", 0.03, 0.02, 0.33, 0.98);
  m_cHadronCDCTRG = new TCanvas("EventT0/c_HadronCDCTRG", "Fractions CDCTRG", 0.35, 0.02, 0.65, 0.98);
  m_cHadronTOPTRG = new TCanvas("EventT0/c_HadronTOPTRG", "Fractions TOPTRG", 0.67, 0.02, 0.97, 0.98);

  m_cT0FractionsForBhaBhas = new TCanvas("EventT0/c_T0FractionsForBhaBhas", "EventT0 source fractions for bhabha events", 1200, 400);
  m_cBhaBhaECLTRG = new TCanvas("EventT0/c_BhaBhaECLTRG", "Fractions ECLTRG", 0.03, 0.02, 0.33, 0.98);
  m_cBhaBhaCDCTRG = new TCanvas("EventT0/c_BhaBhaCDCTRG", "Fractions CDCTRG", 0.35, 0.02, 0.65, 0.98);
  m_cBhaBhaTOPTRG = new TCanvas("EventT0/c_BhaBhaTOPTRG", "Fractions TOPTRG", 0.67, 0.02, 0.97, 0.98);

  m_cT0FractionsForMuMus = new TCanvas("EventT0/c_T0FractionsForMuMus", "EventT0 source fractions for #mu#mu events", 1200, 400);
  m_cMuMuECLTRG = new TCanvas("EventT0/c_MuMuECLTRG", "Fractions ECLTRG", 0.03, 0.02, 0.33, 0.98);
  m_cMuMuCDCTRG = new TCanvas("EventT0/c_MuMuCDCTRG", "Fractions CDCTRG", 0.35, 0.02, 0.65, 0.98);
  m_cMuMuTOPTRG = new TCanvas("EventT0/c_MuMuTOPTRG", "Fractions TOPTRG", 0.67, 0.02, 0.97, 0.98);

  m_eAlgorithmSourceFractionsHadronL1ECLTRG =
    new TEfficiency("effAlgorithmSourceFractionsHadronL1ECLTRG",
                    "EventT0 source fractions, hadronic events, L1ECLTRG;Algorithm;Fraction #epsilon",
                    6, 0, 6);
  m_eAlgorithmSourceFractionsHadronL1CDCTRG =
    new TEfficiency("effAlgorithmSourceFractionsHadronL1CDCTRG",
                    "EventT0 source fractions, hadronic events, L1CDCTRG;Algorithm;Fraction #epsilon",
                    6, 0, 6);
  m_eAlgorithmSourceFractionsHadronL1TOPTRG =
    new TEfficiency("effAlgorithmSourceFractionsHadronL1TOPTRG",
                    "EventT0 source fractions, hadronic events, L1TOPTRG;Algorithm;Fraction #epsilon",
                    6, 0, 6);
  m_eAlgorithmSourceFractionsBhaBhaL1ECLTRG =
    new TEfficiency("effAlgorithmSourceFractionsBhaBhaL1ECLTRG",
                    "EventT0 source fractions, Bhabha events, L1ECLTRG;Algorithm;Fraction #epsilon",
                    6, 0, 6);
  m_eAlgorithmSourceFractionsBhaBhaL1CDCTRG =
    new TEfficiency("effAlgorithmSourceFractionsBhaBhaL1CDCTRG",
                    "EventT0 source fractions, Bhabha events, L1CDCTRG;Algorithm;Fraction #epsilon",
                    6, 0, 6);
  m_eAlgorithmSourceFractionsBhaBhaL1TOPTRG =
    new TEfficiency("effAlgorithmSourceFractionsBhaBhaL1TOPTRG",
                    "EventT0 source fractions, Bhabha events, L1TOPTRG;Algorithm;Fraction #epsilon",
                    6, 0, 6);
  m_eAlgorithmSourceFractionsMuMuL1ECLTRG =
    new TEfficiency("effAlgorithmSourceFractionsMuMuL1ECLTRG",
                    "EventT0 source fractions, #mu#mu events, L1ECLTRG;Algorithm;Fraction #epsilon",
                    6, 0, 6);
  m_eAlgorithmSourceFractionsMuMuL1CDCTRG =
    new TEfficiency("effAlgorithmSourceFractionsMuMuL1CDCTRG",
                    "EventT0 source fractions, #mu#mu events, L1CDCTRG;Algorithm;Fraction #epsilon",
                    6, 0, 6);
  m_eAlgorithmSourceFractionsMuMuL1TOPTRG =
    new TEfficiency("effAlgorithmSourceFractionsMuMuL1TOPTRG",
                    "EventT0 source fractions, #mu#mu events, L1TOPTRG;Algorithm;Fraction #epsilon",
                    6, 0, 6);

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

  m_cT0FractionsForHadrons->Clear();
  m_cT0FractionsForBhaBhas->Clear();
  m_cT0FractionsForMuMus->Clear();
}

void DQMHistAnalysisEventT0Module::event()
{
  std::string histname = "AlgorithmSourceFractionsHadronL1ECLTRG";
  m_cHadronECLTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsHadronL1ECLTRG)) {
    m_cHadronECLTRG->SetFillColor(0);
    m_cHadronECLTRG->Modified();
    m_cHadronECLTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for hadrons from ECLTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cHadronECLTRG->SetFillColor(kGray);
  }

  histname = "AlgorithmSourceFractionsHadronL1CDCTRG";
  m_cHadronCDCTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsHadronL1CDCTRG)) {
    m_cHadronCDCTRG->SetFillColor(0);
    m_cHadronCDCTRG->Modified();
    m_cHadronCDCTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for hadrons from CDCTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cHadronCDCTRG->SetFillColor(kGray);
  }

  histname = "AlgorithmSourceFractionsHadronL1TOPTRG";
  m_cHadronTOPTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsHadronL1TOPTRG)) {
    m_cHadronTOPTRG->SetFillColor(0);
    m_cHadronTOPTRG->Modified();
    m_cHadronTOPTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for hadrons from TOPTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cHadronTOPTRG->SetFillColor(kGray);
  }

  // anonymous helper to draw the pads consistently without repeating oneself
  const auto drawPad = [](TPad * pad) {
    pad->SetMargin(0.07, 0.0, 0.07, 0.07);
    pad->Draw();
  };
  m_cT0FractionsForHadrons->cd();
  drawPad(m_cHadronECLTRG);
  drawPad(m_cHadronCDCTRG);
  drawPad(m_cHadronTOPTRG);

  if (m_printCanvas)
    m_cT0FractionsForHadrons->Print(Form("%s_EventT0FractionsHadron.pdf", m_prefixCanvas.c_str()));


  histname = "AlgorithmSourceFractionsBhaBhaL1ECLTRG";
  m_cBhaBhaECLTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsBhaBhaL1ECLTRG)) {
    m_cBhaBhaECLTRG->SetFillColor(0);
    m_cBhaBhaECLTRG->Modified();
    m_cBhaBhaECLTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for BhaBha from ECLTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cBhaBhaECLTRG->SetFillColor(kGray);
  }

  histname = "AlgorithmSourceFractionsBhaBhaL1CDCTRG";
  m_cBhaBhaCDCTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsBhaBhaL1CDCTRG)) {
    m_cBhaBhaCDCTRG->SetFillColor(0);
    m_cBhaBhaCDCTRG->Modified();
    m_cBhaBhaCDCTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for BhaBha from CDCTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cBhaBhaCDCTRG->SetFillColor(kGray);
  }

  histname = "AlgorithmSourceFractionsBhaBhaL1TOPTRG";
  m_cBhaBhaTOPTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsBhaBhaL1TOPTRG)) {
    m_cBhaBhaTOPTRG->SetFillColor(0);
    m_cBhaBhaTOPTRG->Modified();
    m_cBhaBhaTOPTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for BhaBha from TOPTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cBhaBhaTOPTRG->SetFillColor(kGray);
  }

  m_cT0FractionsForBhaBhas->cd();
  drawPad(m_cBhaBhaECLTRG);
  drawPad(m_cBhaBhaCDCTRG);
  drawPad(m_cBhaBhaTOPTRG);

  if (m_printCanvas)
    m_cT0FractionsForBhaBhas->Print(Form("%s_EventT0FractionsBhaBha.pdf", m_prefixCanvas.c_str()));


  histname = "AlgorithmSourceFractionsMuMuL1ECLTRG";
  m_cMuMuECLTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsMuMuL1ECLTRG)) {
    m_cMuMuECLTRG->SetFillColor(0);
    m_cMuMuECLTRG->Modified();
    m_cMuMuECLTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for MuMu from ECLTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cMuMuECLTRG->SetFillColor(kGray);
  }

  histname = "AlgorithmSourceFractionsMuMuL1CDCTRG";
  m_cMuMuCDCTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsMuMuL1CDCTRG)) {
    m_cMuMuCDCTRG->SetFillColor(0);
    m_cMuMuCDCTRG->Modified();
    m_cMuMuCDCTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for MuMu from CDCTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cMuMuCDCTRG->SetFillColor(kGray);
  }

  histname = "AlgorithmSourceFractionsMuMuL1TOPTRG";
  m_cMuMuTOPTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsMuMuL1TOPTRG)) {
    m_cMuMuTOPTRG->SetFillColor(0);
    m_cMuMuTOPTRG->Modified();
    m_cMuMuTOPTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for MuMu from TOPTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cMuMuTOPTRG->SetFillColor(kGray);
  }

  m_cT0FractionsForMuMus->cd();
  drawPad(m_cMuMuECLTRG);
  drawPad(m_cMuMuCDCTRG);
  drawPad(m_cMuMuTOPTRG);

  if (m_printCanvas)
    m_cT0FractionsForMuMus->Print(Form("%s_EventT0FractionsMuMu.pdf", m_prefixCanvas.c_str()));

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
    h->Draw();
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
    h->Draw();
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
    h->Draw();
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
    h->Draw();
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
    h->Draw();
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
    h->Draw();
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
    h->Draw();
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
    h->Draw();
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
    h->Draw();
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
    h->Draw();
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
    h->Draw();
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
    h->Draw();
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

  delete m_cT0FractionsForHadrons;
  delete m_cT0FractionsForBhaBhas;
  delete m_cT0FractionsForMuMus;

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

bool DQMHistAnalysisEventT0Module::FillEfficiencyHistogram(const std::string& histname, TEfficiency* eff)
{
  B2DEBUG(20, "Begin processing histogram " << histname << " ...");
  TH1* h = findHist("EventT0/" + histname);
  if (not h) {
    return false;
  }

  // Admittedly quite a hacky way to obtain the normalisation values: Create a new histogram and fill each of the bins with
  // the bin content of the -1 bin of h which is used for bin counting, and at the same time set the corresponding bin label.
  const auto totalEntries = h->GetBinContent(-1);
  const auto nBins = h->GetNbinsX();
  TH1D* totalHist = new TH1D("total", "total;Algorithm;Fraction #epsilon", nBins, 0, nBins);
  for (int i = 0; i < nBins; i++) {
    totalHist->SetBinContent(i + 1, totalEntries);
  }
  eff->SetPassedHistogram(*h, "f");
  eff->SetTotalHistogram(*totalHist, "f");

  eff->Paint("AP");

  TGraphAsymmErrors* graph = eff->GetPaintedGraph();
  if (not graph) {
    return false;
  }

  auto ax = graph->GetXaxis();
  if (not ax) {
    return false;
  }
  // Print x-axis bin labels horizontally
  ax->SetTitleOffset(1.0);
  ax->CenterTitle(kTRUE);
  ax->Set(nBins, 0, nBins);
  for (int i = 0; i < nBins; i++) {
    ax->SetBinLabel(i + 1, c_eventT0Algorithms[i]);
  }

  auto ay = graph->GetYaxis();
  if (not ay) {
    return false;
  }
  ay->SetTitleOffset(1.0);
  ay->SetRangeUser(0, 1.05);

  graph->Draw("AP");

  B2DEBUG(20, "Finished processing histogram " << histname << "!");

  return true;
}

