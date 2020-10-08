//+
// File : DQMHistAnalysisPXDCM.cc
// Description : Analysis of PXD Common Modes
//
// Author : Bjoern Spruck, University Mainz
// Date : 2018
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDCM.h>
#include <TROOT.h>
#include <TLatex.h>
#include <vxd/geometry/GeoCache.h>

using namespace std;
using namespace Belle2;

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
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("PXDCM"));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("DQM:PXD:CommonMode:"));
  addParam("useEpics", m_useEpics, "useEpics", true);
  addParam("minEntries", m_minEntries, "minimum number of new entries for last time slot", 10000);
  B2DEBUG(99, "DQMHistAnalysisPXDCM: Constructor done.");
}

DQMHistAnalysisPXDCMModule::~DQMHistAnalysisPXDCMModule()
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (ca_current_context()) ca_context_destroy();
  }
#endif
}

void DQMHistAnalysisPXDCMModule::initialize()
{
  m_monObj = getMonitoringObject("pxd");
  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  // collect the list of all PXD Modules in the geometry here
  std::vector<VxdID> sensors = geo.getListOfSensors();
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = geo.getSensorInfo(aVxdID);
    if (info.getType() != VXD::SensorInfoBase::PXD) continue;
    m_PXDModules.push_back(aVxdID); // reorder, sort would be better
  }
  std::sort(m_PXDModules.begin(), m_PXDModules.end());  // back to natural order

  gROOT->cd(); // this seems to be important, or strange things happen

  m_cCommonMode = new TCanvas((m_histogramDirectoryName + "/c_CommonMode").data());

  m_hCommonMode = new TH2F("CommonMode", "CommonMode; Module; CommonMode", m_PXDModules.size(), 0, m_PXDModules.size(), 63, 0, 63);
  m_hCommonMode->SetDirectory(0);// dont mess with it, this is MY histogram
  m_hCommonMode->SetStats(false);
  m_hCommonModeDelta = new TH2F("CommonModeDelta", "CommonModeDelta; Module; CommonModeDelta", m_PXDModules.size(), 0,
                                m_PXDModules.size(), 63, 0, 63);
  m_hCommonModeDelta->SetDirectory(0);// dont mess with it, this is MY histogram
  m_hCommonModeDelta->SetStats(false);
  m_hCommonModeOld = new TH2F("CommonModeOld", "CommonModeOld; Module; CommonModeOld", m_PXDModules.size(), 0, m_PXDModules.size(),
                              63, 0, 63);
  m_hCommonModeOld->SetDirectory(0);// dont mess with it, this is MY histogram
  m_hCommonModeOld->SetStats(false);

  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    TString ModuleName = (std::string)m_PXDModules[i];
    m_hCommonMode->GetXaxis()->SetBinLabel(i + 1, ModuleName);
    m_hCommonModeDelta->GetXaxis()->SetBinLabel(i + 1, ModuleName);
    m_hCommonModeOld->GetXaxis()->SetBinLabel(i + 1, ModuleName);
  }
  //Unfortunately this only changes the labels, but can't fill the bins by the VxdIDs
  m_hCommonMode->Draw("colz");
  m_hCommonModeDelta->Draw("colz");
  m_hCommonModeOld->Draw("colz");

  m_monObj->addCanvas(m_cCommonMode);

  /// FIXME were to put the lines depends ...
  m_line1 = new TLine(0, 10, m_PXDModules.size(), 10);
  m_line2 = new TLine(0, 16, m_PXDModules.size(), 16);
//   m_line3 = new TLine(0, 3, m_PXDModules.size(), 3);
  m_line1->SetHorizontal(true);
  m_line1->SetLineColor(3);// Green
  m_line1->SetLineWidth(3);
  m_line2->SetHorizontal(true);
  m_line2->SetLineColor(1);// Black
  m_line2->SetLineWidth(3);
//   m_line3->SetHorizontal(true);
//   m_line3->SetLineColor(1);
//   m_line3->SetLineWidth(3);


#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    mychid.resize(3);
    SEVCHK(ca_create_channel((m_pvPrefix + "Outside").data(), NULL, NULL, 10, &mychid[0]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "Status").data(), NULL, NULL, 10, &mychid[1]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "CM63").data(), NULL, NULL, 10, &mychid[2]), "ca_create_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
  B2DEBUG(99, "DQMHistAnalysisPXDCM: initialized.");
}

void DQMHistAnalysisPXDCMModule::beginRun()
{
  B2DEBUG(99, "DQMHistAnalysisPXDCM: beginRun called.");

  m_cCommonMode->Clear();
  m_hCommonModeDelta->Clear();
  m_hCommonModeOld->Clear();
  m_cCommonMode->SetLogz();
}

void DQMHistAnalysisPXDCMModule::event()
{
  double all_outside = 0.0, all = 0.0;
  double all_cm = 0.0;
  bool error_flag = false;
  bool warn_flag = false;
  if (!m_cCommonMode) return;
  m_hCommonMode->Reset(); // dont sum up!!!

  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    std::string name = "PXDDAQCM2_" + (std::string)m_PXDModules[i ];
    // std::replace( name.begin(), name.end(), '.', '_');

    TH1* hh1 = findHist(name);
    if (hh1 == NULL) {
      hh1 = findHist(m_histogramDirectoryName, name);
    }
    if (hh1) {
      double current = 0.0;
      double outside = 0.0;

      auto nevent = hh1->GetBinContent(0); // misuse underflow as event counter
      bool update = nevent - m_hCommonModeOld->GetBinContent(i + 1, 0) > m_minEntries ;
      if (update) m_hCommonModeOld->SetBinContent(i + 1, 0, nevent);
      for (int bin = 1; bin <= 63; bin++) { // we ignore CM63!!!
        double v;
        v = hh1->GetBinContent(bin);
        m_hCommonMode->SetBinContent(i + 1, bin, v); // attention, mixing bin nr and index
        current += v;
        if (nevent < m_minEntries) {
          m_hCommonModeDelta->SetBinContent(i + 1, bin, v); // attention, mixing bin nr and index
        } else if (update) {
          auto old = m_hCommonModeOld->GetBinContent(i + 1, bin); // attention, mixing bin nr and index
          m_hCommonModeDelta->SetBinContent(i + 1, bin, v - old); // attention, mixing bin nr and index
          m_hCommonModeOld->SetBinContent(i + 1, bin, v); // attention, mixing bin nr and index
        }
      }

      m_monObj->setVariable(("cm_" + (std::string)m_PXDModules[i]).c_str(), hh1->GetMean());


      /// TODO: integration intervalls depend on CM default value, this seems to be agreed =10
      outside += hh1->Integral(16, 63);
      // FIXME currently we have to much noise below the line ... thus excluding this to avoid false alarms
      // outside += hh1->Integral(1 /*0*/, 5); /// FIXME we exclude bin 0 as we use it for debugging/timing pixels
      all_outside += outside;
      all += current;
      double dhpc = hh1->GetBinContent(64);
      all_cm += dhpc;
      if (current > 1) {
        error_flag |= (outside / current > 1e-5); /// TODO level might need adjustment
        warn_flag |= (outside / current > 1e-6); /// TODO level might need adjustment
//         error_flag |= (dhpc / current > 1e-5); // DHP Fifo overflow ... might be critical/unrecoverable
//         warn_flag |= (dhpc / current > 1e-6); // DHP Fifo overflow ... might be critical/unrecoverable
      }
    }
  }

  int status = 0;
  {
    m_cCommonMode->cd();
    // not enough Entries
    if (all < 100.) {
      m_cCommonMode->Pad()->SetFillColor(kGray);// Magenta or Gray
      status = 0; // default
    } else {
      /// FIXME: absolute numbers or relative numbers and what is the acceptable limit?
      if (all_outside / all > 1e-5 || /*all_cm / all > 1e-5 ||*/ error_flag) {
        m_cCommonMode->Pad()->SetFillColor(kRed);// Red
        status = 4;
      } else if (all_outside / all > 1e-6 || /*all_cm / all > 1e-6 ||*/ warn_flag) {
        m_cCommonMode->Pad()->SetFillColor(kYellow);// Yellow
        status = 3;
      } else if (all_outside == 0. /*&& all_cm == 0.*/) {
        m_cCommonMode->Pad()->SetFillColor(kGreen);// Green
        status = 2;
      } else { // between 0 and 50 ...
        m_cCommonMode->Pad()->SetFillColor(kWhite);// White
        status = 1;
      }
    }

    if (m_hCommonMode) {
      m_hCommonMode->Draw("colz");
      m_line1->Draw();
      m_line2->Draw();
//     m_line3->Draw();
    }

    auto tt = new TLatex(5.5, 3, "1.3.2 Module is broken, please ignore");
    tt->SetTextAngle(90);// Rotated
    tt->SetTextAlign(12);// Centered
    tt->Draw();

    m_cCommonMode->Modified();
    m_cCommonMode->Update();
  }

  {
    m_cCommonModeDelta->cd();
    // not enough Entries
#if 0
    int status = 0;
    if (all < 100.) {
      m_cCommonModeDelta->Pad()->SetFillColor(kGray);// Magenta or Gray
      status = 0; // default
    } else {
      /// FIXME: absolute numbers or relative numbers and what is the acceptable limit?
      if (all_outside / all > 1e-5 || /*all_cm / all > 1e-5 ||*/ error_flag) {
        m_cCommonModeDelta->Pad()->SetFillColor(kRed);// Red
        status = 4;
      } else if (all_outside / all > 1e-6 || /*all_cm / all > 1e-6 ||*/ warn_flag) {
        m_cCommonModeDelta->Pad()->SetFillColor(kYellow);// Yellow
        status = 3;
      } else if (all_outside == 0. /*&& all_cm == 0.*/) {
        m_cCommonModeDelta->Pad()->SetFillColor(kGreen);// Green
        status = 2;
      } else { // between 0 and 50 ...
        m_cCommonModeDelta->Pad()->SetFillColor(kWhite);// White
        status = 1;
      }
    }
#endif
    if (m_hCommonModeDelta) {
      m_hCommonModeDelta->Draw("colz");
      m_line1->Draw();
      m_line2->Draw();
//     m_line3->Draw();
    }

    auto tt = new TLatex(5.5, 3, "1.3.2 Module is broken, please ignore");
    tt->SetTextAngle(90);// Rotated
    tt->SetTextAlign(12);// Centered
    tt->Draw();

    m_cCommonModeDelta->Modified();
    m_cCommonModeDelta->Update();
  }

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    double data = all > 0 ? (all_outside / all) : 0;
    double data2 = all > 0 ? (all_cm / all) : 0;
    SEVCHK(ca_put(DBR_DOUBLE, mychid[0], (void*)&data), "ca_set failure");
    SEVCHK(ca_put(DBR_INT, mychid[1], (void*)&status), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[2], (void*)&data2), "ca_set failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

void DQMHistAnalysisPXDCMModule::terminate()
{
  B2DEBUG(99, "DQMHistAnalysisPXDCM: terminate called");
  // should delete canvas here, maybe hist, too? Who owns it?
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    for (auto m : mychid) SEVCHK(ca_clear_channel(m), "ca_clear_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

