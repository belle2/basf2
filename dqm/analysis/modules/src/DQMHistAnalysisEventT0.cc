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
  m_cTOPTimeHadronsECLTRG = new TCanvas("TOPTimeHadronsECLTRG", "TOP pad1 ECLTRG");
  m_cTOPTimeBhaBhaECLTRG = new TCanvas("TOPTimeBhaBhaECLTRG", "TOP pad2 ECLTRG");
  m_cTOPTimeMuMuECLTRG = new TCanvas("TOPTimeMuMuECLTRG", "TOP pad3 ECLTRG");

  //CDCTRG canvases
  m_cTOPTimeHadronsCDCTRG = new TCanvas("TOPTimeHadronsCDCTRG", "TOP pad1 CDCTRG");
  m_cTOPTimeBhaBhaCDCTRG = new TCanvas("TOPTimeBhaBhaCDCTRG", "TOP pad2 CDCTRG");
  m_cTOPTimeMuMuCDCTRG = new TCanvas("TOPTimeMuMuCDCTRG", "TOP pad3 CDCTRG");

  //SVD canvases
  //ECLTRG canvas
  m_cSVDTimeHadronsECLTRG = new TCanvas("SVDTimeHadronsECLTRG", "SVD pad1 ECLTRG");
  m_cSVDTimeBhaBhaECLTRG = new TCanvas("SVDTimeBhaBhaECLTRG", "SVD pad2 ECLTRG");
  m_cSVDTimeMuMuECLTRG = new TCanvas("SVDTimeMuMuECLTRG", "SVD pad3 ECLTRG");

  // CDC TRG canvas
  m_cSVDTimeHadronsCDCTRG = new TCanvas("SVDTimeHadronsCDCTRG", "SVD pad1 CDCTRG");
  m_cSVDTimeBhaBhaCDCTRG = new TCanvas("SVDTimeBhaBhaCDCTRG", "SVD pad2 CDCTRG");
  m_cSVDTimeMuMuCDCTRG = new TCanvas("SVDTimeMuMuCDCTRG", "SVD pad3 CDCTRG");

  // EventT0 source fractions
  m_cT0FractionsHadronECLTRG = new TCanvas("EventT0/c_HadronECLTRG", "Fractions ECLTRG for hadron events");
  m_cT0FractionsHadronCDCTRG = new TCanvas("EventT0/c_HadronCDCTRG", "Fractions CDCTRG for hadron events");
  m_cT0FractionsHadronTOPTRG = new TCanvas("EventT0/c_HadronTOPTRG", "Fractions TOPTRG for hadron events");

  m_cT0FractionsBhaBhaECLTRG = new TCanvas("EventT0/c_BhaBhaECLTRG", "Fractions ECLTRG for BhaBha events");
  m_cT0FractionsBhaBhaCDCTRG = new TCanvas("EventT0/c_BhaBhaCDCTRG", "Fractions CDCTRG for BhaBha events");
  m_cT0FractionsBhaBhaTOPTRG = new TCanvas("EventT0/c_BhaBhaTOPTRG", "Fractions TOPTRG for BhaBha events");

  m_cT0FractionsMuMuECLTRG = new TCanvas("EventT0/c_MuMuECLTRG", "Fractions ECLTRG for #mu#mu events");
  m_cT0FractionsMuMuCDCTRG = new TCanvas("EventT0/c_MuMuCDCTRG", "Fractions CDCTRG for #mu#mu events");
  m_cT0FractionsMuMuTOPTRG = new TCanvas("EventT0/c_MuMuTOPTRG", "Fractions TOPTRG for #mu#mu events");

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
}


void DQMHistAnalysisEventT0Module::beginRun()
{
  m_cTOPTimeHadronsECLTRG->Clear();
  m_cTOPTimeBhaBhaECLTRG->Clear();
  m_cTOPTimeMuMuECLTRG->Clear();
  m_cTOPTimeHadronsCDCTRG->Clear();
  m_cTOPTimeBhaBhaCDCTRG->Clear();
  m_cTOPTimeMuMuCDCTRG->Clear();
  m_cSVDTimeHadronsECLTRG->Clear();
  m_cSVDTimeBhaBhaECLTRG->Clear();
  m_cSVDTimeMuMuECLTRG->Clear();
  m_cSVDTimeHadronsCDCTRG->Clear();
  m_cSVDTimeBhaBhaCDCTRG->Clear();
  m_cSVDTimeMuMuCDCTRG->Clear();

  m_cT0FractionsHadronECLTRG->Clear();
  m_cT0FractionsHadronCDCTRG->Clear();
  m_cT0FractionsHadronTOPTRG->Clear();
  m_cT0FractionsBhaBhaECLTRG->Clear();
  m_cT0FractionsBhaBhaCDCTRG->Clear();
  m_cT0FractionsBhaBhaTOPTRG->Clear();
  m_cT0FractionsMuMuECLTRG->Clear();
  m_cT0FractionsMuMuCDCTRG->Clear();
  m_cT0FractionsMuMuTOPTRG->Clear();

}

void DQMHistAnalysisEventT0Module::event()
{
  std::string histname = "AlgorithmSourceFractionsHadronL1ECLTRG";
  m_cT0FractionsHadronECLTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsHadronL1ECLTRG)) {
    m_cT0FractionsHadronECLTRG->SetFillColor(0);
    m_cT0FractionsHadronECLTRG->Modified();
    m_cT0FractionsHadronECLTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for hadrons from ECLTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cT0FractionsHadronECLTRG->SetFillColor(kGray);
  }

  histname = "AlgorithmSourceFractionsHadronL1CDCTRG";
  m_cT0FractionsHadronCDCTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsHadronL1CDCTRG)) {
    m_cT0FractionsHadronCDCTRG->SetFillColor(0);
    m_cT0FractionsHadronCDCTRG->Modified();
    m_cT0FractionsHadronCDCTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for hadrons from CDCTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cT0FractionsHadronCDCTRG->SetFillColor(kGray);
  }

  histname = "AlgorithmSourceFractionsHadronL1TOPTRG";
  m_cT0FractionsHadronTOPTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsHadronL1TOPTRG)) {
    m_cT0FractionsHadronTOPTRG->SetFillColor(0);
    m_cT0FractionsHadronTOPTRG->Modified();
    m_cT0FractionsHadronTOPTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for hadrons from TOPTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cT0FractionsHadronTOPTRG->SetFillColor(kGray);
  }


  histname = "AlgorithmSourceFractionsBhaBhaL1ECLTRG";
  m_cT0FractionsBhaBhaECLTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsBhaBhaL1ECLTRG)) {
    m_cT0FractionsBhaBhaECLTRG->SetFillColor(0);
    m_cT0FractionsBhaBhaECLTRG->Modified();
    m_cT0FractionsBhaBhaECLTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for BhaBha from ECLTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cT0FractionsBhaBhaECLTRG->SetFillColor(kGray);
  }

  histname = "AlgorithmSourceFractionsBhaBhaL1CDCTRG";
  m_cT0FractionsBhaBhaCDCTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsBhaBhaL1CDCTRG)) {
    m_cT0FractionsBhaBhaCDCTRG->SetFillColor(0);
    m_cT0FractionsBhaBhaCDCTRG->Modified();
    m_cT0FractionsBhaBhaCDCTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for BhaBha from CDCTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cT0FractionsBhaBhaCDCTRG->SetFillColor(kGray);
  }

  histname = "AlgorithmSourceFractionsBhaBhaL1TOPTRG";
  m_cT0FractionsBhaBhaTOPTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsBhaBhaL1TOPTRG)) {
    m_cT0FractionsBhaBhaTOPTRG->SetFillColor(0);
    m_cT0FractionsBhaBhaTOPTRG->Modified();
    m_cT0FractionsBhaBhaTOPTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for BhaBha from TOPTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cT0FractionsBhaBhaTOPTRG->SetFillColor(kGray);
  }


  histname = "AlgorithmSourceFractionsMuMuL1ECLTRG";
  m_cT0FractionsMuMuECLTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsMuMuL1ECLTRG)) {
    m_cT0FractionsMuMuECLTRG->SetFillColor(0);
    m_cT0FractionsMuMuECLTRG->Modified();
    m_cT0FractionsMuMuECLTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for MuMu from ECLTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cT0FractionsMuMuECLTRG->SetFillColor(kGray);
  }

  histname = "AlgorithmSourceFractionsMuMuL1CDCTRG";
  m_cT0FractionsMuMuCDCTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsMuMuL1CDCTRG)) {
    m_cT0FractionsMuMuCDCTRG->SetFillColor(0);
    m_cT0FractionsMuMuCDCTRG->Modified();
    m_cT0FractionsMuMuCDCTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for MuMu from CDCTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cT0FractionsMuMuCDCTRG->SetFillColor(kGray);
  }

  histname = "AlgorithmSourceFractionsMuMuL1TOPTRG";
  m_cT0FractionsMuMuTOPTRG->cd();
  if (FillEfficiencyHistogram(histname, m_eAlgorithmSourceFractionsMuMuL1TOPTRG)) {
    m_cT0FractionsMuMuTOPTRG->SetFillColor(0);
    m_cT0FractionsMuMuTOPTRG->Modified();
    m_cT0FractionsMuMuTOPTRG->Update();
  } else {
    B2WARNING("Histogram EventT0 source fractions for MuMu from TOPTRG events (" << histname << ") from EventT0 DQM not processed!");
    m_cT0FractionsMuMuTOPTRG->SetFillColor(kGray);
  }

  if (m_printCanvas) {
    m_cT0FractionsHadronECLTRG->Print(Form("%s_EventT0FractionsHardronECLTRG.pdf[", m_prefixCanvas.c_str()));
    m_cT0FractionsHadronCDCTRG->Print(Form("%s_EventT0FractionsHardronCDCTRG.pdf", m_prefixCanvas.c_str()));
    m_cT0FractionsHadronTOPTRG->Print(Form("%s_EventT0FractionsHardronTOPTRG.pdf", m_prefixCanvas.c_str()));

    m_cT0FractionsBhaBhaECLTRG->Print(Form("%s_EventT0FractionsBhaBhaECLTRG.pdf", m_prefixCanvas.c_str()));
    m_cT0FractionsBhaBhaCDCTRG->Print(Form("%s_EventT0FractionsBhaBhaCDCTRG.pdf", m_prefixCanvas.c_str()));
    m_cT0FractionsBhaBhaTOPTRG->Print(Form("%s_EventT0FractionsBhaBhaTOPTRG.pdf", m_prefixCanvas.c_str()));

    m_cT0FractionsMuMuECLTRG->Print(Form("%s_EventT0FractionsMuMuECLTRG.pdf", m_prefixCanvas.c_str()));
    m_cT0FractionsMuMuCDCTRG->Print(Form("%s_EventT0FractionsMuMuCDCTRG.pdf", m_prefixCanvas.c_str()));
    m_cT0FractionsMuMuTOPTRG->Print(Form("%s_EventT0FractionsMuMuTOPTRG.pdf]", m_prefixCanvas.c_str()));
  }

}

void DQMHistAnalysisEventT0Module::endRun()
{
  // --- TOP EventT0 plots for ECLTRG ---

  // find TOP EventT0 Hadrons ECLTRG histogram and process it
  TH1* h = findHist("EventT0/m_histEventT0_TOP_hadron_L1_ECLTRG");
  TString tag = "hadronECLTRG";
  m_cTOPTimeHadronsECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cTOPTimeHadronsECLTRG->SetFillColor(0);
    m_cTOPTimeHadronsECLTRG->Modified();
    m_cTOPTimeHadronsECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cTOPTimeHadronsECLTRG->SetFillColor(kGray);
    m_cTOPTimeHadronsECLTRG->Draw();
  }

  // find TOP EventT0 Bhabhas ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_bhabha_L1_ECLTRG");
  tag = "bhabhaECLTRG";
  m_cTOPTimeBhaBhaECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cTOPTimeBhaBhaECLTRG->SetFillColor(0);
    m_cTOPTimeBhaBhaECLTRG->Modified();
    m_cTOPTimeBhaBhaECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cTOPTimeBhaBhaECLTRG->SetFillColor(kGray);
    m_cTOPTimeBhaBhaECLTRG->Draw();
  }

  // find TOP EventT0 Mumus ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_mumu_L1_ECLTRG");
  tag = "mumuECLTRG";
  m_cTOPTimeMuMuECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cTOPTimeMuMuECLTRG->SetFillColor(0);
    m_cTOPTimeMuMuECLTRG->Modified();
    m_cTOPTimeMuMuECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cTOPTimeMuMuECLTRG->SetFillColor(kGray);
    m_cTOPTimeMuMuECLTRG->Draw();
  }

  // --- TOP EventT0 plots for CDCTRG ---

  // find TOP EventT0 Hadrons CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_hadron_L1_CDCTRG");
  tag = "hadronCDCTRG";
  m_cTOPTimeHadronsCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cTOPTimeHadronsCDCTRG->SetFillColor(0);
    m_cTOPTimeHadronsCDCTRG->Modified();
    m_cTOPTimeHadronsCDCTRG->Update();
    m_cTOPTimeHadronsCDCTRG->Draw();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cTOPTimeHadronsCDCTRG->SetFillColor(kGray);
    m_cTOPTimeHadronsCDCTRG->Draw();
  }

  // find TOP EventT0 Bhabhas CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_bhabha_L1_CDCTRG");
  tag = "bhabhaCDCTRG";
  m_cTOPTimeBhaBhaCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cTOPTimeBhaBhaCDCTRG->SetFillColor(0);
    m_cTOPTimeBhaBhaCDCTRG->Modified();
    m_cTOPTimeBhaBhaCDCTRG->Update();
    m_cTOPTimeBhaBhaCDCTRG->Draw();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cTOPTimeBhaBhaCDCTRG->SetFillColor(kGray);
    m_cTOPTimeBhaBhaCDCTRG->Draw();
  }

  // find TOP EventT0 Mumus CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_TOP_mumu_L1_CDCTRG");
  tag = "mumuCDCTRG";
  m_cTOPTimeMuMuCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cTOPTimeMuMuCDCTRG->SetFillColor(0);
    m_cTOPTimeMuMuCDCTRG->Modified();
    m_cTOPTimeMuMuCDCTRG->Update();
  } else {
    B2WARNING(Form("Histogram TOP EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cTOPTimeMuMuCDCTRG->SetFillColor(kGray);
    m_cTOPTimeMuMuCDCTRG->Draw();
  }


  // --- SVD EventT0 plots for ECLTRG ---

  // find SVD EventT0 Hadrons ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_hadron_L1_ECLTRG");
  tag = "hadronECLTRG";
  m_cSVDTimeHadronsECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cSVDTimeHadronsECLTRG->SetFillColor(0);
    m_cSVDTimeHadronsECLTRG->Modified();
    m_cSVDTimeHadronsECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cSVDTimeHadronsECLTRG->SetFillColor(kGray);
    m_cSVDTimeHadronsECLTRG->Draw();
  }

  // find SVD EventT0 Bhabhas ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_bhabha_L1_ECLTRG");
  tag = "bhabhaECLTRG";
  m_cSVDTimeBhaBhaECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cSVDTimeBhaBhaECLTRG->SetFillColor(0);
    m_cSVDTimeBhaBhaECLTRG->Modified();
    m_cSVDTimeBhaBhaECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cSVDTimeBhaBhaECLTRG->SetFillColor(kGray);
    m_cSVDTimeBhaBhaECLTRG->Draw();
  }

  // find SVD EventT0 Mumus ECLTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_mumu_L1_ECLTRG");
  tag = "mumuECLTRG";
  m_cSVDTimeMuMuECLTRG->cd();
  if (processHistogram(h, tag)) {
    m_cSVDTimeMuMuECLTRG->SetFillColor(0);
    m_cSVDTimeMuMuECLTRG->Modified();
    m_cSVDTimeMuMuECLTRG->Update();
  } else {
    B2WARNING(Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cSVDTimeMuMuECLTRG->SetFillColor(kGray);
    m_cSVDTimeMuMuECLTRG->Draw();
  }


  // --- SVD EventT0 plots for CDCTRG ---

  // find SVD EventT0 Hadrons CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_hadron_L1_CDCTRG");
  tag = "hadronCDCTRG";
  m_cSVDTimeHadronsCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cSVDTimeHadronsCDCTRG->SetFillColor(0);
    m_cSVDTimeHadronsCDCTRG->Modified();
    m_cSVDTimeHadronsCDCTRG->Update();
    m_cSVDTimeHadronsCDCTRG->Draw();
  } else {
    B2WARNING(Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cSVDTimeHadronsCDCTRG->SetFillColor(kGray);
    m_cSVDTimeHadronsCDCTRG->Draw();
  }

  // find SVD EventT0 Bhabhas CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_bhabha_L1_CDCTRG");
  tag = "bhabhaCDCTRG";
  m_cSVDTimeBhaBhaCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cSVDTimeBhaBhaCDCTRG->SetFillColor(0);
    m_cSVDTimeBhaBhaCDCTRG->Modified();
    m_cSVDTimeBhaBhaCDCTRG->Update();
    m_cSVDTimeBhaBhaCDCTRG->Draw();
  } else {
    B2WARNING(Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cSVDTimeBhaBhaCDCTRG->SetFillColor(kGray);
    m_cSVDTimeBhaBhaCDCTRG->Draw();
  }


  // find SVD EventT0 Mumus CDCTRG histogram and process it
  h = findHist("EventT0/m_histEventT0_SVD_mumu_L1_CDCTRG");
  tag = "mumuCDCTRG";
  m_cSVDTimeMuMuCDCTRG->cd();
  if (processHistogram(h, tag)) {
    m_cSVDTimeMuMuCDCTRG->SetFillColor(0);
    m_cSVDTimeMuMuCDCTRG->Modified();
    m_cSVDTimeMuMuCDCTRG->Update();
  } else {
    B2WARNING(Form("Histogram SVD EventT0 for %s from EventT0 DQM not processed!", tag.Data()));
    if (h) h->Draw();
    m_cSVDTimeMuMuCDCTRG->SetFillColor(kGray);
    m_cSVDTimeMuMuCDCTRG->Draw();
  }

  if (m_printCanvas) {
    m_cTOPTimeHadronsECLTRG->Print(Form("%s_TOPTimeHadronsECLTRG.pdf[", m_prefixCanvas.c_str()));
    m_cTOPTimeBhaBhaECLTRG->Print(Form("%s_TOPTimeBhaBhaECLTRG.pdf", m_prefixCanvas.c_str()));
    m_cTOPTimeMuMuECLTRG->Print(Form("%s_TOPTimeMuMuECLTRG.pdf", m_prefixCanvas.c_str()));

    m_cTOPTimeHadronsCDCTRG->Print(Form("%s_TOPTimeHadronsCDCTRG.pdf", m_prefixCanvas.c_str()));
    m_cTOPTimeBhaBhaCDCTRG->Print(Form("%s_TOPTimeBhaBhaCDCTRG.pdf", m_prefixCanvas.c_str()));
    m_cTOPTimeMuMuCDCTRG->Print(Form("%s_TOPTimeMuMuCDCTRG.pdf", m_prefixCanvas.c_str()));

    m_cSVDTimeHadronsECLTRG->Print(Form("%s_SVDTimeHadronsECLTRG.pdf", m_prefixCanvas.c_str()));
    m_cSVDTimeBhaBhaECLTRG->Print(Form("%s_SVDTimeBhaBhaECLTRG.pdf", m_prefixCanvas.c_str()));
    m_cSVDTimeMuMuECLTRG->Print(Form("%s_SVDTimeMuMuECLTRG.pdf", m_prefixCanvas.c_str()));

    m_cSVDTimeHadronsCDCTRG->Print(Form("%s_SVDTimeHadronsCDCTRG.pdf", m_prefixCanvas.c_str()));
    m_cSVDTimeBhaBhaCDCTRG->Print(Form("%s_SVDTimeBhaBhaCDCTRG.pdf", m_prefixCanvas.c_str()));
    m_cSVDTimeMuMuCDCTRG->Print(Form("%s_SVDTimeMuMuCDCTRG.pdf]", m_prefixCanvas.c_str()));
  }


}

void DQMHistAnalysisEventT0Module::terminate()
{
  delete m_cTOPTimeHadronsECLTRG;
  delete m_cTOPTimeBhaBhaECLTRG;
  delete m_cTOPTimeMuMuECLTRG;
  delete m_cTOPTimeHadronsCDCTRG;
  delete m_cTOPTimeBhaBhaCDCTRG;
  delete m_cTOPTimeMuMuCDCTRG;
  delete m_cSVDTimeHadronsECLTRG;
  delete m_cSVDTimeBhaBhaECLTRG;
  delete m_cSVDTimeMuMuECLTRG;
  delete m_cSVDTimeHadronsCDCTRG;
  delete m_cSVDTimeBhaBhaCDCTRG;
  delete m_cSVDTimeMuMuCDCTRG;

  delete m_cT0FractionsHadronECLTRG;
  delete m_cT0FractionsHadronCDCTRG;
  delete m_cT0FractionsHadronTOPTRG;
  delete m_cT0FractionsBhaBhaECLTRG;
  delete m_cT0FractionsBhaBhaCDCTRG;
  delete m_cT0FractionsBhaBhaTOPTRG;
  delete m_cT0FractionsMuMuECLTRG;
  delete m_cT0FractionsMuMuCDCTRG;
  delete m_cT0FractionsMuMuTOPTRG;

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

