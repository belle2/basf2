//+
// File : DQMHistInjection.cc
// Description : DQM module, which gives histograms showing occupancies after injection
//
// Author : B. Spruck
// Date : someday
//-


#include <dqm/analysis/modules/DQMHistInjection.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <TROOT.h>

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
  addParam("useEpics", m_useEpics, "useEpics", true);
  B2DEBUG(1, "DQMHistInjection: Constructor done.");
}

DQMHistInjectionModule::~DQMHistInjectionModule()
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (ca_current_context()) ca_context_destroy();
  }
#endif
}

void DQMHistInjectionModule::initialize()
{

  gROOT->cd(); // this seems to be important, or strange things happen

  m_cInjectionLERPXD = new TCanvas("PXDINJ/c_InjectionLERPXD");
  m_cInjectionLERPXDOcc = new TCanvas("PXDINJ/c_InjectionLERPXDOcc");
  m_cInjectionLERSVD = new TCanvas("SVDInjection/c_InjectionLERSVD");
  m_cInjectionLERSVDOcc = new TCanvas("SVDInjection/c_InjectionLERSVDOcc");
  m_cInjectionLERECL = new TCanvas("ECLINJ/c_InjectionLERECL");
  m_cBurstLERECL = new TCanvas("ECLINJ/c_BurstInjectionLERECL");
  m_cInjectionLERTOP = new TCanvas("TOP/c_InjectionLERTOP");
  m_cInjectionLERARICH = new TCanvas("ARICH/c_InjectionLERARICH");
  m_cInjectionLERKLM = new TCanvas("KLM/c_InjectionLERKLM");

  m_cInjectionHERPXD = new TCanvas("PXDINJ/c_InjectionHERPXD");
  m_cInjectionHERPXDOcc = new TCanvas("PXDINJ/c_InjectionHERPXDOcc");
  m_cInjectionHERSVD = new TCanvas("SVDInjection/c_InjectionHERSVD");
  m_cInjectionHERSVDOcc = new TCanvas("SVDInjection/c_InjectionHERSVDOcc");
  m_cInjectionHERECL = new TCanvas("ECLINJ/c_InjectionHERECL");
  m_cBurstHERECL = new TCanvas("ECLINJ/c_BurstInjectionHERECL");
  m_cInjectionHERTOP = new TCanvas("TOP/c_InjectionHERTOP");
  m_cInjectionHERARICH = new TCanvas("ARICH/c_InjectionHERARICH");
  m_cInjectionHERKLM = new TCanvas("KLM/c_InjectionHERKLM");

  m_hInjectionLERPXD = new TH1F("HitInjectionLERPXD", "PXD Hits after LER Injection;Time in #mus;Mean Hits/event", 4000, 0 , 20000);
  m_hInjectionLERPXDOcc = new TH1F("HitInjectionPXDLEROcc", "PXD Occ after LER Injection;Time in #mus;Mean Occ in % per module", 4000,
                                   0 , 20000);
  m_hInjectionLERSVD = new TH1F("HitInjectionLERSVD", "SVD Hits after LER Injection;Time in #mus;Mean Hits/event", 4000, 0 , 20000);
  m_hInjectionLERSVDOcc = new TH1F("HitInjectionSVDLEROcc", "SVD Occ after LER Injection;Time in #mus;Mean Occ in % per module", 4000,
                                   0 , 20000);
  m_hInjectionLERECL = new TH1F("HitInjectionLERECL", "ECL Hits after LER Injection;Time in #mus;Mean Hits/event", 4000, 0 , 20000);
  m_hBurstLERECL = new TH1F("BurstInjectionLERECL", "ECL Bursts after LER Injection;Time in #mus;Suppressions/event (1 #mus bins)",
                            20000, 0 , 20000);
  m_hInjectionLERTOP = new TH1F("HitInjectionLERTOP", "TOP Occ after LER Injection;Time in #mus;Mean Occ in % /event", 4000, 0 ,
                                20000);
  m_hInjectionLERARICH = new TH1F("HitInjectionLERARICH", "ARICH Occ after LER Injection;Time in #mus;Mean Hits/event", 4000, 0 ,
                                  20000);
  m_hInjectionLERKLM = new TH1F("HitInjectionLERKLM",
                                "KLM occupancy after LER Injection;Time [#mus];Digits occupancy in % / (5 #mus)", 4000, 0 ,
                                20000);

  m_hInjectionHERPXD = new TH1F("HitInjectionHERPXD", "PXD Hits after HER Injection;Time in #mus;Mean Hits/event", 4000, 0 , 20000);
  m_hInjectionHERPXDOcc = new TH1F("HitInjectionPXDHEROcc", "PXD Occ after HER Injection;Time in #mus;Mean Occ in % per modul", 4000,
                                   0 , 20000);
  m_hInjectionHERSVD = new TH1F("HitInjectionHERSVD", "SVD Hits after HER Injection;Time in #mus;Mean Hits/event", 4000, 0 , 20000);
  m_hInjectionHERSVDOcc = new TH1F("HitInjectionSVDHEROcc", "SVD Occ after HER Injection;Time in #mus;Mean Occ in % per modul", 4000,
                                   0 , 20000);
  m_hInjectionHERECL = new TH1F("HitInjectionHERECL", "ECL Hits after HER Injection;Time in #mus;Mean Hits/event", 4000, 0 , 20000);
  m_hBurstHERECL = new TH1F("BurstInjectionHERECL", "ECL Bursts after HER Injection;Time in #mus;Suppressions/event (1 #mus bins)",
                            20000, 0 , 20000);
  m_hInjectionHERTOP = new TH1F("HitInjectionHERTOP", "TOP Occ after HER Injection;Time in #mus;Mean Occ in % /event", 4000, 0 ,
                                20000);
  m_hInjectionHERARICH = new TH1F("HitInjectionHERARICH", "ARICH Occ after HER Injection;Time in #mus;Mean Hits/event", 4000, 0 ,
                                  20000);
  m_hInjectionHERKLM = new TH1F("HitInjectionHERKLM",
                                "KLM occupancy after HER Injection;Time [#mus];Digits occupancy in % / (5 #mus)", 4000, 0 ,
                                20000);

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    m_nodes.resize(14);
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
    SEVCHK(ca_create_channel((m_pvPrefix + "LER:TOP").data(), NULL, NULL, 10, &m_nodes[6].mychid), "ca_create_channel failure");
    m_nodes[6].histo = m_hInjectionLERTOP;
    SEVCHK(ca_create_channel((m_pvPrefix + "HER:TOP").data(), NULL, NULL, 10, &m_nodes[7].mychid), "ca_create_channel failure");
    m_nodes[7].histo = m_hInjectionHERTOP;
    SEVCHK(ca_create_channel((m_pvPrefix + "LER:SVD").data(), NULL, NULL, 10, &m_nodes[8].mychid), "ca_create_channel failure");
    m_nodes[8].histo = m_hInjectionLERSVD;
    SEVCHK(ca_create_channel((m_pvPrefix + "HER:SVD").data(), NULL, NULL, 10, &m_nodes[9].mychid), "ca_create_channel failure");
    m_nodes[9].histo = m_hInjectionHERSVD;
    SEVCHK(ca_create_channel((m_pvPrefix + "LER:ARICH").data(), NULL, NULL, 10, &m_nodes[10].mychid), "ca_create_channel failure");
    m_nodes[10].histo = m_hInjectionLERARICH;
    SEVCHK(ca_create_channel((m_pvPrefix + "HER:ARICH").data(), NULL, NULL, 10, &m_nodes[11].mychid), "ca_create_channel failure");
    m_nodes[11].histo = m_hInjectionHERARICH;
    SEVCHK(ca_create_channel((m_pvPrefix + "LER:KLM").data(), NULL, NULL, 10, &m_nodes[10].mychid), "ca_create_channel failure");
    m_nodes[12].histo = m_hInjectionLERKLM;
    SEVCHK(ca_create_channel((m_pvPrefix + "HER:KLM").data(), NULL, NULL, 10, &m_nodes[11].mychid), "ca_create_channel failure");
    m_nodes[13].histo = m_hInjectionHERKLM;

    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
    cleanPVs();
  }
#endif
  B2DEBUG(1, "DQMHistInjection: initialized.");
}


void DQMHistInjectionModule::beginRun()
{
  B2DEBUG(1, "DQMHistInjection: beginRun called.");

//   m_cInjectionLERPXD->Clear(); // FIXME, unclear if this lets to crashes on new run?
//   m_cInjectionLERPXDOcc->Clear();
//   m_cInjectionLERECL->Clear();
//   m_cInjectionHERPXD->Clear();
//   m_cInjectionHERPXDOcc->Clear();
//   m_cInjectionHERECL->Clear();

//   cleanPVs();
}


void DQMHistInjectionModule::event()
{
  TH1* Hits = nullptr, *Triggers = nullptr;
  TString locationHits = "";
  TString locationTriggers = "";
  //PXD
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
  //m_nodes[0].histo = Triggers;
  if (Hits && Triggers) {
    m_hInjectionLERPXD->Divide(Hits, Triggers);
    m_hInjectionLERPXDOcc->Divide(Hits, Triggers, 100, 768 * 250); // to percent
  }

  m_cInjectionLERPXD->Clear();
  m_cInjectionLERPXD->cd(0);
  m_hInjectionLERPXD->Draw("hist");

  m_cInjectionLERPXDOcc->Clear();
  m_cInjectionLERPXDOcc->cd(0);
  m_hInjectionLERPXDOcc->Draw("hist");

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
  //m_nodes[3].histo = Triggers;
  if (Hits && Triggers) {
    m_hInjectionHERPXD->Divide(Hits, Triggers);
    m_hInjectionHERPXDOcc->Divide(Hits, Triggers, 100, 768 * 250); // to percent
  }

  m_cInjectionHERPXD->Clear();
  m_cInjectionHERPXD->cd(0);
  m_hInjectionHERPXD->Draw("hist");

  m_cInjectionHERPXDOcc->Clear();
  m_cInjectionHERPXDOcc->cd(0);
  m_hInjectionHERPXDOcc->Draw("hist");

  //SVD
  m_histogramDirectoryName = "SVDInjection";

  locationHits = "SVDOccInjLER";
  if (m_histogramDirectoryName != "") {
    locationHits = m_histogramDirectoryName + "/" + locationHits;
  }
  Hits = (TH1*)findHist(locationHits.Data());
  locationTriggers = "SVDTrgOccInjLER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  //Finding only one of them should only happen in very strange situations...
  //m_nodes[0].histo = Triggers;
  if (Hits && Triggers) {
    m_hInjectionLERSVD->Divide(Hits, Triggers);
    m_hInjectionLERSVDOcc->Divide(Hits, Triggers, 100, 768 * 7 * 2); // to percent (L3V has 768 strips * 2 * 7 sides)
  }

  m_cInjectionLERSVD->Clear();
  m_cInjectionLERSVD->cd(0);
  m_hInjectionLERSVD->Draw("hist");

  m_cInjectionLERSVDOcc->Clear();
  m_cInjectionLERSVDOcc->cd(0);
  m_hInjectionLERSVDOcc->Draw("hist");

  locationHits = "SVDOccInjHER";
  if (m_histogramDirectoryName != "") {
    locationHits = m_histogramDirectoryName + "/" + locationHits;
  }
  Hits = (TH1*)findHist(locationHits.Data());
  locationTriggers = "SVDTrgOccInjHER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  //Finding only one of them should only happen in very strange situations...
  //m_nodes[3].histo = Triggers;
  if (Hits && Triggers) {
    m_hInjectionHERSVD->Divide(Hits, Triggers);
    m_hInjectionHERSVDOcc->Divide(Hits, Triggers, 100, 768 * 2 * 7); // to percent (L3V has 768 strips * 2 * 7 sides)
  }

  m_cInjectionHERSVD->Clear();
  m_cInjectionHERSVD->cd(0);
  m_hInjectionHERSVD->Draw("hist");

  m_cInjectionHERSVDOcc->Clear();
  m_cInjectionHERSVDOcc->cd(0);
  m_hInjectionHERSVDOcc->Draw("hist");


  //ECL
  m_histogramDirectoryName = "ECLINJ";

  locationHits = "ECLHitsInjLER";
  if (m_histogramDirectoryName != "") {
    locationHits = m_histogramDirectoryName + "/" + locationHits;
  }
  Hits = (TH1*)findHist(locationHits.Data());
  locationTriggers = "ECLEHitsInjLER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  //Finding only one of them should only happen in very strange situations...
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    m_nodes[0].histo = Triggers;
  }
#endif
  if (Hits && Triggers) {
    m_hInjectionLERECL->Divide(Hits, Triggers);
  }

  m_cInjectionLERECL->Clear();
  m_cInjectionLERECL->cd(0);
  m_hInjectionLERECL->Draw("hist");

  locationHits = "ECLHitsInjHER";
  if (m_histogramDirectoryName != "") {
    locationHits = m_histogramDirectoryName + "/" + locationHits;
  }
  Hits = (TH1*)findHist(locationHits.Data());
  locationTriggers = "ECLEHitsInjHER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  //Finding only one of them should only happen in very strange situations...
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    m_nodes[3].histo = Triggers;
  }
#endif
  if (Hits && Triggers) {
    m_hInjectionHERECL->Divide(Hits, Triggers);
  }

  m_cInjectionHERECL->Clear();
  m_cInjectionHERECL->cd(0);
  m_hInjectionHERECL->Draw("hist");
// =====
  locationHits = "ECLBurstsInjLER";
  if (m_histogramDirectoryName != "") {
    locationHits = m_histogramDirectoryName + "/" + locationHits;
  }
  Hits = (TH1*)findHist(locationHits.Data());
  locationTriggers = "ECLEBurstsInjLER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  if (Hits && Triggers) {
    m_hBurstLERECL->Divide(Hits, Triggers);
  }

  m_cBurstLERECL->Clear();
  m_cBurstLERECL->cd(0);
  m_hBurstLERECL->Draw("hist");
// =====

  locationHits = "ECLBurstsInjHER";
  if (m_histogramDirectoryName != "") {
    locationHits = m_histogramDirectoryName + "/" + locationHits;
  }
  Hits = (TH1*)findHist(locationHits.Data());
  locationTriggers = "ECLEBurstsInjHER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  if (Hits && Triggers) {
    m_hBurstHERECL->Divide(Hits, Triggers);
  }

  m_cBurstHERECL->Clear();
  m_cBurstHERECL->cd(0);
  m_hBurstHERECL->Draw("hist");
// =====


  //TOP
  m_histogramDirectoryName = "TOP";

  locationHits = "TOPOccInjLER";
  if (m_histogramDirectoryName != "") {
    locationHits = m_histogramDirectoryName + "/" + locationHits;
  }
  Hits = (TH1*)findHist(locationHits.Data());
  locationTriggers = "TOPEOccInjLER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  if (Hits && Triggers) {
    m_hInjectionLERTOP->Divide(Hits, Triggers, 100, 8192);
  }

  m_cInjectionLERTOP->Clear();
  m_cInjectionLERTOP->cd(0);
  m_hInjectionLERTOP->Draw("hist");

  locationHits = "TOPOccInjHER";
  if (m_histogramDirectoryName != "") {
    locationHits = m_histogramDirectoryName + "/" + locationHits;
  }
  Hits = (TH1*)findHist(locationHits.Data());
  locationTriggers = "TOPEOccInjHER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  if (Hits && Triggers) {
    m_hInjectionHERTOP->Divide(Hits, Triggers, 100, 8192);
  }

  m_cInjectionHERTOP->Clear();
  m_cInjectionHERTOP->cd(0);
  m_hInjectionHERTOP->Draw("hist");



  //ARICH
  m_histogramDirectoryName = "ARICH";

  locationHits = "ARICHOccInjLER";
  if (m_histogramDirectoryName != "") {
    locationHits = m_histogramDirectoryName + "/" + locationHits;
  }
  Hits = (TH1*)findHist(locationHits.Data());
  locationTriggers = "ARICHEOccInjLER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  if (Hits && Triggers) {
    m_hInjectionLERARICH->Divide(Hits, Triggers);
  }

  m_cInjectionLERARICH->Clear();
  m_cInjectionLERARICH->cd(0);
  m_hInjectionLERARICH->Draw("hist");

  locationHits = "ARICHOccInjHER";
  if (m_histogramDirectoryName != "") {
    locationHits = m_histogramDirectoryName + "/" + locationHits;
  }
  Hits = (TH1*)findHist(locationHits.Data());
  locationTriggers = "ARICHEOccInjHER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  if (Hits && Triggers) {
    m_hInjectionHERARICH->Divide(Hits, Triggers);
  }

  m_cInjectionHERARICH->Clear();
  m_cInjectionHERARICH->cd(0);
  m_hInjectionHERARICH->Draw("hist");

  // KLM
  m_histogramDirectoryName = "KLM";

  locationHits = "KLMOccInjLER";
  if (m_histogramDirectoryName != "") {
    locationHits = m_histogramDirectoryName + "/" + locationHits;
  }
  Hits = (TH1*)findHist(locationHits.Data());
  locationTriggers = "KLMTrigInjLER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  if (Hits && Triggers) {
    m_hInjectionLERKLM->Divide(Hits, Triggers, 100, KLMElementNumbers::getTotalChannelNumber());
  }

  m_cInjectionLERKLM->Clear();
  m_cInjectionLERKLM->cd(0);
  m_hInjectionLERKLM->Draw("hist");

  locationHits = "KLMOccInjHER";
  if (m_histogramDirectoryName != "") {
    locationHits = m_histogramDirectoryName + "/" + locationHits;
  }
  Hits = (TH1*)findHist(locationHits.Data());
  locationTriggers = "KLMTrigInjHER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  if (Hits && Triggers) {
    m_hInjectionHERKLM->Divide(Hits, Triggers, 100, KLMElementNumbers::getTotalChannelNumber());
  }

  m_cInjectionHERKLM->Clear();
  m_cInjectionHERKLM->cd(0);
  m_hInjectionHERKLM->Draw("hist");

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    for (auto& m : m_nodes) {
      if (!m.mychid) continue;
      int length = m.data.size();
      if (length != int(ca_element_count(m.mychid)) && int(ca_element_count(m.mychid)) > 0) {
        // FIXME, unclear why this is needed to prevent crashes on new run?
        m.data.resize(int(ca_element_count(m.mychid)), 0.0);
        length = m.data.size();
      }
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
        SEVCHK(ca_array_put(DBR_DOUBLE, length, m.mychid, (void*)m.data.data()), "ca_put failure");
      } else {
        B2DEBUG(99, "Inj " << ca_name(m.mychid) << " , " << m.histo << " , " << (m.histo ? m.histo->GetNcells() : 0) << " , " << length <<
                " , "
                <<
                ca_element_count(m.mychid));
      }
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

void DQMHistInjectionModule::cleanPVs(void)
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    for (auto m : m_nodes) {
      if (m.mychid) {
        int length = int(ca_element_count(m.mychid));
        if (length > 0) {
          m.data.resize(length, 0.0);
          SEVCHK(ca_array_put(DBR_DOUBLE, length, m.mychid, (void*)m.data.data()), "ca_put failure");
        } else {
          B2DEBUG(99, "clean: lenght " << ca_name(m.mychid));
        }
      } else {
        B2DEBUG(99, "clean: chid " << ca_name(m.mychid));
      }
    }
  }
#endif
}

void DQMHistInjectionModule::terminate()
{
  B2DEBUG(1, "DQMHistInjection: terminate called");
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    for (auto m : m_nodes) {
      SEVCHK(ca_clear_channel(m.mychid), "ca_clear_channel failure");
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

