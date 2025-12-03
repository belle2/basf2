/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPXDCM.cc
// Description : Analysis of PXD Common Modes
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDCM.h>
#include <TROOT.h>
#include <TLatex.h>
#include <TPaveText.h>
#include <vxd/geometry/GeoCache.h>
#include <framework/core/ModuleParam.templateDetails.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDCM);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDCMModule::DQMHistAnalysisPXDCMModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!
  setDescription("DQM Analysis for PXD Common Mode");

  // Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("PXDDAQ"));
  addParam("minEntries", m_minEntries, "minimum number of new entries for last time slot", 10000);

  addParam("warnMean", m_warnMean, "warn level for peak position", 2.0);
  addParam("errorMean", m_errorMean, "error level for peak position", 3.0);
  addParam("warnOutside", m_warnOutside, "warn level for outside fraction", 1e-5);
  addParam("errorOutside", m_errorOutside, "error level for outside fraction", 1e-4);
  addParam("upperLine", m_upperLine, "upper threshold and line for outside fraction", 17);

  addParam("gateMaskModuleList", m_parModuleList, "Module List for Gate Masking", std::vector <std::string> ());
  addParam("gateMaskGateList", m_parGateList, "Gate List for Gate Masking", std::vector <std::vector<int>>());
  addParam("excluded", m_excluded, "excluded module (indizes starting from 0 to 39)", std::vector<int>());

  B2DEBUG(99, "DQMHistAnalysisPXDCM: Constructor done.");
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
  m_cCommonModeDelta = new TCanvas((m_histogramDirectoryName + "/c_CommonModeDelta").data());

  m_hCommonModeDelta = new TH2D("hPXDCommonMode", "PXD CommonMode ; Module; CommonMode", m_PXDModules.size(), 0,
                                m_PXDModules.size(), 63, 0, 63);
  m_hCommonModeDelta->SetDirectory(0);// dont mess with it, this is MY histogram
  m_hCommonModeDelta->SetStats(false);

  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    TString ModuleName = (std::string)m_PXDModules[i];
    m_hCommonModeDelta->GetXaxis()->SetBinLabel(i + 1, ModuleName);
    addDeltaPar(m_histogramDirectoryName, "PXDDAQCM_" + (std::string)m_PXDModules[i], HistDelta::c_Underflow, m_minEntries,
                1); // register delta
  }
  //Unfortunately this only changes the labels, but can't fill the bins by the VxdIDs
  m_hCommonModeDelta->Draw("colz");

  m_monObj->addCanvas(m_cCommonModeDelta);

  if (m_parModuleList.size() != m_parGateList.size()) {
    B2FATAL("Parameter list need same length");
    return;
  }
  for (size_t i = 0; i < m_parModuleList.size(); i++) {
    for (auto n : m_parGateList[i]) {
      m_maskedGates[VxdID(m_parModuleList[i])].push_back(n);
    }
  }

  /// FIXME were to put the lines depends ...
  m_line10 = new TLine(0, 10, m_PXDModules.size(), 10);
  m_lineOutside = new TLine(0, m_upperLine, m_PXDModules.size(), m_upperLine);
  m_line10->SetHorizontal(true);
  m_line10->SetLineColor(3);// Green
  m_line10->SetLineWidth(3);
  m_lineOutside->SetHorizontal(true);
  m_lineOutside->SetLineColor(1);// Black
  m_lineOutside->SetLineWidth(3);


  registerEpicsPV("PXD:CommonMode:Status", "Status");
  registerEpicsPV("PXD:CommonMode:Outside", "Outside");
  registerEpicsPV("PXD:CommonMode:CM63", "CM63");
  //registerEpicsPV("PXD:CommonMode:CM62", "CM62");

  for (VxdID& aPXDModule : m_PXDModules) {
    auto buff = (std::string)aPXDModule;
    replace(buff.begin(), buff.end(), '.', '_');
    registerEpicsPV("PXD:CommonMode:Mean:" + buff, (std::string)aPXDModule);
  }
  B2DEBUG(99, "DQMHistAnalysisPXDCM: initialized.");
}

void DQMHistAnalysisPXDCMModule::beginRun()
{
  B2DEBUG(99, "DQMHistAnalysisPXDCM: beginRun called.");

  m_cCommonModeDelta->Clear();
  m_cCommonModeDelta->SetLogz();

  // this is needed at least for the "Old" and "Delta" one or update doesn't work
  m_hCommonModeDelta->Reset();
}

void DQMHistAnalysisPXDCMModule::event()
{
  double all_outside = 0.0, all = 0.0;
  double all_cm = 0.0;
  bool error_flag = false;
  bool warn_flag = false;
  bool anyupdate = false;

  static TPaveText* leg = nullptr;

  if (leg == nullptr) {
    leg = new TPaveText(0.1, 0.6, 0.90, 0.95, "NDC");
    leg->SetFillStyle(0);
    leg->SetBorderSize(0);
  } else {
    leg->Clear();
  }

  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    auto modname = (std::string)m_PXDModules[i];
    std::string name = "PXDDAQCM_" + modname;
    bool excluded = find(m_excluded.begin(), m_excluded.end(), i) != m_excluded.end();

    auto hh1 = getDelta(m_histogramDirectoryName, name); // default, only updated
    if (hh1) {
      auto scale = hh1->GetBinContent(0); // misuse underflow as event counter
      bool update = scale >= m_minEntries ; // filter initial sampling
      anyupdate |= update;
      if (scale > 0) scale = 1.0 / scale;
      else scale = 1.; // worst case, no events at run start

      auto& gm = m_maskedGates[m_PXDModules[i]];
      // We loop over a 2d histogram!
      // loop CM values
      for (int bin = 1; bin <= 64; bin++) { // including CM63!!!
        // loop gates*asics
        double v = 0;
        for (int gate = 0; gate < 192; gate++) {
          // attention, gate is not bin nr!
          if (std::find(gm.begin(), gm.end(), gate) == gm.end()) {
            v += hh1->GetBinContent(hh1->GetBin(gate + 1 + 192 * 0, bin)) +
                 hh1->GetBinContent(hh1->GetBin(gate + 1 + 192 * 1, bin)) +
                 hh1->GetBinContent(hh1->GetBin(gate + 1 + 192 * 2, bin)) +
                 hh1->GetBinContent(hh1->GetBin(gate + 1 + 192 * 3, bin));
          }
        }
        // integration intervals depend on CM default value, this seems to be agreed =10
        // FIXME currently we have to much noise below the line ... thus excluding this to avoid false alarms
        // outside_full += hh1->Integral(1 /*0*/, 5); /// FIXME we exclude bin 0 as we use it for debugging/timing pixels
        // attention, n bins!
        // we integrate up including value 62 (cm overflow), but not 63 (fifo full)
        if (!excluded) {
          if (bin == 63 + 1) { // CM63
            all_cm += v;
          } else { // excluding CM63
            all += v;
            if (bin > m_upperLine + 1) all_outside += v;
          }
        }
        m_hCommonModeDelta->SetBinContent(i + 1, bin, v * scale); // attention, mixing bin nr and index
      }

      if (update) {
        Double_t mean = 0.;
        Double_t entries = 0.;
        Double_t outside = 0.;

        // Attention, Bins
        // we do not need to re-scale it as the scale is the same for all bins
        for (int cm_y = 0; cm_y < m_upperLine; cm_y++) {
          auto v = m_hCommonModeDelta->GetBinContent(m_hCommonModeDelta->GetBin(i + 1, cm_y + 1));
          entries += v;
          mean += v * (cm_y + 1);
        }
        // Attention, Bins
        // We ignore CM63 in outside and overall count
        for (int cm_y = m_upperLine; cm_y < 63; cm_y++) {
          auto v = m_hCommonModeDelta->GetBinContent(m_hCommonModeDelta->GetBin(i + 1, cm_y + 1));
          entries += v;
          outside += v;
        }
        if (entries > 0 and scale < 1e-3) {  // ignore modules with minimum events
          // scale <1e-3 == >1000 events
          mean /= entries; // calculate mean
          auto warn_tmp_m = fabs(10.0 - mean) > m_warnMean;
          auto err_tmp_m = fabs(10.0 - mean) > m_errorMean;
          auto warn_tmp_os = outside / entries > m_warnOutside;
          auto err_tmp_os = outside / entries > m_errorOutside;
          if (not excluded) {
            warn_flag |= warn_tmp_m or warn_tmp_os;
            error_flag |= err_tmp_m or err_tmp_os;

            if (warn_tmp_m or err_tmp_m) {
              TString tmp;
              tmp.Form("%s: Mean %f", modname.c_str(), mean);
              leg->AddText(tmp);
              B2INFO(name << " Mean " <<  mean << " " << warn_tmp_m << err_tmp_m);
            }
            if (warn_tmp_os or err_tmp_os) {
              TString tmp;
              tmp.Form("%s: Outside %f %%", modname.c_str(), 100. * outside / entries);
              leg->AddText(tmp);
              B2INFO(name << " Outside " << outside / entries << " (" << outside << "/" << entries << ") " << warn_tmp_os
                     << err_tmp_os);
            }
          }
          m_monObj->setVariable(("cm_" + modname).c_str(), mean);

          setEpicsPV((std::string)m_PXDModules[i], mean);
        }
      }
    }
  }

  auto status = makeStatus(all >= 10000, warn_flag, error_flag);

  m_cCommonModeDelta->cd();
  colorizeCanvas(m_cCommonModeDelta, status);

  if (anyupdate) {
    double dataoutside = all > 0 ? (all_outside / all) : 0;
    double datacm = all > 0 ? (all_cm / all) : 0;
    setEpicsPV("Status", status);
    setEpicsPV("Outside", dataoutside);
    setEpicsPV("CM63", datacm);
  }
  if (m_hCommonModeDelta) {
    m_hCommonModeDelta->Draw("colz");
    leg->Draw();
    m_line10->Draw();
    m_lineOutside->Draw();
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

  UpdateCanvas(m_cCommonModeDelta);
  m_cCommonModeDelta->Modified();
  m_cCommonModeDelta->Update();
}

void DQMHistAnalysisPXDCMModule::terminate()
{
  B2DEBUG(99, "DQMHistAnalysisPXDCM: terminate called");

  if (m_cCommonModeDelta) delete m_cCommonModeDelta;
  if (m_hCommonModeDelta) delete m_hCommonModeDelta;
  if (m_line10) delete m_line10;
  if (m_lineOutside) delete m_lineOutside;
}

