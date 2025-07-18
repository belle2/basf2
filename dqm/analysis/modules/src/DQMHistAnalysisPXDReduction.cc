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
#include <TLatex.h>
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
  setDescription("PXD DQM analysis module for ONSEN Data Reduction Monitoring");

  // Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("PXDDAQ"));
  addParam("LowerWarnLimit", m_meanLowerWarn, "Mean Reduction Low limit for warning", double(NAN)); // default is NAN =disable
  addParam("LowerErrorLimit", m_meanLowerAlarm, "Mean Reduction Low limit for alarms", double(NAN)); // default is NAN =disable
  addParam("UpperWarnLimit", m_meanUpperWarn, "Mean Reduction High limit for warning", double(NAN)); // default is NAN =disable
  addParam("UpperErrorLimit", m_meanUpperAlarm, "Mean Reduction High limit for alarms", double(NAN)); // default is NAN =disable
  addParam("minEntries", m_minEntries, "minimum number of new entries for last time slot", 1000);
  addParam("excluded", m_excluded, "excluded module (indizes starting from 0 to 39)", std::vector<int>());
  B2DEBUG(1, "DQMHistAnalysisPXDReduction: Constructor done.");
}

DQMHistAnalysisPXDReductionModule::~DQMHistAnalysisPXDReductionModule()
{
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

  if (m_PXDModules.size() == 0) {
    // Backup if no geometry is present (testing...)
    B2WARNING("No PXDModules in Geometry found! Use hard-coded setup.");
    std::vector <string> mod = {
      "1.1.1", "1.1.2", "1.2.1", "1.2.2", "1.3.1", "1.3.2", "1.4.1", "1.4.2",
      "1.5.1", "1.5.2", "1.6.1", "1.6.2", "1.7.1", "1.7.2", "1.8.1", "1.8.2",
      "2.1.1", "2.1.2", "2.2.1", "2.2.2", "2.3.1", "2.3.2", "2.4.1", "2.4.2",
      "2.5.1", "2.5.2", "2.6.1", "2.6.2", "2.7.1", "2.7.2", "2.8.1", "2.8.2",
      "2.9.1", "2.9.2", "2.10.1", "2.10.2", "2.11.1", "2.11.2", "2.12.1", "2.12.2"
    };
    for (auto& it : mod) m_PXDModules.push_back(VxdID(it));
  }

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

  m_meanLine = new TLine(0, 10, m_PXDModules.size(), 10);
  m_meanUpperWarnLine = new TLine(0, 16, m_PXDModules.size(), 16);
  m_meanLowerWarnLine = new TLine(0, 0.9, m_PXDModules.size(), 0.9);
  m_meanUpperAlarmLine = new TLine(0, 20, m_PXDModules.size(), 20);
  m_meanLowerAlarmLine = new TLine(0, 0.5, m_PXDModules.size(), 0.5);
  m_meanLine->SetHorizontal(true);
  m_meanLine->SetLineColor(kBlue);
  m_meanLine->SetLineWidth(3);
  m_meanUpperWarnLine->SetHorizontal(true);
  m_meanUpperWarnLine->SetLineColor(c_ColorWarning + 2);
  m_meanUpperWarnLine->SetLineWidth(3);
  m_meanLowerWarnLine->SetHorizontal(true);
  m_meanLowerWarnLine->SetLineColor(c_ColorWarning + 2);
  m_meanLowerWarnLine->SetLineWidth(3);
  m_meanUpperAlarmLine->SetHorizontal(true);
  m_meanUpperAlarmLine->SetLineColor(c_ColorError + 2);
  m_meanUpperAlarmLine->SetLineWidth(3);
  m_meanLowerAlarmLine->SetHorizontal(true);
  m_meanLowerAlarmLine->SetLineColor(c_ColorError + 2);
  m_meanLowerAlarmLine->SetLineWidth(3);

  registerEpicsPV("PXD:Red:Status", "Status");
  registerEpicsPV("PXD:Red:Value", "Value");
}


void DQMHistAnalysisPXDReductionModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDReduction: beginRun called.");

  m_cReduction->Clear();
  m_hReduction->Reset(); // dont sum up!!!
  colorizeCanvas(m_cReduction, c_StatusTooFew);

  // override with limits from EPICS. if they are set
  requestLimitsFromEpicsPVs("Value", m_meanLowerAlarm, m_meanLowerWarn, m_meanUpperWarn, m_meanUpperAlarm);

  if (!std::isnan(m_meanLowerAlarm)) {
    m_meanLowerAlarmLine->SetY1(m_meanLowerAlarm);
    m_meanLowerAlarmLine->SetY2(m_meanLowerAlarm);
  }
  if (!std::isnan(m_meanLowerWarn)) {
    m_meanLowerWarnLine->SetY1(m_meanLowerWarn);
    m_meanLowerWarnLine->SetY2(m_meanLowerWarn);
  }
  if (!std::isnan(m_meanUpperWarn)) {
    m_meanUpperWarnLine->SetY1(m_meanUpperWarn);
    m_meanUpperWarnLine->SetY2(m_meanUpperWarn);
  }
  if (!std::isnan(m_meanUpperAlarm)) {
    m_meanUpperAlarmLine->SetY1(m_meanUpperAlarm);
    m_meanUpperAlarmLine->SetY2(m_meanUpperAlarm);
  }
}

void DQMHistAnalysisPXDReductionModule::event()
{
  if (!m_cReduction) return;

  double ireduction = 0.0;
  int ireductioncnt = 0;

  bool anyupdate = false;
  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    std::string name = "PXDDAQDHEDataReduction_" + (std::string)m_PXDModules[i ];
    // std::replace( name.begin(), name.end(), '.', '_');

    TH1* hh1 = getDelta(m_histogramDirectoryName, name);
    // no initial sampling, we should get plenty of statistics
    if (hh1) {
      auto mean = hh1->GetMean();
      m_hReduction->SetBinContent(i + 1, mean);
      anyupdate = true;
    }
  }

  if (!anyupdate) return; // nothing new -> no update

  // calculate the mean of the mean
  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    // ignore modules in exclude list
    if (std::find(m_excluded.begin(), m_excluded.end(), i) != m_excluded.end()) continue;
    auto mean = m_hReduction->GetBinContent(i + 1);
    if (mean > 0) { // only for valid values
      ireduction += mean; // well fit would be better
      ireductioncnt++;
    }
  }

  m_cReduction->cd();

  double value = ireductioncnt > 0 ? ireduction / ireductioncnt : 0;

  // if any if NaN, the comparison is false
  auto stat_data = makeStatus(ireductioncnt >= 15,
                              value > m_meanUpperWarn || value < m_meanLowerWarn,
                              value > m_meanUpperAlarm || value < m_meanLowerAlarm);

  if (m_hReduction) {
    m_hReduction->Draw("");
    if (stat_data != c_StatusTooFew) {
      m_meanLine->SetY1(value);
      m_meanLine->SetY2(value); // aka SetHorizontal
      m_meanLine->Draw();
    }
    if (!std::isnan(m_meanLowerAlarm)) m_meanLowerAlarmLine->Draw();
    if (!std::isnan(m_meanLowerWarn)) m_meanLowerWarnLine->Draw();
    if (!std::isnan(m_meanUpperWarn)) m_meanUpperWarnLine->Draw();
    if (!std::isnan(m_meanUpperAlarm)) m_meanUpperAlarmLine->Draw();
    for (auto& it : m_excluded) {
      static std::map <int, TLatex*> ltmap;
      auto tt = ltmap[it];
      if (!tt) {
        tt = new TLatex(it + 0.5, 0, (" " + std::string(m_PXDModules[it]) + " Module is excluded, please ignore").c_str());
        tt->SetTextSize(0.035);
        tt->SetTextAngle(90);// Rotated
        tt->SetTextAlign(12);// Centered
        ltmap[it] = tt;
      }
      tt->Draw();
    }
  }

  m_monObj->setVariable("reduction", value);

  colorizeCanvas(m_cReduction, stat_data);
  UpdateCanvas(m_cReduction);
  m_cReduction->Modified();
  m_cReduction->Update();

  // better only update if statistics is reasonable, we dont want "0" drops between runs!
  setEpicsPV("Status", stat_data);
  setEpicsPV("Value", value);
}

void DQMHistAnalysisPXDReductionModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDReduction: terminate called");

  if (m_cReduction) delete m_cReduction;
  if (m_hReduction) delete m_hReduction;

  if (m_meanLine) delete m_meanLine;
  if (m_meanUpperWarnLine) delete m_meanUpperWarnLine;
  if (m_meanLowerWarnLine) delete m_meanLowerWarnLine;
  if (m_meanUpperAlarmLine) delete m_meanUpperAlarmLine;
  if (m_meanLowerAlarmLine) delete m_meanLowerAlarmLine;
}

