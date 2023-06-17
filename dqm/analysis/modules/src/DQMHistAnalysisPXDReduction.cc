/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPXDReduction.cc
// Description : Analysis of PXD Reduction
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDReduction.h>
#include <TROOT.h>
#include <vxd/geometry/GeoCache.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDReduction);

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
  addParam("lowarnlimit", m_lowarnlimit, "Mean Reduction Low Warn limit for alarms", 0.99);
  addParam("LowErrorlimit", m_loerrorlimit, "Mean Reduction Low limit for alarms", 0.90);
  addParam("HighWarnlimit", m_hiwarnlimit, "Mean Reduction High Warn limit for alarms", 1.01);
  addParam("HighErrorlimit", m_hierrorlimit, "Mean Reduction High limit for alarms", 1.10);
  addParam("minEntries", m_minEntries, "minimum number of new entries for last time slot", 1000);
  B2DEBUG(1, "DQMHistAnalysisPXDReduction: Constructor done.");
}

DQMHistAnalysisPXDReductionModule::~DQMHistAnalysisPXDReductionModule()
{
#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
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
    if (!hasDeltaPar(m_histogramDirectoryName,
                     "PXDDAQDHEDataReduction_" + (std::string)m_PXDModules[i])) addDeltaPar(m_histogramDirectoryName,
                           "PXDDAQDHEDataReduction_" + (std::string)m_PXDModules[i], HistDelta::c_Entries, m_minEntries,
                           1); // register delta
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
  if (getUseEpics()) {
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
  m_hReduction->Reset(); // dont sum up!!!

#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    // get warn and error limit
    // as the same array as above, we assume chid exists
    struct dbr_ctrl_double tPvData;
    auto r = ca_get(DBR_CTRL_DOUBLE, mychid[1], &tPvData);
    if (r == ECA_NORMAL) r = ca_pend_io(5.0);
    if (r == ECA_NORMAL) {
      if (!std::isnan(tPvData.lower_alarm_limit)
          && tPvData.lower_alarm_limit > 0.0) {
        //m_hLoErrorLine->SetBinContent(i + 1, tPvData.lower_alarm_limit);
        m_loerrorlimit = tPvData.lower_alarm_limit;
      }
      if (!std::isnan(tPvData.lower_warning_limit)
          && tPvData.lower_warning_limit > 0.0) {
        //m_hLoWarnLine->SetBinContent(i + 1, tPvData.lower_warning_limit);
        m_lowarnlimit = tPvData.lower_warning_limit;
      }
      if (!std::isnan(tPvData.upper_alarm_limit)
          && tPvData.upper_alarm_limit > 0.0) {
        //m_hHiErrorLine->SetBinContent(i + 1, tPvData.upper_alarm_limit);
        m_hierrorlimit = tPvData.upper_alarm_limit;
      }
      if (!std::isnan(tPvData.upper_warning_limit)
          && tPvData.upper_warning_limit > 0.0) {
        //m_hHiWarnLine->SetBinContent(i + 1, tPvData.upper_warning_limit);
        m_hiwarnlimit = tPvData.upper_warning_limit;
      }
    } else {
      SEVCHK(r, "ca_get or ca_pend_io failure");
    }
  }
#endif

}

void DQMHistAnalysisPXDReductionModule::event()
{
  if (!m_cReduction) return;

  double ireduction = 0.0;
  int ireductioncnt = 0;

  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    std::string name = "PXDDAQDHEDataReduction_" + (std::string)m_PXDModules[i ];
    // std::replace( name.begin(), name.end(), '.', '_');

    TH1* hh1 = getDelta(name);
    // no inital sampling, we should get lenty of statistics
    if (hh1) {
      auto mean = hh1->GetMean();
      m_hReduction->SetBinContent(i + 1, mean);
    }
  }
  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    auto mean = m_hReduction->GetBinContent(i + 1);
    if (mean > 0) {
      ireduction += mean; // well fit would be better
      ireductioncnt++;
    }
  }

  m_cReduction->cd();

  double value = ireductioncnt > 0 ? ireduction / ireductioncnt : 0;

  int status = 0;
// not enough Entries
  if (ireductioncnt < 15) { // still have to see how to handle masked modules
    status = 0; // Grey
    m_cReduction->Pad()->SetFillColor(kGray);// Magenta or Gray
  } else {
    if (value > m_hierrorlimit || value < m_loerrorlimit) {
      m_cReduction->Pad()->SetFillColor(kRed);// Red
      status = 4;
    } else if (value >  m_hiwarnlimit ||  value < m_lowarnlimit) {
      m_cReduction->Pad()->SetFillColor(kYellow);// Yellow
      status = 3;
    } else {
      m_cReduction->Pad()->SetFillColor(kGreen);// Green
      status = 2;
//   } else {
// we wont use white anymore here
//    m_cReduction->Pad()->SetFillColor(kWhite);// White
//    status = 1; // White
    }
  }

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
  if (getUseEpics() && !getUseEpicsReadOnly()) {
/// doch besser DBR_DOUBLE wg alarms?
    SEVCHK(ca_put(DBR_INT, mychid[0], (void*)&status), "ca_set failure");
    // only update if statistics is reasonable, we dont want "0" drops between runs!
    SEVCHK(ca_put(DBR_DOUBLE, mychid[1], (void*)&value), "ca_set failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

void DQMHistAnalysisPXDReductionModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDReduction: terminate called");
#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    for (auto m : mychid) SEVCHK(ca_clear_channel(m), "ca_clear_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

