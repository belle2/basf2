//+
// File : DQMHistAnalysisPXDCM.cc
// Description : Analysis of PXD Common Modes
//
// Author : Bjoern Spruck, Univerisity Mainz
// Date : 2018
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDCM.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TClass.h>
#include <vxd/geometry/GeoCache.h>

using namespace std;
using namespace Belle2;


using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDCM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDCMModule::DQMHistAnalysisPXDCMModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  //Parameter definition
  addParam("HistoDir", m_histogramDirectoryName, "Name of Histogram dir", std::string("pxd"));
  addParam("PVName", m_pvPrefix, "PV Prefix", std::string("DQM:PXD:CommonMode"));
  B2DEBUG(1, "DQMHistAnalysisPXDCM: Constructor done.");
}

void DQMHistAnalysisPXDCMModule::initialize()
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  //collect the list of all PXD Modules in the geometry here
  std::vector<VxdID> sensors = geo.getListOfSensors();
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = geo.getSensorInfo(aVxdID);
    // B2DEBUG(20,"VXD " << aVxdID);
    if (info.getType() != VXD::SensorInfoBase::PXD) continue;
    m_PXDModules.push_back(aVxdID); // reorder, sort would be better

  }
  std::sort(m_PXDModules.begin(), m_PXDModules.end());  // back to natural order

  gROOT->cd(); // this seems to be important, or strange things happen

  m_cCommonMode = new TCanvas("c_CommonMode");
  m_hCommonMode = new TH2F("CommonMode", "CommonMode; Module; CommonMode", m_PXDModules.size(), 0, m_PXDModules.size(), 64, 0, 64);
  m_hCommonMode->SetDirectory(0);// dont mess with it, this is MY histogram
  m_hCommonMode->SetStats(false);
  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    TString ModuleName = (std::string)m_PXDModules[i];
    m_hCommonMode->GetXaxis()->SetBinLabel(i + 1, ModuleName);
  }
  //Unfortunately this only changes the labels, but can't fill the bins by the VxdIDs
  m_hCommonMode->Draw("colz");

  /// FIXME were to put the lines depends ...
  m_line1 = new TLine(0, 10, m_PXDModules.size(), 10);
  m_line2 = new TLine(0, 16, m_PXDModules.size(), 16);
  m_line3 = new TLine(0, 3, m_PXDModules.size(), 3);
  m_line1->SetHorizontal(true);
  m_line1->SetLineColor(3);// Green
  m_line1->SetLineWidth(3);
  m_line2->SetHorizontal(true);
  m_line2->SetLineColor(1);// Black
  m_line2->SetLineWidth(3);
  m_line3->SetHorizontal(true);
  m_line3->SetLineColor(1);
  m_line3->SetLineWidth(3);


#ifdef _BELLE2_EPICS
  SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
  SEVCHK(ca_create_channel(m_pvPrefix.data(), NULL, NULL, 10, &mychid), "ca_create_channel failure");
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
  B2DEBUG(1, "DQMHistAnalysisPXDCM: initialized.");
}

void DQMHistAnalysisPXDCMModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDCM: beginRun called.");

  m_cCommonMode->Clear();
  m_cCommonMode->SetLogz();
}


TH1* DQMHistAnalysisPXDCMModule::findHistLocal(TString& a)
{
  B2INFO("Histo " << a << " not in memfile");
  // the following code sux ... is there no root function for that?
  TDirectory* d = gROOT;
  TString myl = a;
  TString tok;
  Ssiz_t from = 0;
  while (myl.Tokenize(tok, from, "/")) {
    TString dummy;
    Ssiz_t f;
    f = from;
    if (myl.Tokenize(dummy, f, "/")) { // check if its the last one
      auto e = d->GetDirectory(tok);
      if (e) {
        B2INFO("Cd Dir " << tok);
        d = e;
      }
      d->cd();
    } else {
      break;
    }
  }
  TObject* obj = d->FindObject(tok);
  if (obj != NULL) {
    if (obj->IsA()->InheritsFrom("TH1")) {
      B2INFO("Histo " << a << " found in mem");
      return (TH1*)obj;
    }
  } else {
    B2INFO("Histo " << a << " NOT found in mem");
  }
  return NULL;
}

void DQMHistAnalysisPXDCMModule::event()
{
  double data = 0.0;
  if (!m_cCommonMode) return;
  m_hCommonMode->Reset(); // dont sum up!!!

  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    VxdID& aModule = m_PXDModules[i ];

    TString buff = (std::string)aModule;
//     buff.ReplaceAll(".", "_");

    TH1* hh1 = NULL;

    TString a = "PXDDAQCM2_" + buff;
    hh1 = findHist(a.Data());
    if (hh1 == NULL) {
      hh1 = findHistLocal(a);
    }
    if (hh1 == NULL) {
      a = m_histogramDirectoryName + "/PXDDAQCM2_" + buff;
      hh1 = findHist(a.Data());
    }
    if (hh1 == NULL) {
      a = m_histogramDirectoryName + "/PXDDAQCM2_" + buff;
      hh1 = findHistLocal(a);
    }
    if (hh1) {
      B2INFO("Histo " << a << " found in mem");
      for (int bin = 1; bin <= 64; bin++) {
        float v;
        v = hh1->GetBinContent(bin);
        m_hCommonMode->Fill(i, bin, v);
      }

      /// FIXME: integration intervalls depend on CM default value
      data += hh1->Integral(16, 64);
//   B2INFO("data inc hi "<<data);
      data += hh1->Integral(0, 5);
//   B2INFO("data inc lo "<<data);
    }
  }
  m_cCommonMode->cd();


  // not enough Entries
  // it->canvas->Pad()->SetFillColor(6);// Magenta
//   B2INFO("data "<<data);
  /// FIXME: absolute numbers or relative numbers and what is the laccpetable limit?
  if (data > 100.) {
    m_cCommonMode->Pad()->SetFillColor(2);// Red
  } else if (data > 50.) {
    m_cCommonMode->Pad()->SetFillColor(5);// Yellow
  } else {
    m_cCommonMode->Pad()->SetFillColor(0);// White
  }

  if (m_hCommonMode) {
    m_hCommonMode->Draw("colz");
    m_line1->Draw();
    m_line2->Draw();
    m_line3->Draw();
  }

  m_cCommonMode->Modified();
  m_cCommonMode->Update();
#ifdef _BELLE2_EPICS
  SEVCHK(ca_put(DBR_DOUBLE, mychid, (void*)&data), "ca_set failure");
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
}

void DQMHistAnalysisPXDCMModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDCM: terminate called");
  // m_cCommonMode->Print("c1.pdf");
  // should delete canvas here, maybe hist, too? Who owns it?
}

