/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPXDEff.cc
// Description : DQM module, which gives histograms showing the efficiency of PXD sensors
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDEff.h>
#include <TROOT.h>
#include <TLatex.h>
#include <TGraphAsymmErrors.h>
#include <vxd/geometry/GeoCache.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDEff);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDEffModule::DQMHistAnalysisPXDEffModule() : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!
  setDescription("DQM Analysis for PXD Efficiency");

  // Parameter definition

  // Would be much more elegant to get bin numbers from the saved histograms, but would need to retrieve at least one of them before the initialize function for this
  // Or get one and clone it
  addParam("binsU", m_u_bins, "histogram bins in u direction, needs to be the same as in PXDDQMEfficiency", int(16));
  addParam("binsV", m_v_bins, "histogram bins in v direction, needs to be the same as in PXDDQMEfficiency", int(48));
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms were placed",
           std::string("PXDEFF"));
  addParam("ConfidenceLevel", m_confidence, "Confidence Level for error bars and alarms", 0.9544);
  addParam("WarnLevel", m_warnlevel, "Efficiency Warn Level for alarms", 0.92);
  addParam("ErrorLevel", m_errorlevel, "Efficiency  Level for alarms", 0.90);
  addParam("perModuleAlarm", m_perModuleAlarm, "Alarm level per module", true);
  addParam("alarmAdhoc", m_alarmAdhoc, "Generate Alarm from adhoc values", true);
  addParam("minEntries", m_minEntries, "minimum number of new entries for last time slot", 1000);
  addParam("excluded", m_excluded, "the list of excluded modules, indices from 0 to 39");
  B2DEBUG(1, "DQMHistAnalysisPXDEff: Constructor done.");
}

DQMHistAnalysisPXDEffModule::~DQMHistAnalysisPXDEffModule()
{
}

void DQMHistAnalysisPXDEffModule::initialize()
{
  B2DEBUG(99, "DQMHistAnalysisPXDEffModule: initialized.");

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

  int nu = 1;//If this does not get overwritten, the histograms will anyway never contain anything useful
  int nv = 1;
  if (m_PXDModules.size() == 0) {
    // This could as well be a B2FATAL, the module won't do anything useful if this happens
    B2WARNING("No PXDModules in Geometry found! Use hard-coded setup.");
    std::vector <string> mod = {
      "1.1.1", "1.1.2", "1.2.1", "1.2.2", "1.3.1", "1.3.2", "1.4.1", "1.4.2",
      "1.5.1", "1.5.2", "1.6.1", "1.6.2", "1.7.1", "1.7.2", "1.8.1", "1.8.2",
      "2.1.1", "2.1.2", "2.2.1", "2.2.2", "2.3.1", "2.3.2", "2.4.1", "2.4.2",
      "2.5.1", "2.5.2", "2.6.1", "2.6.2", "2.7.1", "2.7.2", "2.8.1", "2.8.2",
      "2.9.1", "2.9.2", "2.10.1", "2.10.2", "2.11.1", "2.11.2", "2.12.1", "2.12.2"
    };
    for (auto& it : mod) m_PXDModules.push_back(VxdID(it));
    // set some default size to nu, nv?
  } else {
    // Have been promised that all modules have the same number of pixels, so just take from the first one
    VXD::SensorInfoBase cellGetInfo = geo.getSensorInfo(m_PXDModules[0]);
    nu = cellGetInfo.getUCells();
    nv = cellGetInfo.getVCells();
  }

  for (VxdID& aPXDModule : m_PXDModules) {
    auto buff = (std::string)aPXDModule;
    replace(buff.begin(), buff.end(), '.', '_');
    registerEpicsPV("PXD:Eff:" + buff, (std::string)aPXDModule);

    TString histTitle = "PXD Hit Efficiency on Module " + (std::string)aPXDModule + ";Pixel in U;Pixel in V";
    m_cEffModules[aPXDModule] = new TCanvas((m_histogramDirectoryName + "/c_Eff_" + buff).c_str());
    m_hEffModules[aPXDModule] = new TEfficiency(("ePXDHitEff_" + buff).c_str(), histTitle,
                                                m_u_bins, -0.5, nu - 0.5, m_v_bins, -0.5, nv - 0.5);
  }

  m_cInnerMap = new TCanvas((m_histogramDirectoryName + "/c_InnerMap").data());
  m_cOuterMap = new TCanvas((m_histogramDirectoryName + "/c_OuterMap").data());
  m_hInnerMap = new TH2F("hEffInnerMap", "hEffInnerMap", m_u_bins * 8, 0, m_u_bins * 8,  m_v_bins * 2, 0, m_v_bins * 2);
  m_hOuterMap = new TH2F("hEffOuterMap", "hEffOuterMap", m_u_bins * 12, 0, m_u_bins * 12,  m_v_bins * 2, 0, m_v_bins * 2);

  m_hErrorLine = new TH1F("hPXDErrorlimit", "Error Limit", m_PXDModules.size(), 0, m_PXDModules.size());
  m_hWarnLine = new TH1F("hPXDWarnlimit", "Warn Limit", m_PXDModules.size(), 0, m_PXDModules.size());
  for (int i = 0; i < (int)m_PXDModules.size(); i++) {
    m_hErrorLine->SetBinContent(i + 1, m_errorlevel);
    m_hWarnLine->SetBinContent(i + 1, m_warnlevel);
  }
  m_hWarnLine->SetLineColor(kOrange - 3);
  m_hWarnLine->SetLineWidth(3);
  m_hWarnLine->SetLineStyle(4);
  m_hErrorLine->SetLineColor(kRed + 3);
  m_hErrorLine->SetLineWidth(3);
  m_hErrorLine->SetLineStyle(7);

  //One bin for each module in the geometry, one histogram for each layer
  m_cEffAll = new TCanvas((m_histogramDirectoryName + "/c_EffAll").data());
  m_hEffAll = new TEfficiency("ePXDHitEffAll", "PXD Integrated Efficiency of each module;PXD Module;",
                              m_PXDModules.size(), 0, m_PXDModules.size());
  m_hEffAll->SetConfidenceLevel(m_confidence);
  m_hEffAll->Paint("AP");
  m_hEffAllLastTotal = m_hEffAll->GetCopyTotalHisto();
  m_hEffAllLastPassed = m_hEffAll->GetCopyPassedHisto();

  {
    auto gr = m_hEffAll->GetPaintedGraph();

    if (gr) {
      auto ax = gr->GetXaxis();
      if (ax) {
        ax->Set(m_PXDModules.size(), 0, m_PXDModules.size());
        for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
          TString ModuleName = (std::string)m_PXDModules[i];
          ax->SetBinLabel(i + 1, ModuleName);
        }
      }
    }
  }

  m_cEffAllUpdate = new TCanvas((m_histogramDirectoryName + "/c_EffAllUp").data());
  m_hEffAllUpdate = new TEfficiency("ePXDHitEffAllUpdate", "PXD Integral and last-updated Efficiency per module;PXD Module;",
                                    m_PXDModules.size(), 0, m_PXDModules.size());
  m_hEffAllUpdate->SetConfidenceLevel(m_confidence);

  {
    auto gr = m_hEffAllUpdate->GetPaintedGraph();

    if (gr) {
      auto ax = gr->GetXaxis();
      if (ax) {
        ax->Set(m_PXDModules.size(), 0, m_PXDModules.size());
        for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
          TString ModuleName = (std::string)m_PXDModules[i];
          ax->SetBinLabel(i + 1, ModuleName);
        }
      }
    }
  }

  m_monObj->addCanvas(m_cEffAll);
  m_monObj->addCanvas(m_cEffAllUpdate);

  registerEpicsPV("PXD:Eff:Status", "Status");
  registerEpicsPV("PXD:Eff:Overall", "Overall");
  registerEpicsPV("PXD:Eff:L1", "L1");
  registerEpicsPV("PXD:Eff:L2", "L2");
  B2DEBUG(1, "DQMHistAnalysisPXDEff: initialized.");
}


void DQMHistAnalysisPXDEffModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff: beginRun called.");

  m_cEffAll->Clear();
  m_cEffAllUpdate->Clear();

  // no way to reset TEfficiency, do it bin by bin
  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    int j = i + 1;
    m_hEffAll->SetPassedEvents(j, 0); // order, otherwise it might happen that SetTotalEvents is NOT filling the value!
    m_hEffAll->SetTotalEvents(j, 0);
    m_hEffAllUpdate->SetPassedEvents(j, 0); // otherwise it might happen that SetTotalEvents is NOT filling the value!
    m_hEffAllUpdate->SetTotalEvents(j, 0);

    m_warnlevelmod[m_PXDModules[i]] = m_warnlevel;
    m_errorlevelmod[m_PXDModules[i]] = m_errorlevel;

    // get warn and error limit
    // as the same array as above, we assume chid exists
    double dummy, loerr = 0, lowarn = 0;
    if (requestLimitsFromEpicsPVs((std::string)m_PXDModules[i], loerr, lowarn, dummy, dummy)) {
      m_hErrorLine->SetBinContent(i + 1, loerr);
      if (m_perModuleAlarm) m_errorlevelmod[m_PXDModules[i]] = loerr;
      m_hWarnLine->SetBinContent(i + 1, lowarn);
      if (m_perModuleAlarm) m_warnlevelmod[m_PXDModules[i]] = lowarn;
    }

  }
  // Thus histo will contain old content until first update
  m_hEffAllLastTotal->Reset();
  m_hEffAllLastPassed->Reset();

  for (auto single_cmap : m_cEffModules) {
    if (single_cmap.second) single_cmap.second->Clear();
  }

  m_hInnerMap->Reset();
  m_hOuterMap->Reset();
}


void DQMHistAnalysisPXDEffModule::event()
{
  for (auto aPXDModule : m_PXDModules) {
    auto buff = (std::string)aPXDModule;
    replace(buff.begin(), buff.end(), '.', '_');

    std::string locationHits = "track_hits_" + buff;
    if (m_histogramDirectoryName != "") {
      locationHits = m_histogramDirectoryName + "/" + locationHits;
    }
    std::string locationMatches = "matched_cluster_" + buff;
    if (m_histogramDirectoryName != "") {
      locationMatches = m_histogramDirectoryName + "/" + locationMatches;
    }

    auto Hits = (TH1*)findHist(locationHits, true);// check if updated
    auto Matches = (TH1*)findHist(locationMatches, true);// check if updated

    if (Hits == nullptr && Matches == nullptr) continue; // none updated

    if (Hits == nullptr) Hits = (TH1*)findHist(locationHits); // actually, this should not happen ...
    if (Matches == nullptr) Matches = (TH1*)findHist(locationMatches); // ... as updates should coincide

    // Finding only one of them should only happen in very strange situations... still better check
    if (Hits && Matches) {
      if (m_cEffModules[aPXDModule] && m_hEffModules[aPXDModule]) {// this check creates them with a nullptr ..bad
        m_hEffModules[aPXDModule]->SetTotalHistogram(*Hits, "f");
        m_hEffModules[aPXDModule]->SetPassedHistogram(*Matches, "f");

        m_cEffModules[aPXDModule]->cd();
        m_hEffModules[aPXDModule]->Paint("colz"); // not Draw, enforce to create GetPaintedHistogram?
        m_cEffModules[aPXDModule]->Modified();
        m_cEffModules[aPXDModule]->Update();

        auto h = m_hEffModules[aPXDModule]->GetPaintedHistogram();
        int s = (2 - aPXDModule.getSensorNumber()) * m_v_bins;
        int l = (aPXDModule.getLadderNumber() - 1) * m_u_bins;
        if (m_hInnerMap && aPXDModule.getLayerNumber() == 1) {
          for (int u = 0; u < m_u_bins; u++) {
            for (int v = 0; v < m_v_bins; v++) {
              auto b = h->GetBin(u + 1, v + 1);
              m_hInnerMap->Fill(u + l, v + s, h->GetBinContent(b));
            }
          }
        }
        if (m_hOuterMap && aPXDModule.getLayerNumber() == 2) {
          for (int u = 0; u < m_u_bins; u++) {
            for (int v = 0; v < m_v_bins; v++) {
              auto b = h->GetBin(u + 1, v + 1);
              m_hOuterMap->Fill(u + l, v + s, h->GetBinContent(b));
            }
          }
        }
      }
    }
  }
  // Single-Module histos + 2d overview finished. now draw overviews
  m_cInnerMap->cd();
  if (m_hInnerMap) m_hInnerMap->Draw("colz");
  m_cInnerMap->Modified();
  m_cInnerMap->Update();
  m_cOuterMap->cd();
  if (m_hOuterMap) m_hOuterMap->Draw("colz");
  m_cOuterMap->Modified();
  m_cOuterMap->Update();
  UpdateCanvas(m_cInnerMap->GetName());
  UpdateCanvas(m_cOuterMap->GetName());
  // overview done


  // Change: We now use one histogram for hits and matches to make
  // sure that we have an atomic update which is otherwise not
  // guaranteed by DQM framework
  std::string combinedHistname = "PXD_Eff_combined";
  if (m_histogramDirectoryName != "") {
    combinedHistname = m_histogramDirectoryName + "/" + combinedHistname;
  }
  auto Combined = (TH1*)findHist(combinedHistname, true);// only if updated

  if (Combined == nullptr) return; // histogram was not changed, thus no update

  EStatus stat_data = c_TooFew;
  bool error_flag = false;
  bool warn_flag = false;
  double all = 0.0;

  double imatch = 0.0, ihit = 0.0;
  double imatchL1 = 0.0, ihitL1 = 0.0;
  double imatchL2 = 0.0, ihitL2 = 0.0;
  int ieff = 0; // count number of modules with useful stytistics

  std::map <VxdID, bool> updated{}; // init to false, keep track of updated histograms
  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    // workaround for excluded module
    if (std::find(m_excluded.begin(), m_excluded.end(), i) != m_excluded.end()) continue;
    // excluded modules are not counted at all!
    int j = i + 1;

    VxdID& aModule = m_PXDModules[i];
    double nmatch = Combined->GetBinContent(i * 2 + 2);
    double nhit = Combined->GetBinContent(i * 2 + 1);
    if (nmatch > 10 && nhit > 10) { // could be zero, too
      imatch += nmatch;
      ihit +=  nhit;
      // check layer
      if (i >= 16) {
        imatchL2 += nmatch;
        ihitL2 +=  nhit;
      } else {
        imatchL1 += nmatch;
        ihitL1 +=  nhit;
      }

      ieff++; // only count in modules working
      double var_e = nmatch / nhit; // can never be zero

      m_monObj->setVariable(Form("efficiency_%d_%d_%d", aModule.getLayerNumber(), aModule.getLadderNumber(), aModule.getSensorNumber()),
                            var_e);
    }

    /// TODO: one value per module, and please change to the "delta" instead of integral
    all += nhit;
    m_hEffAll->SetPassedEvents(j, 0); // otherwise it might happen that SetTotalEvents is NOT filling the value!
    m_hEffAll->SetTotalEvents(j, nhit);
    m_hEffAll->SetPassedEvents(j, nmatch);

    if (nhit < m_minEntries) {
      // update the first entries directly (short runs)
      m_hEffAllUpdate->SetPassedEvents(j, 0); // otherwise it might happen that SetTotalEvents is NOT filling the value!
      m_hEffAllUpdate->SetTotalEvents(j, nhit);
      m_hEffAllUpdate->SetPassedEvents(j, nmatch);
      m_hEffAllLastTotal->SetBinContent(j, nhit);
      m_hEffAllLastPassed->SetBinContent(j, nmatch);
      updated[aModule] = true;
    } else if (nhit - m_hEffAllLastTotal->GetBinContent(j) > m_minEntries) {
      m_hEffAllUpdate->SetPassedEvents(j, 0); // otherwise it might happen that SetTotalEvents is NOT filling the value!
      m_hEffAllUpdate->SetTotalEvents(j, nhit - m_hEffAllLastTotal->GetBinContent(j));
      m_hEffAllUpdate->SetPassedEvents(j, nmatch - m_hEffAllLastPassed->GetBinContent(j));
      m_hEffAllLastTotal->SetBinContent(j, nhit);
      m_hEffAllLastPassed->SetBinContent(j, nmatch);
      updated[aModule] = true;
    }

    // workaround for excluded module
    if (std::find(m_excluded.begin(), m_excluded.end(), i) != m_excluded.end()) continue;

    // get the errors and check for limits for each bin seperately ...

    if (nhit > 50) {
      error_flag |= (m_hEffAll->GetEfficiency(j) + m_hEffAll->GetEfficiencyErrorUp(j) <
                     m_errorlevelmod[aModule]); // error if upper error value is below limit
      warn_flag |= (m_hEffAll->GetEfficiency(j) + m_hEffAll->GetEfficiencyErrorUp(j) <
                    m_warnlevelmod[aModule]); // (and not only the actual eff value)
      if (m_alarmAdhoc) {
        error_flag |= (m_hEffAllUpdate->GetEfficiency(j) + m_hEffAllUpdate->GetEfficiencyErrorUp(j) <
                       m_errorlevelmod[aModule]); // error if upper error value is below limit
        warn_flag |= (m_hEffAllUpdate->GetEfficiency(j) + m_hEffAllUpdate->GetEfficiencyErrorUp(j) <
                      m_warnlevelmod[aModule]); // (and not only the actual eff value)
      }
    }
  }

  {
    m_cEffAll->cd();
    m_cEffAll->cd(0);
    m_hEffAll->Paint("AP");
    m_cEffAll->Clear();
    m_cEffAll->cd(0);

    auto gr = m_hEffAll->GetPaintedGraph();
    if (gr) {
      double scale_min = 1.0;
      for (int i = 0; i < gr->GetN(); i++) {
        gr->SetPointEXhigh(i, 0.);
        gr->SetPointEXlow(i, 0.);
        // this has to be done first, as it will recalc Min/Max and destroy axis
        Double_t x, y;
        gr->GetPoint(i, x, y);
        gr->SetPoint(i, x - 0.01, y); // workaround for jsroot bug (fixed upstream)
        auto val = y - gr->GetErrorYlow(i); // Error is relative to value
        if (std::find(m_excluded.begin(), m_excluded.end(), i) == m_excluded.end()) {
          // scale update only for included module
          if (scale_min > val) scale_min = val;
        }
      }
      if (scale_min == 1.0) scale_min = 0.0;
      if (scale_min > 0.9) scale_min = 0.9;
      gr->SetMinimum(0);
      gr->SetMaximum(m_PXDModules.size());
      auto ay = gr->GetYaxis();
      if (ay) ay->SetRangeUser(scale_min, 1.0);
      auto ax = gr->GetXaxis();
      if (ax) {
        ax->Set(m_PXDModules.size(), 0, m_PXDModules.size());
        for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
          TString ModuleName = (std::string)m_PXDModules[i];
          ax->SetBinLabel(i + 1, ModuleName);
        }
      }

      gr->SetLineColor(4);
      gr->SetLineWidth(2);
      gr->SetMarkerStyle(8);

      gr->Draw("AP");

      for (auto& it : m_excluded) {
        auto tt = new TLatex(it + 0.5, scale_min, (" " + std::string(m_PXDModules[it]) + " Module is excluded, please ignore").c_str());
        tt->SetTextAngle(90);// Rotated
        tt->SetTextAlign(12);// Centered
        tt->Draw();
      }


      EStatus all_stat = makeStatus(all >= 100., warn_flag, error_flag);
      colorizeCanvas(m_cEffAll, all_stat);

      m_hWarnLine->Draw("same,hist");
      m_hErrorLine->Draw("same,hist");
    }

    UpdateCanvas(m_cEffAll->GetName());
    m_cEffAll->Modified();
    m_cEffAll->Update();
  }

  {
    m_cEffAllUpdate->cd();
    m_hEffAllUpdate->Paint("AP");
    m_cEffAllUpdate->Clear();
    m_cEffAllUpdate->cd(0);

    auto gr = m_hEffAllUpdate->GetPaintedGraph();
    auto gr3 = (TGraphAsymmErrors*) m_hEffAll->GetPaintedGraph()->Clone();
    if (gr3) {
      for (int i = 0; i < gr3->GetN(); i++) {
        Double_t x, y;
        gr3->GetPoint(i, x, y);
        gr3->SetPoint(i, x + 0.2, y);
      }
    }

    double scale_min = 1.0;
    if (gr) {
      for (int i = 0; i < gr->GetN(); i++) {
        gr->SetPointEXhigh(i, 0.);
        gr->SetPointEXlow(i, 0.);
        // this has to be done first, as it will recalc Min/Max and destroy axis
        Double_t x, y;
        gr->GetPoint(i, x, y);
        gr->SetPoint(i, x - 0.2, y); // shift a bit if in same plot
        auto val = y - gr->GetErrorYlow(i); // Error is relative to value
        if (std::find(m_excluded.begin(), m_excluded.end(), i) == m_excluded.end()) {
          // skip scale update only for included modules
          if (scale_min > val) scale_min = val;
        }
      }
      if (scale_min == 1.0) scale_min = 0.0;
      if (scale_min > 0.9) scale_min = 0.9;
      gr->SetMinimum(0);
      gr->SetMaximum(m_PXDModules.size());
      auto ay = gr->GetYaxis();
      if (ay) ay->SetRangeUser(scale_min, 1.0);
      auto ax = gr->GetXaxis();
      if (ax) {
        ax->Set(m_PXDModules.size(), 0, m_PXDModules.size());
        for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
          TString ModuleName = (std::string)m_PXDModules[i];
          ax->SetBinLabel(i + 1, ModuleName);
        }
      }
      for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
        if (updated[m_PXDModules[i]]) {
          // we should only write if it was updated!
          Double_t x, y;// we assume that double and Double_t are same!
          gr->GetPoint(i, x, y);
          setEpicsPV((std::string)m_PXDModules[i], y);
        }
      }

      gr->SetLineColor(kBlack);
      gr->SetLineWidth(3);
      gr->SetMarkerStyle(33);
      gr->Draw("AP");
    } else scale_min = 0.0;
    if (gr3) gr3->Draw("P"); // both in one plot

    for (auto& it : m_excluded) {
      auto tt = new TLatex(it + 0.5, scale_min, (" " + std::string(m_PXDModules[it]) + " Module is excluded, please ignore").c_str());
      tt->SetTextSize(0.035);
      tt->SetTextAngle(90);// Rotated
      tt->SetTextAlign(12);// Centered
      tt->Draw();
    }


    stat_data = makeStatus(all >= 100., warn_flag, error_flag);
    colorizeCanvas(m_cEffAllUpdate, stat_data);

    m_hWarnLine->Draw("same,hist");
    m_hErrorLine->Draw("same,hist");
  }
  UpdateCanvas(m_cEffAllUpdate->GetName());
  m_cEffAllUpdate->Modified();
  m_cEffAllUpdate->Update();

  double var_efficiency = ihit > 0 ? imatch / ihit : 0.0;
  double var_efficiencyL1 = ihitL1 > 0 ? imatchL1 / ihitL1 : 0.0;
  double var_efficiencyL2 = ihitL2 > 0 ? imatchL2 / ihitL2 : 0.0;

  m_monObj->setVariable("efficiency", var_efficiency);
  m_monObj->setVariable("efficiencyL1", var_efficiencyL1);
  m_monObj->setVariable("efficiencyL2", var_efficiencyL2);
  m_monObj->setVariable("nmodules", ieff);

  setEpicsPV("Status", stat_data);
  // only update if statistics is reasonable, we dont want "0" drops between runs!
  if (stat_data != 0) {
    setEpicsPV("Overall", var_efficiency);
    setEpicsPV("L1", var_efficiencyL1);
    setEpicsPV("L2", var_efficiencyL2);
  }
}

void DQMHistAnalysisPXDEffModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff: terminate called");
}

