//+
// File : DQMHistInjection.cc
// Description : DQM module, which gives histograms showing injecting occupancies
//
// Author : Uwe Gebauer
// based on work from B. Spruck
// Date : someday
//-


#include <dqm/analysis/modules/DQMHistInjection.h>
#include <TROOT.h>
#include <TClass.h>
#include <TLatex.h>
#include <TGraphAsymmErrors.h>
#include <vxd/geometry/GeoCache.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistInjection)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistInjectionModule::DQMHistInjectionModule() : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

//   addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms were placed", std::string("PXDINJ"));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("DQM:INJ:"));
  B2DEBUG(1, "DQMHistInjection: Constructor done.");
}

DQMHistInjectionModule::~DQMHistInjectionModule()
{
#ifdef _BELLE2_EPICS
  if (ca_current_context()) ca_context_destroy();
#endif
}

void DQMHistInjectionModule::initialize()
{

  gROOT->cd(); // this seems to be important, or strange things happen

  m_cInjectionLERPXD = new TCanvas("PXDINJ/c_InjectionLERPXD");
  m_cInjectionLERPXDOcc = new TCanvas("PXDINJ/c_InjectionLERPXDOcc");
  m_cInjectionLERECL = new TCanvas("ECLINJ/c_InjectionLERECL");

  m_cInjectionHERPXD = new TCanvas("PXDINJ/c_InjectionHERPXD");
  m_cInjectionHERPXDOcc = new TCanvas("PXDINJ/c_InjectionHERPXDOcc");
  m_cInjectionHERECL = new TCanvas("ECLINJ/c_InjectionHERECL");

  m_hInjectionLERPXD = new TH1F("HitInjectionLERPXD", "Mean Hits/event;Time in #mu s;", 4000, 0 , 20000);
  m_hInjectionLERPXDOcc = new TH1F("HitInjectionPXDLEROcc", "Mean Occ in % per module;Time in #mu s;", 4000, 0 , 20000);
  m_hInjectionLERECL = new TH1F("HitInjectionLERECL", "Mean Hits/event;Time in #mu s;", 4000, 0 , 20000);

  m_hInjectionHERPXD = new TH1F("HitInjectionHERPXD", "Mean Hits/event;Time in #mu s;", 4000, 0 , 20000);
  m_hInjectionHERPXDOcc = new TH1F("HitInjectionPXDHEROcc", "Mean Occ in % per modul;Time in #mu s;", 4000, 0 , 20000);
  m_hInjectionHERECL = new TH1F("HitInjectionHERECL", "Mean Hits/event;Time in #mu s;", 4000, 0 , 20000);

#ifdef _BELLE2_EPICS
  if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
  m_nodes.resize(6);
  SEVCHK(ca_create_channel((m_pvPrefix + "LER:Triggers").data(), NULL, NULL, 10, &m_nodes[0].mychid), "ca_create_channel failure");
  m_nodes[0].histo = nullptr;
  SEVCHK(ca_create_channel((m_pvPrefix + "LER:PXD").data(), NULL, NULL, 10, &m_nodes[1].mychid), "ca_create_channel failure");
  m_nodes[1].histo = m_hInjectionLERPXD;
  SEVCHK(ca_create_channel((m_pvPrefix + "LER:ECL").data(), NULL, NULL, 10, &m_nodes[2].mychid), "ca_create_channel failure");
  m_nodes[2].histo = m_hInjectionLERECL;
  SEVCHK(ca_create_channel((m_pvPrefix + "HER:Triggers").data(), NULL, NULL, 10, &m_nodes[3].mychid), "ca_create_channel failure");
  m_nodes[3].histo = nullptr;
  SEVCHK(ca_create_channel((m_pvPrefix + "HER:PXD").data(), NULL, NULL, 10, &m_nodes[4].mychid), "ca_create_channel failure");
  m_nodes[4].histo = m_hInjectionHERPXD;
  SEVCHK(ca_create_channel((m_pvPrefix + "HER:ECL").data(), NULL, NULL, 10, &m_nodes[5].mychid), "ca_create_channel failure");
  m_nodes[5].histo = m_hInjectionHERECL;

  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  cleanPVs();
#endif
  B2DEBUG(1, "DQMHistInjection: initialized.");
}


void DQMHistInjectionModule::beginRun()
{
  B2DEBUG(1, "DQMHistInjection: beginRun called.");

  m_cInjectionLERPXD->Clear();
  m_cInjectionLERPXDOcc->Clear();
  m_cInjectionLERECL->Clear();
  m_cInjectionHERPXD->Clear();
  m_cInjectionHERPXDOcc->Clear();
  m_cInjectionHERECL->Clear();

  cleanPVs();
}


void DQMHistInjectionModule::event()
{
  TH1* Hits = nullptr, *Triggers = nullptr;
  TString locationHits = "";
  TString locationTriggers = "";
  m_histogramDirectoryName = "PXDINJ";

  locationHits = "PXDOccInjLER";
  if (m_histogramDirectoryName != "") {
    locationHits = m_histogramDirectoryName + "/" + locationHits;
  }
  Hits = (TH1*)findHist(locationHits.Data());
  locationTriggers = "PXDEOccInjLER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  //Finding only one of them should only happen in very strange situations...
  m_nodes[0].histo = Triggers;
  if (Hits && Triggers) {
    m_hInjectionLERPXD->Divide(Hits, Triggers);
    m_hInjectionLERPXDOcc->Divide(Hits, Triggers, 100, 768 * 250); // to percent
  }

  m_cInjectionLERPXD->Clear();
  m_cInjectionLERPXD->cd(0);
  m_hInjectionLERPXD->Draw();

  m_cInjectionLERPXDOcc->Clear();
  m_cInjectionLERPXDOcc->cd(0);
  m_hInjectionLERPXDOcc->Draw();

  locationHits = "PXDOccInjHER";
  if (m_histogramDirectoryName != "") {
    locationHits = m_histogramDirectoryName + "/" + locationHits;
  }
  Hits = (TH1*)findHist(locationHits.Data());
  locationTriggers = "PXDEOccInjHER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  //Finding only one of them should only happen in very strange situations...
  m_nodes[3].histo = Triggers;
  if (Hits && Triggers) {
    m_hInjectionHERPXD->Divide(Hits, Triggers);
    m_hInjectionHERPXDOcc->Divide(Hits, Triggers, 100, 768 * 250); // to percent
  }

  m_cInjectionHERPXD->Clear();
  m_cInjectionHERPXD->cd(0);
  m_hInjectionHERPXD->Draw();

  m_cInjectionHERPXDOcc->Clear();
  m_cInjectionHERPXDOcc->cd(0);
  m_hInjectionHERPXDOcc->Draw();

  m_histogramDirectoryName = "ECLINJ";

  locationHits = "ECLOccInjLER";
  if (m_histogramDirectoryName != "") {
    locationHits = m_histogramDirectoryName + "/" + locationHits;
  }
  Hits = (TH1*)findHist(locationHits.Data());
  locationTriggers = "ECLEOccInjLER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  //Finding only one of them should only happen in very strange situations...
  if (Hits && Triggers) {
    m_hInjectionLERECL->Divide(Hits, Triggers);
  }

  m_cInjectionLERECL->Clear();
  m_cInjectionLERECL->cd(0);
  m_hInjectionLERECL->Draw();

  locationHits = "ECLOccInjHER";
  if (m_histogramDirectoryName != "") {
    locationHits = m_histogramDirectoryName + "/" + locationHits;
  }
  Hits = (TH1*)findHist(locationHits.Data());
  locationTriggers = "ECLEOccInjHER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  //Finding only one of them should only happen in very strange situations...
  if (Hits && Triggers) {
    m_hInjectionHERECL->Divide(Hits, Triggers);
  }

  m_cInjectionHERECL->Clear();
  m_cInjectionHERECL->cd(0);
  m_hInjectionHERECL->Draw();

#ifdef _BELLE2_EPICS
  for (auto& m : m_nodes) {
    if (!m.mychid) continue;
    int length = m.data.size();
    if (m.histo && m.histo->GetNcells() > 2 && length > 0  && length == int(ca_element_count(m.mychid))) {
      // If bin count doesnt match, we loose bins but otherwise ca_array_put will complain
      // We fill up the array with ZEROs otherwise
      if (m.histo->GetDimension() == 1) {
        int i = 0;
        int nx = m.histo->GetNbinsX() + 1;
        for (int x = 1; x < nx && i < length ; x++) {
          m.data[i++] = m.histo->GetBinContent(x);
        }

      } else if (m.histo->GetDimension() == 2) {
        int i = 0;
        int nx = m.histo->GetNbinsX() + 1;
        int ny = m.histo->GetNbinsY() + 1;
        for (int y = 1; y < ny && i < length; y++) {
          for (int x = 1; x < nx && i < length ; x++) {
            m.data[i++] = m.histo->GetBinContent(x, y);
          }
        }
      }
    }
    SEVCHK(ca_put(DBR_DOUBLE, m.mychid, (void*)m.data.data()), "ca_set failure");
  }
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
}

void DQMHistInjectionModule::cleanPVs(void)
{
#ifdef _BELLE2_EPICS
  for (auto m : m_nodes) {
    int length = int(ca_element_count(m.mychid));
    if (length > 0) {
      m.data.resize(length, 0.0);
      SEVCHK(ca_array_put(DBR_DOUBLE, length, m.mychid, (void*)m.data.data()), "ca_put failure");
    }
  }
#endif
}

void DQMHistInjectionModule::terminate()
{
  B2DEBUG(1, "DQMHistInjection: terminate called");
#ifdef _BELLE2_EPICS
  for (auto m : m_nodes) {
    SEVCHK(ca_clear_channel(m.mychid), "ca_clear_channel failure");
  }
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
}

