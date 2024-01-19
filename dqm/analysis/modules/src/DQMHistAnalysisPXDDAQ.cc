/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPXDDAQ.cc
// Description : Analysis of PXD DAQ and Issues
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDDAQ.h>
#include <TROOT.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDDAQ);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDDAQModule::DQMHistAnalysisPXDDAQModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!
  setDescription("DQM Analysis for PXD DAQ Statistics and Issues");

  // Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("PXDDAQ"));
  addParam("minEntries", m_minEntries, "minimum number of new entries for last time slot", 10000);
  B2DEBUG(1, "DQMHistAnalysisPXDDAQ: Constructor done.");

}

DQMHistAnalysisPXDDAQModule::~DQMHistAnalysisPXDDAQModule()
{
}

void DQMHistAnalysisPXDDAQModule::initialize()
{
  B2DEBUG(1, "DQMHistAnalysisPXDDAQ: initialized.");

  m_monObj = getMonitoringObject("pxd");

  gROOT->cd(); // this seems to be important, or strange things happen

  m_cDAQError = new TCanvas((m_histogramDirectoryName + "/c_DAQError").data());
  m_cMissingDHC = new TCanvas((m_histogramDirectoryName + "/c_MissingDHC").data());
  m_cMissingDHE = new TCanvas((m_histogramDirectoryName + "/c_MissingDHE").data());
  m_cMissingDHP = new TCanvas((m_histogramDirectoryName + "/c_MissingDHP").data());
  m_cStatistic = new TCanvas((m_histogramDirectoryName + "/c_Statistic").data());
  m_cStatisticUpd = new TCanvas((m_histogramDirectoryName + "/c_StatisticUpd").data());
  if (!hasDeltaPar(m_histogramDirectoryName, "PXDDAQStat")) addDeltaPar(m_histogramDirectoryName, "PXDDAQStat",
        HistDelta::c_Underflow, m_minEntries, 1); // register delta

  m_monObj->addCanvas(m_cDAQError);
  m_monObj->addCanvas(m_cMissingDHC);
  m_monObj->addCanvas(m_cMissingDHE);
  m_monObj->addCanvas(m_cMissingDHP);
  m_monObj->addCanvas(m_cStatistic);

  m_hMissingDHC = new TH2F("hPXDMissingDHC", "PXD Missing DHC", 16, 0, 16, 2, 0, 2);
  m_hMissingDHE = new TH2F("hPXDMissingDHE", "PXD Missing DHE", 64, 0, 64, 2, 0, 2);

  registerEpicsPV("PXD:DAQ:HLTRej", "HLTReject");
  registerEpicsPV("PXD:DAQ:Trunc", "Trunc");
  registerEpicsPV("PXD:DAQ:HER_Trunc", "HER_Trunc");
  registerEpicsPV("PXD:DAQ:LER_Trunc", "LER_Trunc");
  registerEpicsPV("PXD:DAQ:CM63", "CM63");
  registerEpicsPV("PXD:DAQ:HER_CM63", "HER_CM63");
  registerEpicsPV("PXD:DAQ:LER_CM63", "LER_CM63");
  registerEpicsPV("PXD:DAQ:HER_CM63_1ms", "HER_CM63_1ms");
  registerEpicsPV("PXD:DAQ:LER_CM63_1ms", "LER_CM63_1ms");
  registerEpicsPV("PXD:DAQ:HER_Trunc_1ms", "HER_Trunc_1ms");
  registerEpicsPV("PXD:DAQ:LER_Trunc_1ms", "LER_Trunc_1ms");
  registerEpicsPV("PXD:DAQ:MissFrame", "MissFrame");
  registerEpicsPV("PXD:DAQ:Timeout", "Timeout");
  registerEpicsPV("PXD:DAQ:LinkDown", "LinkDown");
  registerEpicsPV("PXD:DAQ:Mismatch", "Mismatch");
  registerEpicsPV("PXD:DAQ:HER_Miss", "HER_Miss");
  registerEpicsPV("PXD:DAQ:LER_Miss", "LER_Miss");
  registerEpicsPV("PXD:DAQ:HER_Miss_1ms", "HER_Miss_1ms");
  registerEpicsPV("PXD:DAQ:LER_Miss_1ms", "LER_Miss_1ms");
  registerEpicsPV("PXD:DAQ:unused", "unused");
}

void DQMHistAnalysisPXDDAQModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDDAQ: beginRun called.");

  m_cMissingDHP->Clear();
  m_cMissingDHE->Clear();
  m_cMissingDHC->Clear();
  m_cStatistic->Clear();
  m_cStatisticUpd->Clear();
}

void DQMHistAnalysisPXDDAQModule::event()
{
//   double data = 0.0;
  if (m_cMissingDHP == nullptr || m_cMissingDHE == nullptr || m_cMissingDHC == nullptr
      || m_cStatistic == nullptr) return; // we could assume this

  {
    std::string name = "PXDDAQError";

//    if (m_hDAQError) { delete m_hDAQError; m_hDAQError = nullptr;}

    auto hh1 = findHist(m_histogramDirectoryName, name, true);
    if (hh1) {
      m_cDAQError->Clear();
      m_cDAQError->cd();
      m_hDAQError = (TH1D*)hh1->DrawClone("text");
      m_hDAQError->SetName("hPXDDAQError");
      m_hDAQError->SetTitle("PXD Fraction of DAQ Errors");
      if (m_hDAQError->GetBinContent(0)) {
        m_hDAQError->Scale(1.0 / m_hDAQError->GetBinContent(0));
      }
      m_hDAQError->Draw("text,hist");
      UpdateCanvas(m_cDAQError, true);
    }
  }
  {
    // DHC histogram
    std::string name = "PXDDAQDHCError";

    auto hh1 = findHist(m_histogramDirectoryName, name, true);
    if (hh1) {
      auto events = hh1->GetBinContent(hh1->GetBin(-1, -1));
      m_cMissingDHC->Clear();
      m_cMissingDHC->cd();
      // first, we have to relate the per-DHC overflow (DHC object count) to the overall overflow (event count)
      // second, we have to relate the "fake data" DHC bin to the per-DHC overflow (DHC object count)
      m_hMissingDHC->Reset();
      for (int i = 0; i < 16; i++) {
        auto dhecount = hh1->GetBinContent(hh1->GetBin(i, -1));
        if (events > 0) m_hMissingDHC->Fill((double)i, 0.0, 1.0 - dhecount / events);
        // c_FAKE_NO_DATA_TRIG = 1ull << 29,
        if (dhecount > 0) m_hMissingDHC->Fill((double)i, 1.0, hh1->GetBinContent(hh1->GetBin(i, 29) / dhecount));
      }
      m_hMissingDHC->Draw("text");
      UpdateCanvas(m_cMissingDHC, true);
    }
  }

  {
    // DHE histogram
    std::string name = "PXDDAQDHEError";

    auto hh1 = findHist(m_histogramDirectoryName, name, true);
    if (hh1) {
      auto events = hh1->GetBinContent(hh1->GetBin(-1, -1));
      // first, we have to relate the per-DHE overflow (DHE object count) to the overall overflow (event count)
      // second, we have to relate the "fake data" DHE bin to the per-DHE overflow (DHE object count)
      m_cMissingDHE->Clear();
      m_cMissingDHE->cd();
      m_hMissingDHE->Reset();
      for (int i = 0; i < 64; i++) {
        auto dhecount = hh1->GetBinContent(hh1->GetBin(i, -1));
        if (events > 0) m_hMissingDHE->Fill((double)i, 0.0, 1.0 - dhecount / events);
        // c_FAKE_NO_DATA_TRIG = 1ull << 29,
        if (dhecount > 0) m_hMissingDHE->Fill((double)i, 1.0, hh1->GetBinContent(hh1->GetBin(i, 29) / dhecount));
      }
      m_hMissingDHE->Draw("text");
      UpdateCanvas(m_cMissingDHE, true);
    }
  }

  {
    // DHP histogram
    //if (m_hMissingDHP) { delete m_hMissingDHP; m_hMissingDHP = nullptr;}

    std::string name = "PXDDAQDHPDataMissing";

    auto hh1 = findHist(m_histogramDirectoryName, name, true);
    if (hh1) {
      m_cMissingDHP->Clear();

      m_cMissingDHP->cd();
      m_hMissingDHP = (TH1F*)hh1->DrawClone("text");
      if (m_hMissingDHP->GetBinContent(0)) {
        m_hMissingDHP->Scale(1.0 / m_hMissingDHP->GetBinContent(0));
        m_hMissingDHP->Draw("text");
      }
      m_cMissingDHP->Modified();
      m_cMissingDHP->Update();
      UpdateCanvas(m_cMissingDHP, true);
    }
    //   double data = m_hMissingDHP->Max???;
    //
    //   m_monObj->setVariable("missingDHPFraction", data);
    //
  }

  std::string name = "PXDDAQStat";

  auto* statsum = findHist(m_histogramDirectoryName, name, true);
  if (statsum) {
    // Stat histogram
    //if (m_hStatistic) { delete m_hStatistic; m_hStatistic = nullptr;}
    m_cStatistic->cd();
    m_hStatistic = (TH1D*)statsum->DrawClone("text");
    if (m_hStatistic->GetBinContent(0)) {
      m_hStatistic->Scale(1.0 / m_hStatistic->GetBinContent(0));
      m_hStatistic->Draw("text");
    }
    m_cStatistic->Modified();
    m_cStatistic->Update();
    UpdateCanvas(m_cStatistic, true);
  }

  // now the important part, check fraction of "errors" and export
  double data_HLTRej = 0.0;
  double data_Trunc = 0.0;
  double data_HER_Trunc = 0.0;
  double data_LER_Trunc = 0.0;
  double data_CM63 = 0.0;
  double data_HER_CM63 = 0.0;
  double data_LER_CM63 = 0.0;
  double data_HER_CM63_1ms = 0.0;
  double data_LER_CM63_1ms = 0.0;
  double data_HER_Trunc_1ms = 0.0;
  double data_LER_Trunc_1ms = 0.0;
  double data_MissFrame = 0.0;
  double data_Timeout = 0.0;
  double data_LinkDown = 0.0;
  double data_Mismatch = 0.0;
  double data_HER_Miss = 0.0;
  double data_LER_Miss = 0.0;
  double data_HER_Miss_1ms = 0.0;
  double data_LER_Miss_1ms = 0.0;
  double data_unused = 0.0;

  bool update_epics = false;

  auto* delta = getDelta(m_histogramDirectoryName, name); // only updated by default
  if (delta) {
    gROOT->cd();
    m_cStatisticUpd->Clear();
    m_cStatisticUpd->cd();// necessary!
    delta->Draw("hist");
    // now check that we have enough stats for useful export
    double scale = delta->GetBinContent(0);// underflow is event counter
    if (scale >= m_minEntries) {
      if (scale != 0.0) scale = 1.0 / scale; // just avoid divide by zero, should never happen
      data_HLTRej = delta->GetBinContent(1 + 0) * scale;
      data_Trunc = delta->GetBinContent(1 + 1) * scale;
      data_HER_Trunc = delta->GetBinContent(1 + 2) * scale;
      data_LER_Trunc = delta->GetBinContent(1 + 3) * scale;
      data_CM63 = delta->GetBinContent(1 + 4) * scale;
      data_HER_CM63 = delta->GetBinContent(1 + 5) * scale;
      data_LER_CM63 = delta->GetBinContent(1 + 6) * scale;
      data_HER_CM63_1ms = delta->GetBinContent(1 + 7) * scale;
      data_LER_CM63_1ms = delta->GetBinContent(1 + 8) * scale;
      data_HER_Trunc_1ms = delta->GetBinContent(1 + 9) * scale;
      data_LER_Trunc_1ms = delta->GetBinContent(1 + 10) * scale;
      data_MissFrame = delta->GetBinContent(1 + 11) * scale;
      data_Timeout = delta->GetBinContent(1 + 12) * scale;
      data_LinkDown = delta->GetBinContent(1 + 13) * scale;
      data_Mismatch = delta->GetBinContent(1 + 14) * scale;
      data_HER_Miss = delta->GetBinContent(1 + 15) * scale;
      data_LER_Miss = delta->GetBinContent(1 + 16) * scale;
      data_HER_Miss_1ms = delta->GetBinContent(1 + 17) * scale;
      data_LER_Miss_1ms = delta->GetBinContent(1 + 18) * scale;
      data_unused = delta->GetBinContent(1 + 19) * scale;
      update_epics = true;
    }
    m_cStatisticUpd->Modified();
    m_cStatisticUpd->Update();
  }

  if (update_epics) {
    m_monObj->setVariable("HLTReject", data_HLTRej);
    m_monObj->setVariable("Trunc", data_Trunc);
    m_monObj->setVariable("HER_Trunc",      data_HER_Trunc);
    m_monObj->setVariable("LER_Trunc",      data_LER_Trunc);
    m_monObj->setVariable("CM63", data_CM63);
    m_monObj->setVariable("HER_CM63", data_HER_CM63);
    m_monObj->setVariable("LER_CM63", data_LER_CM63);
    m_monObj->setVariable("HER_CM63_1ms", data_HER_CM63_1ms);
    m_monObj->setVariable("LER_CM63_1ms", data_LER_CM63_1ms);
    m_monObj->setVariable("HER_Trunc_1ms", data_HER_Trunc_1ms);
    m_monObj->setVariable("LER_Trunc_1ms", data_LER_Trunc_1ms);
    m_monObj->setVariable("MissFrame", data_MissFrame);
    m_monObj->setVariable("Timeout", data_Timeout);
    m_monObj->setVariable("LinkDown", data_LinkDown);
    m_monObj->setVariable("Mismatch", data_Mismatch);
    m_monObj->setVariable("HER_Miss", data_HER_Miss);
    m_monObj->setVariable("LER_Miss", data_LER_Miss);
    m_monObj->setVariable("HER_Miss_1ms", data_HER_Miss_1ms);
    m_monObj->setVariable("LER_Miss_1ms", data_LER_Miss_1ms);

    setEpicsPV("HLTReject", data_HLTRej);
    setEpicsPV("Trunc", data_Trunc);
    setEpicsPV("HER_Trunc", data_HER_Trunc);
    setEpicsPV("LER_Trunc", data_LER_Trunc);
    setEpicsPV("CM63", data_CM63);
    setEpicsPV("HER_CM63", data_HER_CM63);
    setEpicsPV("LER_CM63", data_LER_CM63);
    setEpicsPV("HER_CM63_1ms", data_HER_CM63_1ms);
    setEpicsPV("LER_CM63_1ms", data_LER_CM63_1ms);
    setEpicsPV("HER_Trunc_1ms", data_HER_Trunc_1ms);
    setEpicsPV("LER_Trunc_1ms", data_LER_Trunc_1ms);
    setEpicsPV("MissFrame", data_MissFrame);
    setEpicsPV("Timeout", data_Timeout);
    setEpicsPV("LinkDown", data_LinkDown);
    setEpicsPV("Mismatch", data_Mismatch);
    setEpicsPV("HER_Miss", data_HER_Miss);
    setEpicsPV("LER_Miss", data_LER_Miss);
    setEpicsPV("HER_Miss_1ms", data_HER_Miss_1ms);
    setEpicsPV("LER_Miss_1ms", data_LER_Miss_1ms);
    setEpicsPV("unused", data_unused);
  }
}
