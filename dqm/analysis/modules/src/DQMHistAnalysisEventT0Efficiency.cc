/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisEventT0Efficiency.h
// Description : module for DQM histogram analysis of EventT0 algorithm efficiencies
//-


#include <dqm/analysis/modules/DQMHistAnalysisEventT0Efficiency.h>

#include <TROOT.h>
#include <TGraphAsymmErrors.h>
#include <TStyle.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisEventT0Efficiency);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisEventT0EfficiencyModule::DQMHistAnalysisEventT0EfficiencyModule()
  : DQMHistAnalysisModule()
{
  setDescription("Calculate EventT0 algorithm efficiencies for the different trigger types, EventT0 algorithm sources, and event types (hadron, bhabha, µµ).");

  //Parameter definition
  addParam("min_nEntries", m_nEntriesMin, "Minimum number of entries to process the histogram.", m_nEntriesMin);
  addParam("prefixCanvas", m_prefixCanvas, "Prefix to be added to canvas filename when saved as pdf.", std::string("c"));
  addParam("printCanvas", m_printCanvas, "If true, prints pdf of the analysis canvas.", bool(false));
}


DQMHistAnalysisEventT0EfficiencyModule::~DQMHistAnalysisEventT0EfficiencyModule() { }

void DQMHistAnalysisEventT0EfficiencyModule::initialize()
{
  gROOT->cd();

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
                    "EventT0 source fractions, hadronic events, L1TRG timing from ECL;Algorithm;Fraction #epsilon",
                    6, 0, 6);
  m_eAlgorithmSourceFractionsHadronL1CDCTRG =
    new TEfficiency("effAlgorithmSourceFractionsHadronL1CDCTRG",
                    "EventT0 source fractions, hadronic events, L1TRG timing from CDC;Algorithm;Fraction #epsilon",
                    6, 0, 6);
  m_eAlgorithmSourceFractionsHadronL1TOPTRG =
    new TEfficiency("effAlgorithmSourceFractionsHadronL1TOPTRG",
                    "EventT0 source fractions, hadronic events, L1TRG timing from TOP;Algorithm;Fraction #epsilon",
                    6, 0, 6);
  m_eAlgorithmSourceFractionsBhaBhaL1ECLTRG =
    new TEfficiency("effAlgorithmSourceFractionsBhaBhaL1ECLTRG",
                    "EventT0 source fractions, Bhabha events, L1TRG timing from ECL;Algorithm;Fraction #epsilon",
                    6, 0, 6);
  m_eAlgorithmSourceFractionsBhaBhaL1CDCTRG =
    new TEfficiency("effAlgorithmSourceFractionsBhaBhaL1CDCTRG",
                    "EventT0 source fractions, Bhabha events, L1TRG timing from CDC;Algorithm;Fraction #epsilon",
                    6, 0, 6);
  m_eAlgorithmSourceFractionsBhaBhaL1TOPTRG =
    new TEfficiency("effAlgorithmSourceFractionsBhaBhaL1TOPTRG",
                    "EventT0 source fractions, Bhabha events, L1TRG timing from TOP;Algorithm;Fraction #epsilon",
                    6, 0, 6);
  m_eAlgorithmSourceFractionsMuMuL1ECLTRG =
    new TEfficiency("effAlgorithmSourceFractionsMuMuL1ECLTRG",
                    "EventT0 source fractions, #mu#mu events, L1TRG timing from ECL;Algorithm;Fraction #epsilon",
                    6, 0, 6);
  m_eAlgorithmSourceFractionsMuMuL1CDCTRG =
    new TEfficiency("effAlgorithmSourceFractionsMuMuL1CDCTRG",
                    "EventT0 source fractions, #mu#mu events, L1TRG timing from CDC;Algorithm;Fraction #epsilon",
                    6, 0, 6);
  m_eAlgorithmSourceFractionsMuMuL1TOPTRG =
    new TEfficiency("effAlgorithmSourceFractionsMuMuL1TOPTRG",
                    "EventT0 source fractions, #mu#mu events, L1TRG timing from TOP;Algorithm;Fraction #epsilon",
                    6, 0, 6);

  m_monObj = getMonitoringObject("eventT0");
}


void DQMHistAnalysisEventT0EfficiencyModule::beginRun()
{
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

void DQMHistAnalysisEventT0EfficiencyModule::event()
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
    m_cT0FractionsHadronECLTRG->Print("EventT0_Algorithm_Efficiency.pdf(");
    m_cT0FractionsHadronCDCTRG->Print("EventT0_Algorithm_Efficiency.pdf");
    m_cT0FractionsHadronTOPTRG->Print("EventT0_Algorithm_Efficiency.pdf");

    m_cT0FractionsBhaBhaECLTRG->Print("EventT0_Algorithm_Efficiency.pdf");
    m_cT0FractionsBhaBhaCDCTRG->Print("EventT0_Algorithm_Efficiency.pdf");
    m_cT0FractionsBhaBhaTOPTRG->Print("EventT0_Algorithm_Efficiency.pdf");

    m_cT0FractionsMuMuECLTRG->Print("EventT0_Algorithm_Efficiency.pdf");
    m_cT0FractionsMuMuCDCTRG->Print("EventT0_Algorithm_Efficiency.pdf");
    m_cT0FractionsMuMuTOPTRG->Print("EventT0_Algorithm_Efficiency.pdf)");
  }

}

void DQMHistAnalysisEventT0EfficiencyModule::endRun()
{

}

void DQMHistAnalysisEventT0EfficiencyModule::terminate()
{
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


bool DQMHistAnalysisEventT0EfficiencyModule::FillEfficiencyHistogram(const std::string& histname, TEfficiency* eff)
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

