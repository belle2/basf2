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
  addParam("ConfidenceLevel", m_confidence, "Confidence Level for error bars and alarms", 0.99);
  addParam("WarnLevel", m_warnlevel, "Efficiency Warn Level for alarms", 0.92);
  addParam("ErrorLevel", m_errorlevel, "Efficiency  Level for alarms", 0.90);
  addParam("perModuleAlarm", m_perModuleAlarm, "Alarm level per module", true);
  addParam("alarmAdhoc", m_alarmAdhoc, "Generate Alarm from adhoc values", true);
  addParam("minEntries", m_minEntries, "minimum number of new entries for last time slot", 1000);
  addParam("excluded", m_excluded, "the list of excluded modules, indices from 0 to 39", std::vector<int>());
  B2DEBUG(1, "DQMHistAnalysisPXDEff: Constructor done.");
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
    m_eEffModules[aPXDModule] = new TEfficiency(("ePXDHitEff_" + buff).c_str(), histTitle,
                                                m_u_bins, -0.5, nu - 0.5, m_v_bins, -0.5, nv - 0.5);
  }

  m_cInnerMap = new TCanvas((m_histogramDirectoryName + "/c_InnerMap").data());
  m_cOuterMap = new TCanvas((m_histogramDirectoryName + "/c_OuterMap").data());
  m_hInnerMap = new TH2F("hEffInnerMap", "hEffInnerMap", m_u_bins * 8, 0, m_u_bins * 8,  m_v_bins * 2, 0, m_v_bins * 2);
  m_hOuterMap = new TH2F("hEffOuterMap", "hEffOuterMap", m_u_bins * 12, 0, m_u_bins * 12,  m_v_bins * 2, 0, m_v_bins * 2);

  m_nrxbins = m_PXDModules.size() + 3; // Modules + L1 + L2 + All
  m_hErrorLine = new TH1F("hPXDErrorlimit", "Error Limit", m_nrxbins, 0, m_nrxbins);
  m_hWarnLine = new TH1F("hPXDWarnlimit", "Warn Limit", m_nrxbins, 0, m_nrxbins);
  for (int i = 0; i < (int)m_nrxbins; i++) {
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
  m_eEffAll = new TEfficiency("ePXDHitEffAll", "PXD Integrated Efficiency of each module;PXD Module;", m_nrxbins, 0, m_nrxbins);
  m_eEffAll->SetConfidenceLevel(m_confidence);
  m_eEffAll->Paint("AP");
  m_hEffAllLastTotal = m_eEffAll->GetCopyTotalHisto();
  m_hEffAllLastPassed = m_eEffAll->GetCopyPassedHisto();

  setLabels(m_eEffAll->GetPaintedGraph());

  m_cEffAllUpdate = new TCanvas((m_histogramDirectoryName + "/c_EffAllUp").data());
  m_eEffAllUpdate = new TEfficiency("ePXDHitEffAllUpdate", "PXD Integral and last-updated Efficiency per module;PXD Module;",
                                    m_nrxbins, 0, m_nrxbins);
  m_eEffAllUpdate->SetConfidenceLevel(m_confidence);

  m_eEffAllUpdate->Paint("AP");
  setLabels(m_eEffAllUpdate->GetPaintedGraph());

  m_monObj->addCanvas(m_cEffAll);
  m_monObj->addCanvas(m_cEffAllUpdate);

  registerEpicsPV("PXD:Eff:Status", "Status");
  registerEpicsPV("PXD:Eff:Overall", "All");
  registerEpicsPV("PXD:Eff:L1", "L1");
  registerEpicsPV("PXD:Eff:L2", "L2");
  B2DEBUG(1, "DQMHistAnalysisPXDEff: initialized.");
}


void DQMHistAnalysisPXDEffModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff: beginRun called.");

  // Clear all used canvases
  m_cEffAll->Clear();
  m_cEffAllUpdate->Clear();
  m_cInnerMap->Clear();
  m_cOuterMap->Clear();
  for (auto single_cmap : m_cEffModules) {
    if (single_cmap.second) single_cmap.second->Clear();
  }

  // The 2d Efficiency maps (m_eEffModules[]) per module are not cleared, but re-created each update
  // also they are only drawn to Canvas on update, thus no clear is needed here

  // Reset TEfficiency and get (new) alarm limits from PVs
  // no way to reset TEfficiency, do it bin by bin
  for (int i = 0; i < m_nrxbins; i++) {
    int bin = i + 1;
    m_eEffAll->SetPassedEvents(bin, 0); // order, otherwise it might happen that SetTotalEvents is NOT filling the value!
    m_eEffAll->SetTotalEvents(bin, 0);
    m_eEffAllUpdate->SetPassedEvents(bin, 0); // otherwise it might happen that SetTotalEvents is NOT filling the value!
    m_eEffAllUpdate->SetTotalEvents(bin, 0);

    if (i < int(m_PXDModules.size())) { // only for modules
      m_warnlevelmod[m_PXDModules[i]] = m_warnlevel;
      m_errorlevelmod[m_PXDModules[i]] = m_errorlevel;

      // get warn and error limit
      // as the same array as above, we assume chid exists
      double dummy, loerr = 0, lowarn = 0;
      if (requestLimitsFromEpicsPVs((std::string)m_PXDModules[i], loerr, lowarn, dummy, dummy)) {
        m_hErrorLine->SetBinContent(bin, loerr);
        if (m_perModuleAlarm) m_errorlevelmod[m_PXDModules[i]] = loerr;
        m_hWarnLine->SetBinContent(bin, lowarn);
        if (m_perModuleAlarm) m_warnlevelmod[m_PXDModules[i]] = lowarn;
      }
    }
  }
  {
    double dummy, loerr = 0, lowarn = 0;
    m_warnlevelmod["L1"] = m_warnlevel;
    m_errorlevelmod["L1"] = m_errorlevel;
    if (requestLimitsFromEpicsPVs("L1", loerr, lowarn, dummy, dummy)) {
      m_hErrorLine->SetBinContent(m_PXDModules.size() + 1, loerr);
      if (m_perModuleAlarm) m_errorlevelmod["L1"] = loerr;
      m_hWarnLine->SetBinContent(m_PXDModules.size() + 1, lowarn);
      if (m_perModuleAlarm) m_warnlevelmod["L1"] = lowarn;
    }
    m_warnlevelmod["L2"] = m_warnlevel;
    m_errorlevelmod["L2"] = m_errorlevel;
    if (requestLimitsFromEpicsPVs("L2", loerr, lowarn, dummy, dummy)) {
      m_hErrorLine->SetBinContent(m_PXDModules.size() + 2, loerr);
      if (m_perModuleAlarm) m_errorlevelmod["L2"] = loerr;
      m_hWarnLine->SetBinContent(m_PXDModules.size() + 2, lowarn);
      if (m_perModuleAlarm) m_warnlevelmod["L2"] = lowarn;
    }
    m_warnlevelmod["All"] = m_warnlevel;
    m_errorlevelmod["All"] = m_errorlevel;
    if (requestLimitsFromEpicsPVs("All", loerr, lowarn, dummy, dummy)) {
      m_hErrorLine->SetBinContent(m_PXDModules.size() + 3, loerr);
      if (m_perModuleAlarm) m_errorlevelmod["All"] = loerr;
      m_hWarnLine->SetBinContent(m_PXDModules.size() + 3, lowarn);
      if (m_perModuleAlarm) m_warnlevelmod["All"] = lowarn;
    }
  }

  // Clear all remaining Histograms (e.g. for our private delta histogramming)
  m_hEffAllLastTotal->Reset();
  m_hEffAllLastPassed->Reset();
  m_hInnerMap->Reset();
  m_hOuterMap->Reset();
}

bool DQMHistAnalysisPXDEffModule::updateEffBins(int bin, int nhit, int nmatch, int minentries)
{
  m_eEffAll->SetPassedEvents(bin, 0); // otherwise it might happen that SetTotalEvents is NOT filling the value!
  m_eEffAll->SetTotalEvents(bin, nhit);
  m_eEffAll->SetPassedEvents(bin, nmatch);

  if (nhit < minentries) {
    // update the first entries directly (short runs)
    m_eEffAllUpdate->SetPassedEvents(bin, 0); // otherwise it might happen that SetTotalEvents is NOT filling the value!
    m_eEffAllUpdate->SetTotalEvents(bin, nhit);
    m_eEffAllUpdate->SetPassedEvents(bin, nmatch);
    m_hEffAllLastTotal->SetBinContent(bin, nhit);
    m_hEffAllLastPassed->SetBinContent(bin, nmatch);
    return true;
  } else if (nhit - m_hEffAllLastTotal->GetBinContent(bin) > minentries) {
    m_eEffAllUpdate->SetPassedEvents(bin, 0); // otherwise it might happen that SetTotalEvents is NOT filling the value!
    m_eEffAllUpdate->SetTotalEvents(bin, nhit - m_hEffAllLastTotal->GetBinContent(bin));
    m_eEffAllUpdate->SetPassedEvents(bin, nmatch - m_hEffAllLastPassed->GetBinContent(bin));
    m_hEffAllLastTotal->SetBinContent(bin, nhit);
    m_hEffAllLastPassed->SetBinContent(bin, nmatch);
    return true;
  }// else
  return false;
}

bool DQMHistAnalysisPXDEffModule::check_warn_level(int bin, std::string name)
{
  bool warn_flag = (m_eEffAll->GetEfficiency(bin) + m_eEffAll->GetEfficiencyErrorUp(bin) <
                    m_warnlevelmod[name]); // (and not only the actual eff value)
  if (m_alarmAdhoc) {
    warn_flag |= (m_eEffAllUpdate->GetEfficiency(bin) + m_eEffAllUpdate->GetEfficiencyErrorUp(bin) <
                  m_warnlevelmod[name]); // (and not only the actual eff value)
  }
  return warn_flag;
}

bool DQMHistAnalysisPXDEffModule::check_error_level(int bin, std::string name)
{
  bool error_flag = (m_eEffAll->GetEfficiency(bin) + m_eEffAll->GetEfficiencyErrorUp(bin) <
                     m_errorlevelmod[name]); // error if upper error value is below limit
  if (m_alarmAdhoc) {
    error_flag |= (m_eEffAllUpdate->GetEfficiency(bin) + m_eEffAllUpdate->GetEfficiencyErrorUp(bin) <
                   m_errorlevelmod[name]); // error if upper error value is below limit
  }
  return error_flag;
}

void DQMHistAnalysisPXDEffModule::setLabels(TGraphAsymmErrors* gr)
{
  if (gr) {
    auto ax = gr->GetXaxis();
    if (ax) {
      ax->Set(m_nrxbins, 0, m_nrxbins);
      for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
        TString ModuleName = (std::string)m_PXDModules[i];
        ax->SetBinLabel(i + 1, ModuleName);
      }
      ax->SetBinLabel(m_PXDModules.size() + 1, "L1");
      ax->SetBinLabel(m_PXDModules.size() + 2, "L2");
      ax->SetBinLabel(m_PXDModules.size() + 3, "All");
    }
  }
}

void DQMHistAnalysisPXDEffModule::event()
{
  {
    // First create some 2d overview of efficiency for all modules
    // This is not taken into account for efficiency calculation as
    // there may be update glitches dues to separate histograms
    // The histograms
    bool updateinner = false, updateouter = false;
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

      auto Hits = findHist(locationHits, true);// check if updated
      auto Matches = findHist(locationMatches, true);// check if updated

      if (Hits == nullptr && Matches == nullptr) continue; // none updated

      if (Hits == nullptr) Hits = findHist(locationHits); // actually, this should not happen ...
      if (Matches == nullptr) Matches = findHist(locationMatches); // ... as updates should coincide

      // Finding only one of them should only happen in very strange situations... still better check
      if (Hits && Matches) {
        if (m_cEffModules[aPXDModule] && m_eEffModules[aPXDModule]) {// this check creates them with a nullptr ..bad
          m_eEffModules[aPXDModule]->SetTotalHistogram(*Hits, "f");
          m_eEffModules[aPXDModule]->SetPassedHistogram(*Matches, "f");

          m_cEffModules[aPXDModule]->cd();
          m_eEffModules[aPXDModule]->Paint("colz"); // not Draw, enforce to create GetPaintedHistogram?
          m_eEffModules[aPXDModule]->Draw("colz"); // but Draw needed to export Canvas!
          m_cEffModules[aPXDModule]->Modified();
          m_cEffModules[aPXDModule]->Update();
          UpdateCanvas(m_cEffModules[aPXDModule]);

          auto h = m_eEffModules[aPXDModule]->GetPaintedHistogram();
          int s = (2 - aPXDModule.getSensorNumber()) * m_v_bins;
          int l = (aPXDModule.getLadderNumber() - 1) * m_u_bins;
          if (m_hInnerMap && aPXDModule.getLayerNumber() == 1) {
            updateinner = true;
            for (int u = 0; u < m_u_bins; u++) {
              for (int v = 0; v < m_v_bins; v++) {
                auto b = h->GetBin(u + 1, v + 1);
                m_hInnerMap->Fill(u + l, v + s, h->GetBinContent(b));
              }
            }
          }
          if (m_hOuterMap && aPXDModule.getLayerNumber() == 2) {
            updateouter = true;
            for (int u = 0; u < m_u_bins; u++) {
              for (int v = 0; v < m_v_bins; v++) {
                auto b = h->GetBin(u + 1, v + 1);
                m_hOuterMap->Fill(u + l, v + s, h->GetBinContent(b));
              }
            }
          }
        }
      } else {
        B2WARNING("only one plot upd " << aPXDModule);
      }
    }
    // Single-Module histos + 2d overview finished. now draw overviews
    if (updateinner) {
      m_cInnerMap->cd();
      if (m_hInnerMap) m_hInnerMap->Draw("colz");
      m_cInnerMap->Modified();
      m_cInnerMap->Update();
      UpdateCanvas(m_cInnerMap);
    }
    if (updateouter) {
      m_cOuterMap->cd();
      if (m_hOuterMap) m_hOuterMap->Draw("colz");
      m_cOuterMap->Modified();
      m_cOuterMap->Update();
      UpdateCanvas(m_cOuterMap);
    }
    // 3d overview done
  }


// Now, calculate and update efficiency.
// Only histogram is used for numerator AND denominatorto to have an atomic update.
// (avoid possible update glitches from daq/dqm framework side)
// The bins per module are read out and filled into an TEfficiency as total and passed events into bin
// Summaries for L1, L2, Overall are added, too
  auto Combined = findHist(m_histogramDirectoryName, "PXD_Eff_combined", true);// only if updated

  if (Combined) {
    // only if histogram was changed

    EStatus stat_data = c_StatusTooFew;
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
      int bin = i + 1; // bin nr is index +1

      VxdID& aModule = m_PXDModules[i];
      double nmatch = Combined->GetBinContent(i * 2 + 2);
      double nhit = Combined->GetBinContent(i * 2 + 1);

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

      if (nhit >= m_minEntries) { // dont update if there is nothing to calculate
        ieff++; // only count in modules with significant stat
        double var_e = nmatch / nhit; // can never be zero
        m_monObj->setVariable(Form("efficiency_%d_%d_%d", aModule.getLayerNumber(), aModule.getLadderNumber(), aModule.getSensorNumber()),
                              var_e);
      }

      /// TODO: one value per module, and please change to the "delta" instead of integral
      all += nhit;

      updated[aModule] = updateEffBins(bin, nhit, nmatch, m_minEntries);

      // workaround for excluded module
      if (std::find(m_excluded.begin(), m_excluded.end(), i) != m_excluded.end()) continue;

      // get the errors and check for limits for each bin separately ...

      if (nhit >= m_minEntries) {
        error_flag |= check_error_level(bin, aModule);
        warn_flag |= check_warn_level(bin, aModule);
      }
    }

    updateEffBins(m_PXDModules.size() + 1, ihitL1, imatchL1, m_minEntries * 8);
    if (ihitL1 >= m_minEntries) {
      error_flag |= check_error_level(m_PXDModules.size() + 1, "L1");
      warn_flag |= check_warn_level(m_PXDModules.size() + 1, "L1");
    }
    updateEffBins(m_PXDModules.size() + 2, ihitL2, imatchL2, m_minEntries * 12);
    if (ihitL2 >= m_minEntries) {
      error_flag |= check_error_level(m_PXDModules.size() + 2, "L2");
      warn_flag |= check_warn_level(m_PXDModules.size() + 2, "L2");
    }
    updateEffBins(m_PXDModules.size() + 3, ihit, imatch, m_minEntries * 20);
    if (ihit >= m_minEntries) {
      error_flag |= check_error_level(m_PXDModules.size() + 3, "All");
      warn_flag |= check_warn_level(m_PXDModules.size() + 3, "All");
    }

    {
      m_cEffAll->cd();
      m_cEffAll->cd(0);
      m_eEffAll->Paint("AP");
      m_cEffAll->Clear();
      m_cEffAll->cd(0);

      auto gr = m_eEffAll->GetPaintedGraph();
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
        auto ay = gr->GetYaxis();
        if (ay) ay->SetRangeUser(scale_min, 1.0);
        setLabels(gr);

        gr->SetLineColor(4);
        gr->SetLineWidth(2);
        gr->SetMarkerStyle(8);

        gr->Draw("AP");

        for (auto& it : m_excluded) {
          static std::map <int, TLatex*> ltmap;
          auto tt = ltmap[it];
          if (!tt) {
            tt = new TLatex(it + 0.5, scale_min, (" " + std::string(m_PXDModules[it]) + " Module is excluded, please ignore").c_str());
            tt->SetTextSize(0.035);
            tt->SetTextAngle(90);// Rotated
            tt->SetTextAlign(12);// Centered
            ltmap[it] = tt;
          } else {
            tt->SetY(scale_min);
          }
          tt->Draw();
        }


        EStatus all_stat = makeStatus(all >= m_minEntries, warn_flag, error_flag);
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
      m_eEffAllUpdate->Paint("AP");
      m_cEffAllUpdate->Clear();
      m_cEffAllUpdate->cd(0);

      auto gr = m_eEffAllUpdate->GetPaintedGraph();
      auto gr3 = (TGraphAsymmErrors*) m_eEffAll->GetPaintedGraph()->Clone();
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
        auto ay = gr->GetYaxis();
        if (ay) ay->SetRangeUser(scale_min, 1.0);
        setLabels(gr);

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
        std::map <int, TLatex*> ltmap;
        auto tt = ltmap[it];
        if (!tt) {
          tt = new TLatex(it + 0.5, scale_min, (" " + std::string(m_PXDModules[it]) + " Module is excluded, please ignore").c_str());
          tt->SetTextSize(0.035);
          tt->SetTextAngle(90);// Rotated
          tt->SetTextAlign(12);// Centered
        } else {
          tt->SetY(scale_min);
        }
        tt->Draw();
      }

      stat_data = makeStatus(all >= m_minEntries, warn_flag, error_flag);
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
    if (stat_data != c_StatusTooFew) {
      setEpicsPV("All", var_efficiency);
      setEpicsPV("L1", var_efficiencyL1);
      setEpicsPV("L2", var_efficiencyL2);
    }
  }
}

void DQMHistAnalysisPXDEffModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff: terminate called");

  for (VxdID& aPXDModule : m_PXDModules) {
    if (m_cEffModules[aPXDModule]) delete m_cEffModules[aPXDModule];
    if (m_eEffModules[aPXDModule]) delete m_eEffModules[aPXDModule];
  }

  if (m_hEffAllLastTotal) delete m_hEffAllLastTotal;
  if (m_hEffAllLastPassed) delete m_hEffAllLastPassed;

  if (m_cInnerMap) delete m_cInnerMap;
  if (m_cOuterMap) delete m_cOuterMap;
  if (m_hInnerMap) delete m_hInnerMap;
  if (m_hOuterMap) delete m_hOuterMap;

  if (m_hErrorLine) delete m_hErrorLine;
  if (m_hWarnLine) delete m_hWarnLine;

  if (m_cEffAll) delete m_cEffAll;
  if (m_eEffAll) delete m_eEffAll;

  if (m_cEffAllUpdate) delete m_cEffAllUpdate;
  if (m_eEffAllUpdate) delete m_eEffAllUpdate;
}

