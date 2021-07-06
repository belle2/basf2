/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPXDDAQ.cc
// Description : Analysis of PXD Reduction
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDDAQ.h>
#include <TROOT.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDDAQ)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDDAQModule::DQMHistAnalysisPXDDAQModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  //Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("PXDDAQ"));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("DQM:PXD:DAQ:"));
  addParam("useEpics", m_useEpics, "useEpics", true);
  B2DEBUG(1, "DQMHistAnalysisPXDDAQ: Constructor done.");

}

DQMHistAnalysisPXDDAQModule::~DQMHistAnalysisPXDDAQModule()
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (ca_current_context()) ca_context_destroy();
  }
#endif
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

  m_monObj->addCanvas(m_cDAQError);
  m_monObj->addCanvas(m_cMissingDHC);
  m_monObj->addCanvas(m_cMissingDHE);
  m_monObj->addCanvas(m_cMissingDHP);
  m_monObj->addCanvas(m_cStatistic);

  m_hMissingDHC = new TH2F("hPXDMissingDHC", "PXD Missing DHC", 16, 0, 16, 2, 0, 2);
  m_hMissingDHE = new TH2F("hPXDMissingDHE", "PXD Missing DHE", 64, 0, 64, 2, 0, 2);

#ifdef _BELLE2_EPICS
  mychid.resize(20);
  if (m_useEpics) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    SEVCHK(ca_create_channel((m_pvPrefix + "HLTRej").data(), NULL, NULL, 10, &mychid[0]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "Trunc").data(), NULL, NULL, 10, &mychid[1]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "HER_Trunc").data(), NULL, NULL, 10, &mychid[2]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "LER_Trunc").data(), NULL, NULL, 10, &mychid[3]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "CM63").data(), NULL, NULL, 10, &mychid[4]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "HER_CM63").data(), NULL, NULL, 10, &mychid[5]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "LER_CM63").data(), NULL, NULL, 10, &mychid[6]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "HER_CM63_1ms").data(), NULL, NULL, 10, &mychid[7]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "LER_CM63_1ms").data(), NULL, NULL, 10, &mychid[8]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "HER_Trunc_1ms").data(), NULL, NULL, 10, &mychid[9]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "LER_Trunc_1ms").data(), NULL, NULL, 10, &mychid[10]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "MissFrame").data(), NULL, NULL, 10, &mychid[11]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "Timeout").data(), NULL, NULL, 10, &mychid[12]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "LinkDown").data(), NULL, NULL, 10, &mychid[13]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "Mismatch").data(), NULL, NULL, 10, &mychid[14]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "HER_Miss").data(), NULL, NULL, 10, &mychid[15]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "LER_Miss").data(), NULL, NULL, 10, &mychid[16]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "HER_Miss_1ms").data(), NULL, NULL, 10, &mychid[17]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "LER_Miss_1ms").data(), NULL, NULL, 10, &mychid[18]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "unused").data(), NULL, NULL, 10, &mychid[19]), "ca_create_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

void DQMHistAnalysisPXDDAQModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDDAQ: beginRun called.");

  m_cMissingDHP->Clear();
}

void DQMHistAnalysisPXDDAQModule::event()
{
//   double data = 0.0;
  if (m_cMissingDHP == nullptr || m_cMissingDHE == nullptr || m_cMissingDHC == nullptr
      || m_cStatistic == nullptr) return; // we could assume this

  m_cDAQError->Clear();
  m_cMissingDHP->Clear();
  m_cMissingDHE->Clear();
  m_cMissingDHC->Clear();
  m_cStatistic->Clear();


  {
    std::string name = "PXDDAQError";

    if (m_hDAQError) { delete m_hDAQError; m_hDAQError = nullptr;}

    TH1* hh1 = findHist(name);
    if (hh1 == NULL) {
      hh1 = findHist(m_histogramDirectoryName, name);
    }
    m_cDAQError->cd();
    if (hh1) {
      m_hDAQError = (TH1F*)hh1->DrawClone("text");
      m_hDAQError->SetName("hPXDDAQError");
      m_hDAQError->SetTitle("PXD Fraction of DAQ Errors");
      if (m_hDAQError->GetBinContent(0)) {
        m_hDAQError->Scale(1.0 / m_hDAQError->GetBinContent(0));
      }
      m_hDAQError->Draw("text,hist");
    }
  }
  {
    // DHC histogram
    std::string name = "PXDDAQDHCError";

    TH1* hh1 = findHist(name);
    if (hh1 == NULL) {
      hh1 = findHist(m_histogramDirectoryName, name);
    }
    m_cMissingDHC->cd();
    if (hh1) {
      auto events = hh1->GetBinContent(hh1->GetBin(-1, -1));
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
    }
  }

  {
    // DHE histogram
    std::string name = "PXDDAQDHEError";

    TH1* hh1 = findHist(name);
    if (hh1 == NULL) {
      hh1 = findHist(m_histogramDirectoryName, name);
    }
    m_cMissingDHE->cd();
    if (hh1) {
      auto events = hh1->GetBinContent(hh1->GetBin(-1, -1));
      // first, we have to relate the per-DHE overflow (DHE object count) to the overall overflow (event count)
      // second, we have to relate the "fake data" DHE bin to the per-DHE overflow (DHE object count)
      m_hMissingDHE->Reset();
      for (int i = 0; i < 64; i++) {
        auto dhecount = hh1->GetBinContent(hh1->GetBin(i, -1));
        if (events > 0) m_hMissingDHE->Fill((double)i, 0.0, 1.0 - dhecount / events);
        // c_FAKE_NO_DATA_TRIG = 1ull << 29,
        if (dhecount > 0) m_hMissingDHE->Fill((double)i, 1.0, hh1->GetBinContent(hh1->GetBin(i, 29) / dhecount));
      }
      m_hMissingDHE->Draw("text");
    }
  }

  {
    // DHP histogram
    if (m_hMissingDHP) { delete m_hMissingDHP; m_hMissingDHP = nullptr;}

    std::string name = "PXDDAQDHPDataMissing";

    TH1* hh1 = findHist(name);
    if (hh1 == NULL) {
      hh1 = findHist(m_histogramDirectoryName, name);
    }
    m_cMissingDHP->cd();
    if (hh1) {
      m_hMissingDHP = (TH1F*)hh1->DrawClone("text");
      if (m_hMissingDHP->GetBinContent(0)) {
        m_hMissingDHP->Scale(1.0 / m_hMissingDHP->GetBinContent(0));
        m_hMissingDHP->Draw("text");
      }
    }
    //   double data = m_hMissingDHP->Max???;
    //
    //   m_monObj->setVariable("missingDHPFraction", data);
    //
    m_cMissingDHP->Modified();
    m_cMissingDHP->Update();
  }

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

  // Stat histogram
  if (m_hStatistic) { delete m_hStatistic; m_hStatistic = nullptr;}

  std::string name = "PXDDAQStat";

  TH1* hh1 = findHist(name);
  if (hh1 == NULL) {
    hh1 = findHist(m_histogramDirectoryName, name);
  }
  m_cStatistic->cd();
  if (hh1) {
    m_hStatistic = (TH1F*)hh1->DrawClone("text");
    if (m_hStatistic->GetBinContent(0)) {
      m_hStatistic->Scale(1.0 / m_hStatistic->GetBinContent(0));
      m_hStatistic->Draw("text");
    }
    double scale = hh1->GetBinContent(0);// underflow is event counter
    if (scale != 0.0) scale = 1.0 / scale; // just avoid dive by zero
    data_HLTRej = hh1->GetBinContent(1 + 0) * scale;
    data_Trunc = hh1->GetBinContent(1 + 1) * scale;
    data_HER_Trunc = hh1->GetBinContent(1 + 2) * scale;
    data_LER_Trunc = hh1->GetBinContent(1 + 3) * scale;
    data_CM63 = hh1->GetBinContent(1 + 4) * scale;
    data_HER_CM63 = hh1->GetBinContent(1 + 5) * scale;
    data_LER_CM63 = hh1->GetBinContent(1 + 6) * scale;
    data_HER_CM63_1ms = hh1->GetBinContent(1 + 7) * scale;
    data_LER_CM63_1ms = hh1->GetBinContent(1 + 8) * scale;
    data_HER_Trunc_1ms = hh1->GetBinContent(1 + 9) * scale;
    data_LER_Trunc_1ms = hh1->GetBinContent(1 + 10) * scale;
    data_MissFrame = hh1->GetBinContent(1 + 11) * scale;
    data_Timeout = hh1->GetBinContent(1 + 12) * scale;
    data_LinkDown = hh1->GetBinContent(1 + 13) * scale;
    data_Mismatch = hh1->GetBinContent(1 + 14) * scale;
    data_HER_Miss = hh1->GetBinContent(1 + 15) * scale;
    data_LER_Miss = hh1->GetBinContent(1 + 16) * scale;
    data_HER_Miss_1ms = hh1->GetBinContent(1 + 17) * scale;
    data_LER_Miss_1ms = hh1->GetBinContent(1 + 18) * scale;
    data_unused = hh1->GetBinContent(1 + 19) * scale;
  }

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

  m_cStatistic->Modified();
  m_cStatistic->Update();

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    SEVCHK(ca_put(DBR_DOUBLE, mychid[0], (void*)&data_HLTRej), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[1], (void*)&data_Trunc), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[2], (void*)&data_HER_Trunc), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[3], (void*)&data_LER_Trunc), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[4], (void*)&data_CM63), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[5], (void*)&data_HER_CM63), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[6], (void*)&data_LER_CM63), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[7], (void*)&data_HER_CM63_1ms), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[8], (void*)&data_LER_CM63_1ms), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[9], (void*)&data_HER_Trunc_1ms), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[10], (void*)&data_LER_Trunc_1ms), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[11], (void*)&data_MissFrame), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[12], (void*)&data_Timeout), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[13], (void*)&data_LinkDown), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[14], (void*)&data_Mismatch), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[15], (void*)&data_HER_Miss), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[16], (void*)&data_LER_Miss), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[17], (void*)&data_HER_Miss_1ms), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[18], (void*)&data_LER_Miss_1ms), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[19], (void*)&data_unused), "ca_set failure");
    // write out
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

void DQMHistAnalysisPXDDAQModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDDAQ: terminate called");
  // should delete canvas here, maybe hist, too? Who owns it?
}

