//+
// File : DQMHistAnalysisPXDReduction.cc
// Description : Analysis of PXD Reduction
//
// Author : Bjoern Spruck, Univerisity Mainz
// Date : 2018
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDReduction.h>
#include <TROOT.h>
#include <vxd/geometry/GeoCache.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDReduction)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDReductionModule::DQMHistAnalysisPXDReductionModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  //Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("PXDDAQ"));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("DQM:PXD:Red:"));
  addParam("useEpics", m_useEpics, "useEpics", true);
  B2DEBUG(1, "DQMHistAnalysisPXDReduction: Constructor done.");
}

DQMHistAnalysisPXDReductionModule::~DQMHistAnalysisPXDReductionModule()
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (ca_current_context()) ca_context_destroy();
  }
#endif
}

void DQMHistAnalysisPXDReductionModule::initialize()
{
  B2DEBUG(1, "DQMHistAnalysisPXDReduction: initialized.");

  m_monObj = getMonitoringObject("pxd");
  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();

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

  m_cReduction = new TCanvas((m_histogramDirectoryName + "/c_Reduction").data());
  m_hReduction = new TH1F("hPXDReduction", "PXD Reduction; Module; Reduction", m_PXDModules.size(), 0, m_PXDModules.size());
  m_hReduction->SetDirectory(0);// dont mess with it, this is MY histogram
  m_hReduction->SetStats(false);
  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    TString ModuleName = (std::string)m_PXDModules[i];
    m_hReduction->GetXaxis()->SetBinLabel(i + 1, ModuleName);
  }
  //Unfortunately this only changes the labels, but can't fill the bins by the VxdIDs
  m_hReduction->Draw("");
  m_monObj->addCanvas(m_cReduction);

  /// FIXME were to put the lines depends ...
  m_line1 = new TLine(0, 10, m_PXDModules.size(), 10);
//   m_line2 = new TLine(0, 16, m_PXDModules.size(), 16);
//   m_line3 = new TLine(0, 3, m_PXDModules.size(), 3);
  m_line1->SetHorizontal(true);
  m_line1->SetLineColor(3);// Green
  m_line1->SetLineWidth(3);
//   m_line2->SetHorizontal(true);
//   m_line2->SetLineColor(1);// Black
//   m_line2->SetLineWidth(3);
//   m_line3->SetHorizontal(true);
//   m_line3->SetLineColor(1);
//   m_line3->SetLineWidth(3);


#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    mychid.resize(2);
    SEVCHK(ca_create_channel((m_pvPrefix + "Status").data(), NULL, NULL, 10, &mychid[0]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "Value").data(), NULL, NULL, 10, &mychid[1]), "ca_create_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}


void DQMHistAnalysisPXDReductionModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDReduction: beginRun called.");

  m_cReduction->Clear();
}

void DQMHistAnalysisPXDReductionModule::event()
{
  if (!m_cReduction) return;
  m_hReduction->Reset(); // dont sum up!!!

  bool enough = false;
  double ireduction = 0.0;
  int ireductioncnt = 0;

  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    std::string name = "PXDDAQDHEDataReduction_" + (std::string)m_PXDModules[i ];
    // std::replace( name.begin(), name.end(), '.', '_');

    TH1* hh1 = findHist(name);
    if (hh1 == NULL) {
      hh1 = findHist(m_histogramDirectoryName, name);
    }
    if (hh1) {
      auto mean = hh1->GetMean();
      m_hReduction->Fill(i, mean);
      if (hh1->GetEntries() > 100) enough = true;
      if (mean > 0) {
        ireduction += mean; // well fit would be better
        ireductioncnt++;
      }
    }
  }
  m_cReduction->cd();

  int status = 0;
  // not enough Entries
  if (!enough) {
    status = 0; // Grey
    m_cReduction->Pad()->SetFillColor(kGray);// Magenta or Gray
  } else {
    status = 1; // White
    /// FIXME: absolute numbers or relative numbers and what is the accpetable limit?
//   if (value > m_up_err_limit || value < m_low_err_limit ) {
//     m_cReduction->Pad()->SetFillColor(kRed);// Red
//   } else if (value >  m_up_warn_limit ||  value < m_low_warn_limit ) {
//     m_cReduction->Pad()->SetFillColor(kYellow);// Yellow
//   } else {
//     m_cReduction->Pad()->SetFillColor(kGreen);// Green
//   } else {
    m_cReduction->Pad()->SetFillColor(kWhite);// White
//   }
  }

  double value = ireductioncnt > 0 ? ireduction / ireductioncnt : 0;

  if (m_hReduction) {
    m_hReduction->Draw("");
    if (status != 0) {
      m_line1->SetY1(value);
      m_line1->SetY2(value); // aka SetHorizontal
      m_line1->Draw();
    }
//     m_line2->Draw();
//     m_line3->Draw();
  }

  m_monObj->setVariable("reduction", value);

  m_cReduction->Modified();
  m_cReduction->Update();
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    SEVCHK(ca_put(DBR_INT, mychid[0], (void*)&status), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[1], (void*)&value), "ca_set failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

void DQMHistAnalysisPXDReductionModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDReduction: terminate called");
  // m_cReduction->Print("c1.pdf");
  // should delete canvas here, maybe hist, too? Who owns it?
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    for (auto m : mychid) SEVCHK(ca_clear_channel(m), "ca_clear_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

