//+
// File : DQMHistAnalysisPXDEff.cc
// Description : DQM module, which gives histograms showing the efficiency of PXD sensors
//
// Author : Uwe Gebauer
// based on work from B. Spruck
// based on work from Tomoyuki Konno, Tokyo Metropolitan Univerisity
// Date : someday
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
  addParam("ConfidenceLevel", m_confidence, "Confidence Level for error bars and alarms", 0.9544);
  addParam("WarnLevel", m_warnlevel, "Efficiency Warn Level for alarms", 0.92);
  addParam("ErrorLevel", m_errorlevel, "Efficiency  Level for alarms", 0.90);
  addParam("minEntries", m_minEntries, "minimum number of new entries for last time slot", 1000);
  B2DEBUG(1, "DQMHistAnalysisPXDEff: Constructor done.");
}

DQMHistAnalysisPXDEffModule::~DQMHistAnalysisPXDEffModule()
{
#ifdef _BELLE2_EPICS
  if (ca_current_context()) ca_context_destroy();
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
    // B2DEBUG(20,"VXD " << aVxdID);
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
    B2ERROR("No PXDModules found! Can't really do anything useful now...");
    // set some default size to nu, nv?
  } else {
    VXD::SensorInfoBase cellGetInfo = geo.getSensorInfo(m_PXDModules[0]);
    nu = cellGetInfo.getUCells();
    nv = cellGetInfo.getVCells();
  }

  for (VxdID& aPXDModule : m_PXDModules) {
    TString buff = (std::string)aPXDModule;
    buff.ReplaceAll(".", "_");
    TString histTitle = "Hit Efficiency on Module " + (std::string)aPXDModule + ";Pixel in U;Pixel in V";
    if (m_singleHists) {
      m_cEffModules[aPXDModule] = new TCanvas((m_histogramDirectoryName + "/c_Eff_").data() + buff);
      m_hEffModules[aPXDModule] = new TEfficiency("HitEff_" + buff, histTitle,
                                                  m_u_bins, -0.5, nu - 0.5, m_v_bins, -0.5, nv - 0.5);
    }
  }

  //One bin for each module in the geometry, one histogram for each layer
  m_cEffAll = new TCanvas((m_histogramDirectoryName + "/c_EffAll").data());
  m_cEffAllUpdate = new TCanvas((m_histogramDirectoryName + "/c_EffAllUp").data());

  m_hEffAll = new TEfficiency("HitEffAll", "Integrated Efficiency of each module;PXD Module;",
                              m_PXDModules.size(), 0, m_PXDModules.size());
  m_hEffAllUpdate = new TEfficiency("HitEffAllUpdate", "Up-to-date Efficiency of each module;PXD Module;",
                                    m_PXDModules.size(), 0, m_PXDModules.size());

  m_hEffAllLastTotal = m_hEffAll->GetCopyTotalHisto();
  m_hEffAllLastPassed = m_hEffAll->GetCopyPassedHisto();

  m_hEffAll->SetConfidenceLevel(m_confidence);
  m_hEffAllUpdate->SetConfidenceLevel(m_confidence);

//   m_hEffAll->GetYaxis()->SetRangeUser(0, 1.05);
  m_hEffAll->Paint("AP");

  auto gr = m_hEffAll->GetPaintedGraph();

  if (gr) {
    auto ax = gr->GetXaxis();
    if (ax) {
      ax->Set(m_PXDModules.size(), 0, m_PXDModules.size());
      for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
        TString ModuleName = (std::string)m_PXDModules[i];
        ax->SetBinLabel(i + 1, ModuleName);
        B2RESULT(ModuleName);
      }
    }
  }
  //Unfortunately this only changes the labels, but can't fill the bins by the VxdIDs
  m_line_warn = new TLine(0, m_warnlevel, m_PXDModules.size(), m_warnlevel);
  m_line_error = new TLine(0, m_errorlevel, m_PXDModules.size(), m_errorlevel);
  m_line_warn->SetHorizontal(true);
  m_line_warn->SetLineColor(kOrange - 3);
  m_line_warn->SetLineWidth(3);
  m_line_warn->SetLineStyle(4);
  m_line_error->SetHorizontal(true);
  m_line_error->SetLineColor(kRed + 3);
  m_line_error->SetLineWidth(3);
  m_line_error->SetLineStyle(7);

#ifdef _BELLE2_EPICS
  if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
  SEVCHK(ca_create_channel((m_pvPrefix + "Status").data(), NULL, NULL, 10, &mychid), "ca_create_channel failure");
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
  B2DEBUG(1, "DQMHistAnalysisPXDEff: initialized.");
}


void DQMHistAnalysisPXDEffModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff: beginRun called.");

  // no way to reset TEfficiency
  // Thus histo will contain old content until first update
  m_hEffAllLastTotal->Reset();
  m_hEffAllLastPassed->Reset();
  m_cEffAll->Clear();

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

  bool error_flag = false;
  bool warn_flag = false;
  double all = 0.0;

  double imatch = 0.0, ihit = 0.0;
  int ieff = 0;
//   int ccnt = 1;

  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    VxdID& aModule = m_PXDModules[i];
    int j = i + 1;

    if (mapHits[aModule] == nullptr || mapMatches[aModule] == nullptr) {
      m_hEffAll->SetTotalEvents(j, 0);
      m_hEffAll->SetPassedEvents(j, 0);
    } else {
      double nmatch = mapMatches[aModule]->Integral(); // GetEntries()?
      double nhit = mapHits[aModule]->Integral();
      if (nmatch > 10 && nhit > 10) { // could be zero, too
        imatch += nmatch;
        ihit +=  nhit;
        ieff++; // only count in modules working
        double var_e = nmatch / nhit; // can never be zero
        if (j == 6) continue; // wrkaround for 1.3.2 module
        m_monObj->setVariable(Form("efficiency_%d_%d_%d", aModule.getLayerNumber(), aModule.getLadderNumber(), aModule.getSensorNumber()),
                              var_e);
      }

      all += ihit;
      m_hEffAll->SetTotalEvents(j, nhit);
      m_hEffAll->SetPassedEvents(j, nmatch);

      if (m_hEffAllLastTotal->GetBinContent(j) + m_minEntries < nhit) {
        m_hEffAllUpdate->SetTotalEvents(j, nhit - m_hEffAllLastTotal->GetBinContent(j));
        m_hEffAllUpdate->SetPassedEvents(j, nmatch - m_hEffAllLastPassed->GetBinContent(j));
        m_hEffAllLastTotal->SetBinContent(j, nhit);
        m_hEffAllLastPassed->SetBinContent(j, nmatch);
      }

      if (j == 6) continue; // wrkaround for 1.3.2 module

      // get the errors and check for limits for each bin seperately ...
      /// FIXME: absolute numbers or relative numbers and what is the acceptable limit?

      error_flag |= (ihit > 10)
                    && (m_hEffAll->GetEfficiency(j) + m_hEffAll->GetEfficiencyErrorUp(j) < m_errorlevel); // error if upper error value is below limit
      warn_flag |= (ihit > 10)
                   && (m_hEffAll->GetEfficiency(j) + m_hEffAll->GetEfficiencyErrorUp(j) < m_warnlevel); // (and not only the actual eff value)
    }
  }

  m_cEffAll->cd();
  m_hEffAll->Paint("AP");


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
        B2RESULT(ModuleName);
      }
    }

    gr->SetLineColor(4);
    gr->SetLineWidth(2);
    gr->SetMarkerStyle(8);

    m_cEffAll->Clear();
    m_cEffAll->cd(0);
    gr->Draw("AP");

    auto tt = new TLatex(5.5, scale_min, " 1.3.2 Module is broken, please ignore");
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
    m_line_warn->Draw();
    m_line_error->Draw();
  }

  m_cEffAll->Modified();
  m_cEffAll->Update();

  {
    m_cEffAllUpdate->cd();
    m_hEffAllUpdate->Paint("AP");
    auto gru = m_hEffAllUpdate->GetPaintedGraph();
    if (gru) {
      for (int i = 0; i < gr->GetN(); i++) {
        gru->SetPointEXhigh(i, 0.);
        gru->SetPointEXlow(i, 0.);
        // this has to be done first, as it will recalc Min/Max and destroy axis
        Double_t x, y;
        gru->GetPoint(i, x, y);
        gru->SetPoint(i, x + 0.2, y); // shift a bit
      }
      gru->SetLineColor(kOrange);
      gru->SetLineWidth(2);
      gru->SetMarkerStyle(33);
    }
    m_cEffAllUpdate->Clear();
    m_cEffAllUpdate->cd(0);
    m_hEffAllUpdate->Draw("AP");
    m_cEffAllUpdate->Modified();
    m_cEffAllUpdate->Update();
  }


  double var_efficiency = ihit > 0 ? imatch / ihit : 0.0;
  m_monObj->setVariable("efficiency", var_efficiency);
  m_monObj->setVariable("nmodules", ieff);

#ifdef _BELLE2_EPICS
  double data = 0;
  SEVCHK(ca_put(DBR_DOUBLE, mychid, (void*)&data), "ca_set failure");
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
}

void DQMHistAnalysisPXDEffModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff: terminate called");
}

