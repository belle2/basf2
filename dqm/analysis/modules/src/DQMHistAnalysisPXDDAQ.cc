//+
// File : DQMHistAnalysisPXDDAQ.cc
// Description : Analysis of PXD Reduction
//
// Author : Bjoern Spruck, Univerisity Mainz
// Date : 2020
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
  B2DEBUG(1, "DQMHistAnalysisPXDDAQ: Constructor done.");
}

DQMHistAnalysisPXDDAQModule::~DQMHistAnalysisPXDDAQModule()
{
#ifdef _BELLE2_EPICS
  if (ca_current_context()) ca_context_destroy();
#endif
}

void DQMHistAnalysisPXDDAQModule::initialize()
{
  B2DEBUG(1, "DQMHistAnalysisPXDDAQ: initialized.");

  m_monObj = getMonitoringObject("pxd");

  gROOT->cd(); // this seems to be important, or strange things happen

  m_cMissingDHC = new TCanvas((m_histogramDirectoryName + "/c_MissingDHC").data());
  m_cMissingDHE = new TCanvas((m_histogramDirectoryName + "/c_MissingDHE").data());
  m_cMissingDHP = new TCanvas((m_histogramDirectoryName + "/c_MissingDHP").data());

  m_hMissingDHC = new TH2F("hPXDMissingDHC", "hPXDMissingDHC", 16, 0, 16, 2, 0, 2);
  m_hMissingDHE = new TH2F("hPXDMissingDHE", "hPXDMissingDHE", 64, 0, 64, 2, 0, 2);

#ifdef _BELLE2_EPICS
  if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
  SEVCHK(ca_create_channel((m_pvPrefix + "Status").data(), NULL, NULL, 10, &mychid), "ca_create_channel failure");
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
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
  if (m_cMissingDHP == nullptr || m_cMissingDHE == nullptr || m_cMissingDHC == nullptr) return; // we could assume this

  m_cMissingDHP->Clear();
  m_cMissingDHE->Clear();
  m_cMissingDHC->Clear();


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

// #ifdef _BELLE2_EPICS
//
//   SEVCHK(ca_put(DBR_DOUBLE, mychid, (void*)&data), "ca_set failure");
//   SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
// #endif
}

void DQMHistAnalysisPXDDAQModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDDAQ: terminate called");
  // should delete canvas here, maybe hist, too? Who owns it?
}

