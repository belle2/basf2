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

  m_cMissingDHP = new TCanvas((m_histogramDirectoryName + "/c_MissingDHP").data());

  m_monObj->addCanvas(m_cMissingDHP);

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    SEVCHK(ca_create_channel((m_pvPrefix + "Status").data(), NULL, NULL, 10, &mychid), "ca_create_channel failure");
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
  if (!m_cMissingDHP) return;
  m_cMissingDHP->Clear();

  if (m_hMissingDHP) delete m_hMissingDHP;


  std::string name = "PXDDAQDHPDataMissing";

  TH1* hh1 = findHist(name);
  if (hh1 == NULL) {
    hh1 = findHist(m_histogramDirectoryName, name);
  }
  m_cMissingDHP->cd();
  if (hh1) {
    m_hMissingDHP = (TH1F*)hh1->DrawClone("text");
    m_hMissingDHP->Scale(1.0 / m_hMissingDHP->GetBinContent(0));
  }

  if (m_hMissingDHP) {
    m_hMissingDHP->Draw("text");
  }

//   double data = m_hMissingDHP->Max???;
//
//   m_monObj->setVariable("missingDHPFraction", data);
//
  m_cMissingDHP->Modified();
  m_cMissingDHP->Update();
// #ifdef _BELLE2_EPICS
//  if (m_useEpics) {
//   SEVCHK(ca_put(DBR_DOUBLE, mychid, (void*)&data), "ca_set failure");
//   SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
//  }
// #endif
}

void DQMHistAnalysisPXDDAQModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDDAQ: terminate called");
  // should delete canvas here, maybe hist, too? Who owns it?
}

