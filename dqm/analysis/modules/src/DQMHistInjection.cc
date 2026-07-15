/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistInjection.cc
// Description : DQM module, which gives histograms showing occupancies after injection
//-


#include <dqm/analysis/modules/DQMHistInjection.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <TROOT.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistInjection);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistInjectionModule::DQMHistInjectionModule() : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("DQM:INJ:"));
  B2DEBUG(1, "DQMHistInjection: Constructor done.");
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

  m_hInjectionLERPXD = new TH1F("HitInjectionLERPXD", "PXD Hits after LER Injection;Time in #mus;Mean Hits/event", 4000, 0, 20000);
  m_hInjectionLERPXDOcc = new TH1F("HitInjectionPXDLEROcc", "PXD Occ after LER Injection;Time in #mus;Mean Occ in % per module", 4000,
                                   0, 20000);
  m_hInjectionLERSVD = new TH1F("HitInjectionLERSVD", "SVD Hits after LER Injection;Time in #mus;Mean Hits/event", 4000, 0, 20000);
  m_hInjectionLERSVDOcc = new TH1F("HitInjectionSVDLEROcc", "SVD Occ after LER Injection;Time in #mus;Mean Occ in % per module", 4000,
                                   0, 20000);
  m_hInjectionLERECL = new TH1F("HitInjectionLERECL", "ECL Hits after LER Injection;Time in #mus;Mean Hits/event", 4000, 0, 20000);
  m_hBurstLERECL = new TH1F("BurstInjectionLERECL", "ECL Bursts after LER Injection;Time in #mus;Suppressions/event (1 #mus bins)",
                            20000, 0, 20000);
  m_hInjectionLERTOP = new TH1F("HitInjectionLERTOP", "TOP Occ after LER Injection;Time in #mus;Mean Occ in % /event", 4000, 0,
                                20000);
  m_hInjectionLERARICH = new TH1F("HitInjectionLERARICH", "ARICH Occ after LER Injection;Time in #mus;Mean Hits/event", 4000, 0,
                                  20000);
  m_hInjectionLERKLM = new TH1F("HitInjectionLERKLM",
                                "KLM occupancy after LER Injection;Time [#mus];Digits occupancy in % / (5 #mus)", 4000, 0,
                                20000);

  m_hInjectionHERPXD = new TH1F("HitInjectionHERPXD", "PXD Hits after HER Injection;Time in #mus;Mean Hits/event", 4000, 0, 20000);
  m_hInjectionHERPXDOcc = new TH1F("HitInjectionPXDHEROcc", "PXD Occ after HER Injection;Time in #mus;Mean Occ in % per module", 4000,
                                   0, 20000);
  m_hInjectionHERSVD = new TH1F("HitInjectionHERSVD", "SVD Hits after HER Injection;Time in #mus;Mean Hits/event", 4000, 0, 20000);
  m_hInjectionHERSVDOcc = new TH1F("HitInjectionSVDHEROcc", "SVD Occ after HER Injection;Time in #mus;Mean Occ in % per module", 4000,
                                   0, 20000);
  m_hInjectionHERECL = new TH1F("HitInjectionHERECL", "ECL Hits after HER Injection;Time in #mus;Mean Hits/event", 4000, 0, 20000);
  m_hBurstHERECL = new TH1F("BurstInjectionHERECL", "ECL Bursts after HER Injection;Time in #mus;Suppressions/event (1 #mus bins)",
                            20000, 0, 20000);
  m_hInjectionHERTOP = new TH1F("HitInjectionHERTOP", "TOP Occ after HER Injection;Time in #mus;Mean Occ in % /event", 4000, 0,
                                20000);
  m_hInjectionHERARICH = new TH1F("HitInjectionHERARICH", "ARICH Occ after HER Injection;Time in #mus;Mean Hits/event", 4000, 0,
                                  20000);
  m_hInjectionHERKLM = new TH1F("HitInjectionHERKLM",
                                "KLM occupancy after HER Injection;Time [#mus];Digits occupancy in % / (5 #mus)", 4000, 0,
                                20000);

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

}


void DQMHistInjectionModule::event()
{

  // PXD
  {
    // LER
    auto Hits = findHist("PXDOccInjLER");
    auto Triggers = findHist("PXDINJ", "PXDEOccInjLER");
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
  }
  {
    // HER
    auto Hits = findHist("PXDINJ", "PXDOccInjHER");
    auto Triggers = findHist("PXDINJ", "PXDEOccInjHER");
    //Finding only one of them should only happen in very strange situations...
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
  }

  // SVD
  {
    // LER
    auto Hits = findHist("SVDInjection", "SVDOccInjLER");
    auto Triggers = findHist("SVDInjection", "SVDTrgOccInjLER");
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
  }
  {
    // HER
    auto Hits = findHist("SVDInjection", "SVDOccInjHER");
    auto Triggers = findHist("SVDInjection", "SVDTrgOccInjHER");
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
  }

  // ECL
  {
    // LER
    auto Hits = findHist("ECLINJ", "ECLHitsInjLER");
    auto Triggers = findHist("ECLINJ", "ECLEHitsInjLER");
    if (Hits && Triggers) {
      m_hInjectionLERECL->Divide(Hits, Triggers);
    }
    m_cInjectionLERECL->Clear();
    m_cInjectionLERECL->cd(0);
    m_hInjectionLERECL->Draw("hist");
  }
  {
    // HER
    auto Hits = findHist("ECLINJ", "ECLHitsInjHER");
    auto Triggers = findHist("ECLINJ", "ECLEHitsInjHER");
    if (Hits && Triggers) {
      m_hInjectionHERECL->Divide(Hits, Triggers);
    }
    m_cInjectionHERECL->Clear();
    m_cInjectionHERECL->cd(0);
    m_hInjectionHERECL->Draw("hist");
  }
  {
    // Burst LER
    auto Hits = findHist("ECLINJ", "ECLBurstsInjLER");
    auto Triggers = findHist("ECLINJ", "ECLEBurstsInjLER");
    if (Hits && Triggers) {
      m_hBurstLERECL->Divide(Hits, Triggers);
    }
    m_cBurstLERECL->Clear();
    m_cBurstLERECL->cd(0);
    m_hBurstLERECL->Draw("hist");
  }
  {
    // Burst HER
    auto Hits = findHist("ECLINJ", "ECLBurstsInjHER");
    auto Triggers = findHist("ECLINJ", "ECLEBurstsInjHER");
    if (Hits && Triggers) {
      m_hBurstHERECL->Divide(Hits, Triggers);
    }
    m_cBurstHERECL->Clear();
    m_cBurstHERECL->cd(0);
    m_hBurstHERECL->Draw("hist");
  }

  // TOP
  {
    // LER
    auto Hits = findHist("TOP", "TOPOccInjLER");
    auto Trigger = findHist("TOP", "TOPEOccInjLER");
    if (Hits && Triggers) {
      m_hInjectionLERTOP->Divide(Hits, Triggers, 100, 8192);
    }
    m_cInjectionLERTOP->Clear();
    m_cInjectionLERTOP->cd(0);
    m_hInjectionLERTOP->Draw("hist");
  }
  {
    // HER
    auto Hits = findHist("TOP", "TOPOccInjHER");
    auto Trigger = findHist("TOP", "TOPEOccInjHER");
    if (Hits && Triggers) {
      m_hInjectionHERTOP->Divide(Hits, Triggers, 100, 8192);
    }
    m_cInjectionHERTOP->Clear();
    m_cInjectionHERTOP->cd(0);
    m_hInjectionHERTOP->Draw("hist");
  }

  // ARICH
  {
    // LER
    auto Hits = findHist("ARICH", "ARICHOccInjLER");
    auto Triggers = findHist("ARICH", "ARICHEOccInjLER");
    if (Hits && Triggers) {
      m_hInjectionLERARICH->Divide(Hits, Triggers);
    }
    m_cInjectionLERARICH->Clear();
    m_cInjectionLERARICH->cd(0);
    m_hInjectionLERARICH->Draw("hist");
  }
  {
    // HER
    auto Hits = findHist("ARICH", "ARICHOccInjHER");
    auto Triggers = findHist("ARICH", "ARICHEOccInjHER");
    if (Hits && Triggers) {
      m_hInjectionHERARICH->Divide(Hits, Triggers);
    }
    m_cInjectionHERARICH->Clear();
    m_cInjectionHERARICH->cd(0);
    m_hInjectionHERARICH->Draw("hist");
  }

  // KLM
  {
    // LER
    auto Hits = findHist("KLM", "KLMOccInjLER");
    auto Triggers = findHist("KLM", "KLMTrigInjLER");
    if (Hits && Triggers) {
      m_hInjectionLERKLM->Divide(Hits, Triggers, 100, KLMElementNumbers::getTotalChannelNumber());
    }
    m_cInjectionLERKLM->Clear();
    m_cInjectionLERKLM->cd(0);
    m_hInjectionLERKLM->Draw("hist");
  }
  {
    // HER
    auto Hits = findHist("KLM", "KLMOccInjHER");
    auto Triggers = findHist("KLM", "KLMTrigInjHER");
    if (Hits && Triggers) {
      m_hInjectionHERKLM->Divide(Hits, Triggers, 100, KLMElementNumbers::getTotalChannelNumber());
    }
    m_cInjectionHERKLM->Clear();
    m_cInjectionHERKLM->cd(0);
    m_hInjectionHERKLM->Draw("hist");
  }
}

