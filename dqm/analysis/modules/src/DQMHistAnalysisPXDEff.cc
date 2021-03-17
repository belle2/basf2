//+
// File : DQMHistAnalysisPXDEff.cc
// Description : DQM module, which gives histograms showing the efficiency of PXD sensors
//
// Author : Uwe Gebauer, Bjoern Spruck
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
REG_MODULE(DQMHistAnalysisPXDEff)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDEffModule::DQMHistAnalysisPXDEffModule() : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  //Parameter definition

  //Would be much more elegant to get bin numbers from the saved histograms, but would need to retrieve at least one of them before the initialize function for this
  //Or get one and clone it
  addParam("binsU", m_u_bins, "histogram bins in u direction, needs to be the same as in PXDDQMEfficiency", int(4));
  addParam("binsV", m_v_bins, "histogram bins in v direction, needs to be the same as in PXDDQMEfficiency", int(6));
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms were placed",
           std::string("PXDEFF"));
  addParam("singleHists", m_singleHists, "Also plot one efficiency histogram per module", bool(false));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("DQM:PXD:Eff:"));
  addParam("useEpics", m_useEpics, "useEpics", true);
  addParam("ConfidenceLevel", m_confidence, "Confidence Level for error bars and alarms", 0.9544);
  addParam("WarnLevel", m_warnlevel, "Efficiency Warn Level for alarms", 0.92);
  addParam("ErrorLevel", m_errorlevel, "Efficiency  Level for alarms", 0.90);
  addParam("perModuleAlarm", m_perModuleAlarm, "Alarm level per module", false);
  addParam("minEntries", m_minEntries, "minimum number of new entries for last time slot", 1000);
  B2DEBUG(1, "DQMHistAnalysisPXDEff: Constructor done.");
}

DQMHistAnalysisPXDEffModule::~DQMHistAnalysisPXDEffModule()
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (ca_current_context()) ca_context_destroy();
  }
#endif
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
  //Have been promised that all modules have the same number of pixels, so just take from the first one
  if (m_PXDModules.size() == 0) {
    //This could as well be a B2FATAL, the module won't do anything useful if this happens
    B2WARNING("No PXDModules in Geometry found! Use hard-coded setup.");
    std::vector <string> mod = {
      "1.1.1", "1.1.2", "1.2.1", "1.2.2", "1.3.1", "1.3.2", "1.4.1", "1.4.2",
      "1.5.1", "1.5.2", "1.6.1", "1.6.2", "1.7.1", "1.7.2", "1.8.1", "1.8.2",
      "2.4.1", "2.4.2", "2.5.1", "2.5.2"
    };
    for (auto& it : mod) m_PXDModules.push_back(VxdID(it));
    // set some default size to nu, nv?
  } else {
    VXD::SensorInfoBase cellGetInfo = geo.getSensorInfo(m_PXDModules[0]);
    nu = cellGetInfo.getUCells();
    nv = cellGetInfo.getVCells();
  }

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
  }
#endif


  for (VxdID& aPXDModule : m_PXDModules) {
    TString buff = (std::string)aPXDModule;
    buff.ReplaceAll(".", "_");
#ifdef _BELLE2_EPICS
    if (m_useEpics) {
      auto& my = mychid_eff[aPXDModule];
      SEVCHK(ca_create_channel((m_pvPrefix + buff).Data(), NULL, NULL, 10, &my), "ca_create_channel failure");
      B2WARNING(m_pvPrefix + (std::string)aPXDModule);
    }
#endif
    TString histTitle = "Hit Efficiency on Module " + (std::string)aPXDModule + ";Pixel in U;Pixel in V";
    if (m_singleHists) {
      m_cEffModules[aPXDModule] = new TCanvas((m_histogramDirectoryName + "/c_Eff_").data() + buff);
      m_hEffModules[aPXDModule] = new TEfficiency("HitEff_" + buff, histTitle,
                                                  m_u_bins, -0.5, nu - 0.5, m_v_bins, -0.5, nv - 0.5);
    }
  }

  m_hErrorLine = new TH1F("Errorlimit", "Error Limit", m_PXDModules.size(), 0, m_PXDModules.size());
  m_hWarnLine = new TH1F("Warnlimit", "Warn Limit", m_PXDModules.size(), 0, m_PXDModules.size());
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
  m_hEffAll = new TEfficiency("HitEffAll", "Integrated Efficiency of each module;PXD Module;",
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
  m_hEffAllUpdate = new TEfficiency("HitEffAllUpdate", "Integral and last-updated Efficiency per module;PXD Module;",
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

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    // values per module, see above
    mychid_status.resize(2);
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    SEVCHK(ca_create_channel((m_pvPrefix + "Status").data(), NULL, NULL, 10, &mychid_status[0]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "Overall").data(), NULL, NULL, 10, &mychid_status[1]), "ca_create_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
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

#ifdef _BELLE2_EPICS
    if (m_useEpics) {
      // get warn and error limit
      // as the same array as above, we assume chid exists
      struct dbr_ctrl_double tPvData;
      auto r = ca_get(DBR_CTRL_DOUBLE, mychid_eff[m_PXDModules[i]], &tPvData);


      if (r == ECA_NORMAL) {
        if (!std::isnan(tPvData.lower_alarm_limit)
            && tPvData.lower_alarm_limit > 0.0) {
          m_hErrorLine->SetBinContent(i + 1, tPvData.lower_alarm_limit);
          if (m_perModuleAlarm) m_errorlevelmod[m_PXDModules[i]] = tPvData.lower_alarm_limit;
        }
        if (!std::isnan(tPvData.lower_warning_limit)
            && tPvData.lower_warning_limit > 0.0) {
          m_hWarnLine->SetBinContent(i + 1, tPvData.lower_warning_limit);
          if (m_perModuleAlarm) m_warnlevelmod[m_PXDModules[i]] = tPvData.lower_warning_limit;
        }
      } else {
        SEVCHK(r, "ca_get failure");
      }
    }
#endif

  }
  // Thus histo will contain old content until first update
  m_hEffAllLastTotal->Reset();
  m_hEffAllLastPassed->Reset();

  for (auto single_cmap : m_cEffModules) {
    if (single_cmap.second) single_cmap.second->Clear();
  }

}


void DQMHistAnalysisPXDEffModule::event()
{

  //Save the pointers to create the summary hists later
  std::map<VxdID, TH1*> mapHits;
  std::map<VxdID, TH1*> mapMatches;

  //Count how many of each type of histogram there are for the averaging
  //std::map<std::string, int> typeCounter;

  for (unsigned int i = 1; i <= m_PXDModules.size(); i++) {
    VxdID& aPXDModule = m_PXDModules[i - 1];

    TString buff = (std::string)aPXDModule;
    buff.ReplaceAll(".", "_");

    TH1* Hits, *Matches;
    TString locationHits = "track_hits_" + buff;
    if (m_histogramDirectoryName != "") {
      locationHits = m_histogramDirectoryName + "/" + locationHits;
    }
    Hits = (TH1*)findHist(locationHits.Data());
    TString locationMatches = "matched_cluster_" + buff;
    if (m_histogramDirectoryName != "") {
      locationMatches = m_histogramDirectoryName + "/" + locationMatches;
    }
    Matches = (TH1*)findHist(locationMatches.Data());

    //Finding only one of them should only happen in very strange situations...
    if (Hits == nullptr || Matches == nullptr) {
      B2ERROR("Missing histogram for sensor " << aPXDModule);
      mapHits[aPXDModule] = nullptr;
      mapMatches[aPXDModule] = nullptr;
    } else {
      mapHits[aPXDModule] = Hits;
      mapMatches[aPXDModule] = Matches;
      if (m_singleHists) {
        if (m_cEffModules[aPXDModule] && m_hEffModules[aPXDModule]) {// this check creates them with a nullptr ..bad
          m_hEffModules[aPXDModule]->SetTotalHistogram(*Hits, "f");
          m_hEffModules[aPXDModule]->SetPassedHistogram(*Matches, "f");

          m_cEffModules[aPXDModule]->cd();
          m_hEffModules[aPXDModule]->Draw("colz");
          m_cEffModules[aPXDModule]->Modified();
          m_cEffModules[aPXDModule]->Update();
        }
      }
    }
  }//One-Module histos finished

  double stat_data = 0;
  bool error_flag = false;
  bool warn_flag = false;
  double all = 0.0;

  double imatch = 0.0, ihit = 0.0;
  int ieff = 0;

  std::map <VxdID, bool> updated{}; // init to false
  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    VxdID& aModule = m_PXDModules[i];
    int j = i + 1;

    if (mapHits[aModule] == nullptr || mapMatches[aModule] == nullptr) {
      m_hEffAll->SetPassedEvents(j, 0); // order, otherwise it might happen that SetTotalEvents is NOT filling the value!
      m_hEffAll->SetTotalEvents(j, 0);
    } else {
      double nmatch = mapMatches[aModule]->Integral(); // GetEntries()?
      double nhit = mapHits[aModule]->Integral();
      if (nmatch > 10 && nhit > 10) { // could be zero, too
        imatch += nmatch;
        ihit +=  nhit;
        ieff++; // only count in modules working
        double var_e = nmatch / nhit; // can never be zero
        if (j == 6) continue; // workaround for 1.3.2 module
        m_monObj->setVariable(Form("efficiency_%d_%d_%d", aModule.getLayerNumber(), aModule.getLadderNumber(), aModule.getSensorNumber()),
                              var_e);
      }

      /// TODO: one value per module, and please change to the "delta" instead of integral
      all += ihit;
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

      if (j == 6) continue; // workaround for 1.3.2 module

      // get the errors and check for limits for each bin seperately ...
      /// FIXME: absolute numbers or relative numbers and what is the acceptable limit?

      error_flag |= (ihit > 10)
                    && (m_hEffAll->GetEfficiency(j) + m_hEffAll->GetEfficiencyErrorUp(j) <
                        m_errorlevelmod[aModule]); // error if upper error value is below limit
      warn_flag |= (ihit > 10)
                   && (m_hEffAll->GetEfficiency(j) + m_hEffAll->GetEfficiencyErrorUp(j) <
                       m_warnlevelmod[aModule]); // (and not only the actual eff value)
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
        if (i != 5) { // exclude 1.3.2
          /// check for val > 0.0) { would exclude all zero efficient modules!!!
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

      auto tt = new TLatex(5.5, scale_min, " 1.3.2 Module is excluded, please ignore");
      tt->SetTextAngle(90);// Rotated
      tt->SetTextAlign(12);// Centered
      tt->Draw();

      if (all < 100.) {
        m_cEffAll->Pad()->SetFillColor(kGray);// Magenta or Gray
      } else {
        if (error_flag) {
          m_cEffAll->Pad()->SetFillColor(kRed);// Red
        } else if (warn_flag) {
          m_cEffAll->Pad()->SetFillColor(kYellow);// Yellow
        } else {
          m_cEffAll->Pad()->SetFillColor(kGreen);// Green
          //       m_cEffAll->Pad()->SetFillColor(kWhite);// White
        }
      }

      m_cEffAll->Pad()->SetFrameFillColor(kWhite - 1); // White
      m_cEffAll->Pad()->SetFrameFillStyle(1001);// White
      m_cEffAll->Pad()->Modified();
      m_cEffAll->Pad()->Update();
      m_hWarnLine->Draw("same,hist");
      m_hErrorLine->Draw("same,hist");
    }

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
        if (i != 5) { // exclude 1.3.2
          /// check for val > 0.0) { would exclude all zero efficient modules!!!
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
        ax->Set(m_PXDModules.size() , 0, m_PXDModules.size());
        for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
          TString ModuleName = (std::string)m_PXDModules[i];
          ax->SetBinLabel(i + 1, ModuleName);
        }
      }
#ifdef _BELLE2_EPICS
      if (m_useEpics) {
        for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
          if (updated[m_PXDModules[i]]) {
            Double_t x, y;// we assume that double and Double_t are same!
            gr->GetPoint(i, x, y);
            auto& my = mychid_eff[m_PXDModules[i]];// as the same array as above, we assume it exists
            // we should only write if it was updated!
            SEVCHK(ca_put(DBR_DOUBLE, my, (void*)&y), "ca_set failure");
          }
        }
      }
#endif
      gr->SetLineColor(kBlack);
      gr->SetLineWidth(3);
      gr->SetMarkerStyle(33);
    } else scale_min = 0.0;
    if (gr) gr->Draw("AP");
    if (gr3) gr3->Draw("P");
    auto tt = new TLatex(5.5, scale_min, "1.3.2 Module is excluded, please ignore");
    tt->SetTextSize(0.035);
    tt->SetTextAngle(90);// Rotated
    tt->SetTextAlign(12);// Centered
    tt->Draw();

    if (all < 100.) {
      m_cEffAllUpdate->Pad()->SetFillColor(kGray);// Magenta or Gray
      stat_data = 0.;
    } else {
      if (error_flag) {
        m_cEffAllUpdate->Pad()->SetFillColor(kRed);// Red
        stat_data = 4.;
      } else if (warn_flag) {
        m_cEffAllUpdate->Pad()->SetFillColor(kYellow);// Yellow
        stat_data = 3.;
      } else {
        m_cEffAllUpdate->Pad()->SetFillColor(kGreen);// Green
        stat_data = 2.;
        /// we wont use "white" =1 in this module
        //       m_cEffAllUpdate->Pad()->SetFillColor(kWhite);// White
      }
    }
    m_cEffAllUpdate->Pad()->SetFrameFillColor(kWhite - 1); // White
    m_cEffAllUpdate->Pad()->SetFrameFillStyle(1001);// White
    m_cEffAllUpdate->Pad()->Modified();
    m_cEffAllUpdate->Pad()->Update();
    m_hWarnLine->Draw("same,hist");
    m_hErrorLine->Draw("same,hist");
  }
  m_cEffAllUpdate->Modified();
  m_cEffAllUpdate->Update();


  double var_efficiency = ihit > 0 ? imatch / ihit : 0.0;
  m_monObj->setVariable("efficiency", var_efficiency);
  m_monObj->setVariable("nmodules", ieff);

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    SEVCHK(ca_put(DBR_DOUBLE, mychid_status[0], (void*)&stat_data), "ca_set failure");
    // only update if statistics is reasonable, we dont want "0" drops between runs!
    if (stat_data != 0) SEVCHK(ca_put(DBR_DOUBLE, mychid_status[1], (void*)&var_efficiency), "ca_set failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

void DQMHistAnalysisPXDEffModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff: terminate called");
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    for (auto& m : mychid_status) SEVCHK(ca_clear_channel(m), "ca_clear_channel failure");
    for (auto& m : mychid_eff) SEVCHK(ca_clear_channel(m.second), "ca_clear_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

