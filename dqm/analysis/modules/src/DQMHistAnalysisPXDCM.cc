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

  addParam("warnMeanFull", m_warnMeanFull, "warn level for peak position", 2.0);
  addParam("errorMeanFull", m_errorMeanFull, "error level for peak position", 3.0);
  addParam("warnOutsideFull", m_warnOutsideFull, "warn level for outside fraction", 1e-5);
  addParam("errorOutsideFull", m_errorOutsideFull, "error level for outside fraction", 1e-4);
  addParam("upperLineFull", m_upperLineFull, "upper threshold and line for outside fraction", 17);

  addParam("gateMaskModuleList", m_par_module_list, "Module List for Gate Masking");
  addParam("gateMaskGateList", m_par_gate_list, "Gate List for Gate Masking");

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
  m_cCommonModeDelta = new TCanvas((m_histogramDirectoryName + "/c_CommonModeDelta").data());

  m_hCommonMode = new TH2D("hPXDCommonMode", "PXD CommonMode; Module; CommonMode", m_PXDModules.size(), 0, m_PXDModules.size(), 63, 0,
                           63);
  m_hCommonMode->SetDirectory(0);// dont mess with it, this is MY histogram
  m_hCommonMode->SetStats(false);
  m_hCommonModeDelta = new TH2D("hPXDCommonModeAdhoc", "PXD CommonMode Adhoc; Module; CommonMode", m_PXDModules.size(), 0,
                                m_PXDModules.size(), 63, 0, 63);
  m_hCommonModeDelta->SetDirectory(0);// dont mess with it, this is MY histogram
  m_hCommonModeDelta->SetStats(false);
  m_hCommonModeOld = new TH2D("hPXDCommonModeOld", "PXD CommonMode Old; Module; CommonMode", m_PXDModules.size(), 0,
                              m_PXDModules.size(),
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

  if (m_par_module_list.size() != m_par_gate_list.size()) {
    B2FATAL("Parameter list need same length");
    return;
  }
  for (size_t i = 0; i < m_par_module_list.size(); i++) {
    for (auto n : m_par_gate_list[i]) {
      m_masked_gates[VxdID(m_par_module_list[i])].push_back(n);
    }
  }

  /// FIXME were to put the lines depends ...
  m_line1 = new TLine(0, 10, m_PXDModules.size(), 10);
  m_lineA = new TLine(0, m_upperLineAdhoc, m_PXDModules.size(), m_upperLineAdhoc);
  m_lineF = new TLine(0, m_upperLineFull, m_PXDModules.size(), m_upperLineFull);
  m_line1->SetHorizontal(true);
  m_line1->SetLineColor(3);// Green
  m_line1->SetLineWidth(3);
  m_lineA->SetHorizontal(true);
  m_lineA->SetLineColor(1);// Black
  m_lineA->SetLineWidth(3);
  m_lineF->SetHorizontal(true);
  m_lineF->SetLineColor(1);// Black
  m_lineF->SetLineWidth(3);


#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    mychid.resize(4);
    SEVCHK(ca_create_channel((m_pvPrefix + "Outside").data(), NULL, NULL, 10, &mychid[0]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "Status").data(), NULL, NULL, 10, &mychid[1]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "CM63").data(), NULL, NULL, 10, &mychid[2]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "Status_Adhoc").data(), NULL, NULL, 10, &mychid[3]), "ca_create_channel failure");

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

  m_cCommonMode->Clear();
  m_cCommonModeDelta->Clear();
  m_cCommonMode->SetLogz();
  m_cCommonModeDelta->SetLogz();

  // this is needed at least for the "Old" and "Delta" one or update doesnt work
  m_hCommonMode->Reset();
  m_hCommonModeDelta->Reset();
  m_hCommonModeOld->Reset();
}

void DQMHistAnalysisPXDCMModule::event()
{
  double all_outside = 0.0, all = 0.0;
  double all_cm = 0.0;
  bool error_full_flag = false;
  bool warn_full_flag = false;
  bool error_adhoc_flag = false;
  bool warn_adhoc_flag = false;
  bool anyupdate = false;
  if (!m_cCommonMode) return;
  m_hCommonMode->Reset(); // dont sum up!!!

  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    std::string name = "PXDDAQCM_" + (std::string)m_PXDModules[i ];
    // std::replace( name.begin(), name.end(), '.', '_');

    TH1* hh1 = findHist(name);
    if (hh1 == NULL) {
      hh1 = findHist(m_histogramDirectoryName, name);
    }
    if (hh1) {
      double current_full = 0.0;
      double outside_full = 0.0;

      auto nevent = hh1->GetBinContent(0); // misuse underflow as event counter
      double scale = nevent - m_hCommonModeOld->GetBinContent(i + 1, 0); // number of new events for delta
      bool update = scale > m_minEntries ;
      anyupdate |= update;
      if (update) m_hCommonModeOld->SetBinContent(i + 1, 0, nevent);
      if (scale > 0) scale = 1.0 / scale;
      else scale = 1.; // worst case, no events at run start

      auto& gm = m_masked_gates[m_PXDModules[i]];
      // We loop over a 2d histogram!
      // loop CM values
      double dhpc = 0.0;
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
        m_hCommonMode->SetBinContent(i + 1, bin, v); // attention, mixing bin nr and index
        // integration intervalls depend on CM default value, this seems to be agreed =10
        // FIXME currently we have to much noise below the line ... thus excluding this to avoid false alarms
        // outside_full += hh1->Integral(1 /*0*/, 5); /// FIXME we exclude bin 0 as we use it for debugging/timing pixels
        // attention, n bins!
        // we integrate up including value 62 (cm overflow), but not 63 (fifo full)
        if (bin == 63 + 1) { // CM63
          dhpc += v;
        } else { // excluding CM63
          current_full += v;
          if (bin > m_upperLineFull + 1) outside_full += v;
        }
        if (nevent < m_minEntries) {
          m_hCommonModeDelta->SetBinContent(i + 1, bin, v * scale); // attention, mixing bin nr and index
        } else if (update) {
          auto old = m_hCommonModeOld->GetBinContent(i + 1, bin); // attention, mixing bin nr and index
          m_hCommonModeDelta->SetBinContent(i + 1, bin, (v - old)*scale); // attention, mixing bin nr and index
          m_hCommonModeOld->SetBinContent(i + 1, bin, v); // attention, mixing bin nr and index
        }
      }

      all_outside += outside_full;
      all += current_full;
      all_cm += dhpc;
      if (current_full > 1) {
        error_full_flag |= (outside_full / current_full > m_errorOutsideFull);
        warn_full_flag |= (outside_full / current_full > m_warnOutsideFull);
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
        for (int cm_y = m_upperLineAdhoc; cm_y < 64; cm_y++) {
          auto v = m_hCommonModeDelta->GetBinContent(m_hCommonModeDelta->GetBin(i + 1, cm_y + 1));
          entries_adhoc += v;
          outside_adhoc += v;
        }
        if (entries_adhoc > 0) { // ignore 1.3.2
          mean_adhoc /= entries_adhoc; // calculate mean
          // scale <1e-3 == >1000 entries
          auto warn_tmp = scale < 1e-3 && (fabs(10.0 - mean_adhoc) > m_warnMeanAdhoc || outside_adhoc > m_warnOutsideAdhoc);
          warn_adhoc_flag |= warn_tmp;
          auto err_tmp = scale < 1e-3 && (fabs(10.0 - mean_adhoc) > m_errorMeanAdhoc || outside_adhoc > m_errorOutsideAdhoc);
          error_adhoc_flag |= err_tmp;
          m_monObj->setVariable(("cm_" + (std::string)m_PXDModules[i]).c_str(), mean_adhoc);
          if (warn_tmp
              || err_tmp)  B2INFO(name << " Mean " <<  mean_adhoc << " Outside " << outside_adhoc << " " << entries_adhoc << " " << warn_tmp <<
                                    err_tmp);
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

  int status = 0;
  {
    m_cCommonMode->cd();
    // not enough Entries
    if (all < 100.) {
      m_cCommonMode->Pad()->SetFillColor(kGray);// Magenta or Gray
      status = 0; // default
    } else {
      /// use flags set above
      if (all_outside / all > m_errorOutsideFull || error_full_flag) {
        m_cCommonMode->Pad()->SetFillColor(kRed);// Red
        status = 4;
      } else if (all_outside / all > m_warnOutsideFull || warn_full_flag) {
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
      m_lineA->Draw();
    }

    auto tt = new TLatex(5.5, 3, "1.3.2 Module is excluded, please ignore");
    tt->SetTextAngle(90);// Rotated
    tt->SetTextAlign(12);// Centered
    tt->Draw();

    m_cCommonMode->Modified();
    m_cCommonMode->Update();
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
    if (m_useEpics && anyupdate) SEVCHK(ca_put(DBR_INT, mychid[3], (void*)&status_adhoc), "ca_set failure");
#endif
    if (m_hCommonModeDelta) {
      m_hCommonModeDelta->Draw("colz");
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
    for (auto& m : mychid_mean) SEVCHK(ca_clear_channel(m.second), "ca_clear_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

