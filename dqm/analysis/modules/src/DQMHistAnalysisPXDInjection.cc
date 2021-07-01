//+
// File : DQMHistAnalysisPXDInjection.cc
// Description : DQM module, which gives histograms showing occupancies after injection
//
// Author : B. Spruck
// Date : someday
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDInjection.h>
#include <TROOT.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDInjection)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDInjectionModule::DQMHistAnalysisPXDInjectionModule() : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms were placed",
           std::string("PXDINJ"));
  B2DEBUG(1, "DQMHistAnalysisPXDInjection: Constructor done.");
}

void DQMHistAnalysisPXDInjectionModule::initialize()
{

  gROOT->cd(); // this seems to be important, or strange things happen

  m_cInjectionLERPXD = new TCanvas(m_histogramDirectoryName +
                                   TString("/c_PXDInjectionLER")); // different name, as we have the other module
  m_cInjectionHERPXD = new TCanvas(m_histogramDirectoryName +
                                   TString("/c_PXDInjectionHER")); // which makes this hist for ALL detectors (SVD, ECl, TOP)
  m_hInjectionLERPXD = new TH1F("HitInjectionLERPXD", "PXD Hits after LER Injection;Time in #mus;Mean Hits/event", 4000, 0 , 20000);
  m_hInjectionHERPXD = new TH1F("HitInjectionHERPXD", "PXD Hits after HER Injection;Time in #mus;Mean Hits/event", 4000, 0 , 20000);
  const VXD::GeoCache& vxdGeometry = VXD::GeoCache::getInstance();
  std::vector<VxdID> vxdsensors = vxdGeometry.getListOfSensors();
  for (VxdID& avxdid : vxdsensors) {
    VXD::SensorInfoBase info = vxdGeometry.getSensorInfo(avxdid);
    if (info.getType() != VXD::SensorInfoBase::PXD) continue;
    m_sensors.push_back(avxdid);
  }

  for (VxdID& avxdid : m_sensors) {
    TString buff = (std::string)avxdid;
    TString bufful = buff;
    bufful.ReplaceAll(".", "_");

    m_cInjectionLERPXDMod[avxdid] = new TCanvas(m_histogramDirectoryName + "/c_PXDInjectionLER_" + bufful);
    m_cInjectionHERPXDMod[avxdid] = new TCanvas(m_histogramDirectoryName + "/c_PXDInjectionHER_" + bufful);
    m_hInjectionLERPXDMod[avxdid] = new TH1F("HitInjectionLERPXD_" + bufful,
                                             "PXD Hits after LER Injection " + buff + "/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
    m_hInjectionHERPXDMod[avxdid] = new TH1F("HitInjectionHERPXD_" + bufful,
                                             "PXD Hits after HER Injection " + buff + "/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
    m_cInjectionLERPXDModNorm[avxdid] = new TCanvas(m_histogramDirectoryName + "/c_PXDInjectionLERNorm_" + bufful);
    m_cInjectionHERPXDModNorm[avxdid] = new TCanvas(m_histogramDirectoryName + "/c_PXDInjectionHERNorm_" + bufful);
    m_hInjectionLERPXDModNorm[avxdid] = new TH1F("HitInjectionLERPXDNorm_" + bufful,
                                                 "PXD Hits after LER Injection " + buff + " normalized to 1.1.x/Time;Time in #mus;factor", 4000, 0, 20000);
    m_hInjectionHERPXDModNorm[avxdid] = new TH1F("HitInjectionHERPXDNorm_" + bufful,
                                                 "PXD Hits after HER Injection " + buff + " normalized to 1.1.x/Time;Time in #mus;factor", 4000, 0, 20000);
  }

  B2DEBUG(1, "DQMHistAnalysisPXDInjection: initialized.");
}


void DQMHistAnalysisPXDInjectionModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDInjection: beginRun called.");

//   m_cInjectionLERPXD->Clear(); // FIXME, unclear if this lets to crashes on new run?
//   m_cInjectionLERPXDOcc->Clear();
//   m_cInjectionLERECL->Clear();
//   m_cInjectionHERPXD->Clear();
//   m_cInjectionHERPXDOcc->Clear();
//   m_cInjectionHERECL->Clear();
}


void DQMHistAnalysisPXDInjectionModule::event()
{
  TH1* Triggers = nullptr;
  // cppcheck-suppress unreadVariable
  TString locationHits = "";
  TString locationTriggers = "PXDEOccInjLER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  //Finding only one of them should only happen in very strange situations...
  //m_nodes[0].histo = Triggers;
  if (Triggers) {
    TH1* Hits = nullptr, *RefMod_fw = nullptr, *RefMod_bw = nullptr;
    locationHits = "PXDOccInjLER";
    if (m_histogramDirectoryName != "") {
      locationHits = m_histogramDirectoryName + "/" + locationHits;
    }
    Hits = (TH1*)findHist(locationHits.Data());
    if (Hits) {
      m_hInjectionLERPXD->Divide(Hits, Triggers);
    }
    locationHits = "PXDOccInjLER_1_1_1";
    if (m_histogramDirectoryName != "") {
      locationHits = m_histogramDirectoryName + "/" + locationHits;
    }
    RefMod_fw = (TH1*)findHist(locationHits.Data());
    locationHits = "PXDOccInjLER_1_1_2";
    if (m_histogramDirectoryName != "") {
      locationHits = m_histogramDirectoryName + "/" + locationHits;
    }
    RefMod_bw = (TH1*)findHist(locationHits.Data());

    // assume trigger is the same for all modules (makes sense :-)
    for (VxdID& avxdid : m_sensors) {
      TString buff = (std::string)avxdid;
      TString bufful = buff;
      bufful.ReplaceAll(".", "_");

      locationHits = "PXDOccInjLER_" + bufful;
      if (m_histogramDirectoryName != "") {
        locationHits = m_histogramDirectoryName + "/" + locationHits;
      }
      Hits = (TH1*)findHist(locationHits.Data());
      if (Hits) {
        m_hInjectionLERPXDMod[avxdid]->Divide(Hits, Triggers);
        if (avxdid.getSensorNumber() == 1 && RefMod_fw) {
          m_hInjectionLERPXDModNorm[avxdid]->Divide(Hits, RefMod_fw);
        } else if (avxdid.getSensorNumber() == 2 && RefMod_bw) {
          m_hInjectionLERPXDModNorm[avxdid]->Divide(Hits, RefMod_bw);
        }
      }

    }

  }

  locationTriggers = "PXDEOccInjHER";
  if (m_histogramDirectoryName != "") {
    locationTriggers = m_histogramDirectoryName + "/" + locationTriggers;
  }
  Triggers = (TH1*)findHist(locationTriggers.Data());

  //Finding only one of them should only happen in very strange situations...
  //m_nodes[3].histo = Triggers;
  if (Triggers) {
    TH1* Hits = nullptr, *RefMod_fw = nullptr, *RefMod_bw = nullptr;
    locationHits = "PXDOccInjHER";
    if (m_histogramDirectoryName != "") {
      locationHits = m_histogramDirectoryName + "/" + locationHits;
    }
    Hits = (TH1*)findHist(locationHits.Data());
    if (Hits) {
      m_hInjectionHERPXD->Divide(Hits, Triggers);
    }
    locationHits = "PXDOccInjHER_1_1_1";
    if (m_histogramDirectoryName != "") {
      locationHits = m_histogramDirectoryName + "/" + locationHits;
    }
    RefMod_fw = (TH1*)findHist(locationHits.Data());
    locationHits = "PXDOccInjHER_1_1_2";
    if (m_histogramDirectoryName != "") {
      locationHits = m_histogramDirectoryName + "/" + locationHits;
    }
    RefMod_bw = (TH1*)findHist(locationHits.Data());
    // assume trigger is the same for all modules (makes sense :-)
    for (VxdID& avxdid : m_sensors) {
      TString buff = (std::string)avxdid;
      TString bufful = buff;
      bufful.ReplaceAll(".", "_");
      locationHits = "PXDOccInjHER_" + bufful;
      if (m_histogramDirectoryName != "") {
        locationHits = m_histogramDirectoryName + "/" + locationHits;
      }
      Hits = (TH1*)findHist(locationHits.Data());
      if (Hits) {
        m_hInjectionHERPXDMod[avxdid]->Divide(Hits, Triggers);
        if (avxdid.getSensorNumber() == 1 && RefMod_fw) {
          m_hInjectionHERPXDModNorm[avxdid]->Divide(Hits, RefMod_fw);
        } else if (avxdid.getSensorNumber() == 2 && RefMod_bw) {
          m_hInjectionHERPXDModNorm[avxdid]->Divide(Hits, RefMod_bw);
        }
      }


    }
  }

  m_cInjectionLERPXD->Clear();
  m_cInjectionLERPXD->cd(0);
  m_cInjectionLERPXD->Pad()->SetLogy();
  m_hInjectionLERPXD->Draw("hist");

  m_cInjectionHERPXD->Clear();
  m_cInjectionHERPXD->cd(0);
  m_cInjectionHERPXD->Pad()->SetLogy();
  m_hInjectionHERPXD->Draw("hist");

  for (VxdID& avxdid : m_sensors) {
    m_cInjectionHERPXDMod[avxdid]->Clear();
    m_cInjectionHERPXDMod[avxdid]->cd(0);
    m_cInjectionHERPXDMod[avxdid]->Pad()->SetLogy();
    m_hInjectionHERPXDMod[avxdid]->Draw("hist");
    m_cInjectionHERPXDModNorm[avxdid]->Clear();
    m_cInjectionHERPXDModNorm[avxdid]->cd(0);
    m_hInjectionHERPXDModNorm[avxdid]->Draw("hist");
    m_cInjectionLERPXDMod[avxdid]->Clear();
    m_cInjectionLERPXDMod[avxdid]->cd(0);
    m_cInjectionLERPXDMod[avxdid]->Pad()->SetLogy();
    m_hInjectionLERPXDMod[avxdid]->Draw("hist");
    m_cInjectionLERPXDModNorm[avxdid]->Clear();
    m_cInjectionLERPXDModNorm[avxdid]->cd(0);
    m_hInjectionLERPXDModNorm[avxdid]->Draw("hist");
  }
}

