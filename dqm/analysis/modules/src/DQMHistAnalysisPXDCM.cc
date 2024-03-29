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

  //Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("PXDCM"));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("DQM:PXD:CommonMode:"));
  addParam("useEpics", m_useEpics, "Whether to update EPICS PVs.", false);
  addParam("minEntries", m_minEntries, "minimum number of new entries for last time slot", 10000);

  addParam("warnMeanAdhoc", m_warnMeanAdhoc, "warn level for peak position", 2.0);
  addParam("errorMeanAdhoc", m_errorMeanAdhoc, "error level for peak position", 3.0);
  addParam("warnOutsideAdhoc", m_warnOutsideAdhoc, "warn level for outside fraction", 1e-5);
  addParam("errorOutsideAdhoc", m_errorOutsideAdhoc, "error level for outside fraction", 1e-4);
  addParam("upperLineAdhoc", m_upperLineAdhoc, "upper threshold and line for outside fraction", 17);

  addParam("gateMaskModuleList", m_parModuleList, "Module List for Gate Masking");
  addParam("gateMaskGateList", m_parGateList, "Gate List for Gate Masking");

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

  m_cCommonModeDelta = new TCanvas((m_histogramDirectoryName + "/c_CommonModeDelta").data());

  m_hCommonModeDelta = new TH2D("hPXDCommonModeAdhoc", "PXD CommonMode Adhoc; Module; CommonMode", m_PXDModules.size(), 0,
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
  m_line1 = new TLine(0, 10, m_PXDModules.size(), 10);
  m_lineA = new TLine(0, m_upperLineAdhoc, m_PXDModules.size(), m_upperLineAdhoc);
  m_line1->SetHorizontal(true);
  m_line1->SetLineColor(3);// Green
  m_line1->SetLineWidth(3);
  m_lineA->SetHorizontal(true);
  m_lineA->SetLineColor(1);// Black
  m_lineA->SetLineWidth(3);


#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    mychid.resize(3);
    SEVCHK(ca_create_channel((m_pvPrefix + "Status_Adhoc").data(), NULL, NULL, 10, &mychid[0]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "Outside").data(), NULL, NULL, 10, &mychid[1]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "CM63").data(), NULL, NULL, 10, &mychid[2]), "ca_create_channel failure");

    for (VxdID& aPXDModule : m_PXDModules) {
      TString buff = (std::string)aPXDModule;
      buff.ReplaceAll(".", "_");
      auto& my = mychid_mean[aPXDModule];
      SEVCHK(ca_create_channel((m_pvPrefix + "Mean:" + buff).Data(), NULL, NULL, 10, &my), "ca_create_channel failure");
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
  B2DEBUG(99, "DQMHistAnalysisPXDCM: initialized.");
}

void DQMHistAnalysisPXDCMModule::beginRun()
{
  B2DEBUG(99, "DQMHistAnalysisPXDCM: beginRun called.");

  m_cCommonModeDelta->Clear();
  m_cCommonModeDelta->SetLogz();

  // this is needed at least for the "Old" and "Delta" one or update doesnt work
  m_hCommonModeDelta->Reset();
}

void DQMHistAnalysisPXDCMModule::event()
{
  double all_outside = 0.0, all = 0.0;
  double all_cm = 0.0;
  bool error_adhoc_flag = false;
  bool warn_adhoc_flag = false;
  bool anyupdate = false;

  auto leg = new TPaveText(0.1, 0.6, 0.90, 0.95, "NDC");
  leg->SetFillStyle(0);
  leg->SetBorderSize(0);

  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    auto modname = (std::string)m_PXDModules[i];
    std::string name = "PXDDAQCM_" + modname;

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
        // integration intervalls depend on CM default value, this seems to be agreed =10
        // FIXME currently we have to much noise below the line ... thus excluding this to avoid false alarms
        // outside_full += hh1->Integral(1 /*0*/, 5); /// FIXME we exclude bin 0 as we use it for debugging/timing pixels
        // attention, n bins!
        // we integrate up including value 62 (cm overflow), but not 63 (fifo full)
        if (bin == 63 + 1) { // CM63
          all_cm += v;
        } else { // excluding CM63
          all += v;
          if (bin > m_upperLineAdhoc + 1) all_outside += v;
        }
        m_hCommonModeDelta->SetBinContent(i + 1, bin, v * scale); // attention, mixing bin nr and index
      }

      if (update) {
        Double_t mean_adhoc = 0.;
        Double_t entries_adhoc = 0.;
        Double_t outside_adhoc = 0.;

        // Attention, Bins
        // we do not need to re-scale it as the scale is the same for all bins
        for (int cm_y = 0; cm_y < m_upperLineAdhoc; cm_y++) {
          auto v = m_hCommonModeDelta->GetBinContent(m_hCommonModeDelta->GetBin(i + 1, cm_y + 1));
          entries_adhoc += v;
          mean_adhoc += v * (cm_y + 1);
        }
        // Attention, Bins
        // We ignore CM63 in outside and overall count
        for (int cm_y = m_upperLineAdhoc; cm_y < 63; cm_y++) {
          auto v = m_hCommonModeDelta->GetBinContent(m_hCommonModeDelta->GetBin(i + 1, cm_y + 1));
          entries_adhoc += v;
          outside_adhoc += v;
        }
        if (entries_adhoc > 0 && scale < 1e-3) { // ignore 1.3.2 and minimum events
          // scale <1e-3 == >1000 events
          mean_adhoc /= entries_adhoc; // calculate mean
          auto warn_tmp_m = fabs(10.0 - mean_adhoc) > m_warnMeanAdhoc;
          auto err_tmp_m = fabs(10.0 - mean_adhoc) > m_errorMeanAdhoc;
          auto warn_tmp_os = outside_adhoc / entries_adhoc > m_warnOutsideAdhoc;
          auto err_tmp_os = outside_adhoc / entries_adhoc > m_errorOutsideAdhoc;
          warn_adhoc_flag |= warn_tmp_m || warn_tmp_os;
          error_adhoc_flag |= err_tmp_m || err_tmp_os;

          if (warn_tmp_m || err_tmp_m) {
            TString tmp;
            tmp.Form("%s: Mean %f", modname.c_str(), mean_adhoc);
            leg->AddText(tmp);
            B2INFO(name << " Mean " <<  mean_adhoc << " " << warn_tmp_m << err_tmp_m);
          }
          if (warn_tmp_os || err_tmp_os) {
            TString tmp;
            tmp.Form("%s: Outside %f %%", modname.c_str(), 100. * outside_adhoc / entries_adhoc);
            leg->AddText(tmp);
            B2INFO(name << " Outside " << outside_adhoc / entries_adhoc << " (" << outside_adhoc << "/" << entries_adhoc << ") " << warn_tmp_os
                   << err_tmp_os);
          }
          m_monObj->setVariable(("cm_" + modname).c_str(), mean_adhoc);
#ifdef _BELLE2_EPICS
          if (m_useEpics) {
            auto my = mychid_mean[m_PXDModules[i]];
            if (my) SEVCHK(ca_put(DBR_DOUBLE, my, (void*)&mean_adhoc), "ca_set failure");
          }
#endif
        }
      }
    }
  }

  {
    int status_adhoc = 0;
    m_cCommonModeDelta->cd();
    // not enough Entries

    if (all < 100.) { // delta cannot be more than all
      m_cCommonModeDelta->Pad()->SetFillColor(kGray);// Magenta or Gray
      status_adhoc = 0; // default
    } else {
      /// use flags set above
      if (error_adhoc_flag) {
        m_cCommonModeDelta->Pad()->SetFillColor(kRed);// Red
        status_adhoc = 4;
      } else if (warn_adhoc_flag) {
        m_cCommonModeDelta->Pad()->SetFillColor(kYellow);// Yellow
        status_adhoc = 3;
      } else {
        m_cCommonModeDelta->Pad()->SetFillColor(kGreen);// Green
        status_adhoc = 2;
        /*      } else { // between 0 and 50 ...
                m_cCommonModeDelta->Pad()->SetFillColor(kWhite);// White
                status_adhoc = 1;*/
      }
    }
#ifdef _BELLE2_EPICS
    if (m_useEpics && anyupdate) {
      double dataoutside = all > 0 ? (all_outside / all) : 0;
      double datacm = all > 0 ? (all_cm / all) : 0;
      SEVCHK(ca_put(DBR_INT, mychid[0], (void*)&status_adhoc), "ca_set failure");
      SEVCHK(ca_put(DBR_DOUBLE, mychid[1], (void*)&dataoutside), "ca_set failure");
      SEVCHK(ca_put(DBR_DOUBLE, mychid[2], (void*)&datacm), "ca_set failure");
      SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
    }
#endif
    if (m_hCommonModeDelta) {
      m_hCommonModeDelta->Draw("colz");
      leg->Draw();
      m_line1->Draw();
      m_lineA->Draw();
    }

    auto tt = new TLatex(5.5, 3, "1.3.2 Module is excluded, please ignore");
    tt->SetTextAngle(90);// Rotated
    tt->SetTextAlign(12);// Centered
    tt->Draw();

    m_cCommonModeDelta->Modified();
    m_cCommonModeDelta->Update();
  }

}

void DQMHistAnalysisPXDCMModule::terminate()
{
  B2DEBUG(99, "DQMHistAnalysisPXDCM: terminate called");
  // should delete canvas here, maybe hist, too? Who owns it?
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    for (auto m : mychid) SEVCHK(ca_clear_channel(m), "ca_clear_channel failure");
    for (auto& m : mychid_mean) SEVCHK(ca_clear_channel(m.second), "ca_clear_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

