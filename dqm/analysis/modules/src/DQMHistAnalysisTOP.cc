/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisTOP.h>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <TClass.h>
#include <TF1.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TString.h>
#include <map>

using namespace std;
using namespace Belle2;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisTOP);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisTOPModule::DQMHistAnalysisTOPModule(): DQMHistAnalysisModule()
{
  // Set description
  setDescription("Histogram analysis module for TOP DQM.");

  // Add parameters
  addParam("asicWindowsBand", m_asicWindowsBand,
           "lower and upper bin of a band denoting good windows", m_asicWindowsBand);
  addParam("asicWindowsAlarmLevels", m_asicWindowsAlarmLevels,
           "alarm levels for the fraction of windows outside the band (yellow, red)", m_asicWindowsAlarmLevels);
  addParam("eventMonitorAlarmLevels", m_eventMonitorAlarmLevels,
           "alarm levels for the fraction of desynchronized digits (yellow, red)", m_eventMonitorAlarmLevels);
  addParam("junkHitsAlarmLevels", m_junkHitsAlarmLevels,
           "alarm levels for the fraction of junk hits (yellow, red)", m_junkHitsAlarmLevels);
  addParam("deadChannelsAlarmLevels", m_deadChannelsAlarmLevels,
           "alarm levels for the fraction of dead + hot channels (yellow, red)", m_deadChannelsAlarmLevels);
  addParam("backgroundAlarmLevels", m_backgroundAlarmLevels,
           "alarm levels for background rates [MHz/PMT] (yellow, red)", m_backgroundAlarmLevels);
  addParam("photonYieldsAlarmLevels", m_photonYieldsAlarmLevels,
           "alarm levels for the number of photons per track (red, yellow)", m_photonYieldsAlarmLevels);
  addParam("excludedBoardstacks", m_excludedBoardstacks,
           "boarstacks to be excluded from alarming. Names are given like '5c', '13d' etc.", m_excludedBoardstacks);
  addParam("pvPrefix", m_pvPrefix, "Epics PV prefix", std::string("TOP:"));
  addParam("injectionBGAlarmLevels", m_injectionBGAlarmLevels,
           "alarm levels for injection background (in % of events)", m_injectionBGAlarmLevels);
  addParam("timingAlarmLevels", m_timingAlarmLevels,
           "alarm levels for time distribution (residual fraction w.r.t reference plot)", m_timingAlarmLevels);
  addParam("eventT0MeanAlarmLevels", m_eventT0MeanAlarmLevels,
           "alarm levels for mean of event T0 [ns]", m_eventT0MeanAlarmLevels);
  addParam("eventT0RmsAlarmLevels", m_eventT0RmsAlarmLevels,
           "alarm levels for r.m.s. of event T0 [ns]", m_eventT0RmsAlarmLevels);
  addParam("offsetMeanAlarmLevels", m_offsetMeanAlarmLevels,
           "alarm levels for mean of bunch offset [ns]", m_offsetMeanAlarmLevels);
  addParam("offsetRmsAlarmLevels", m_offsetRmsAlarmLevels,
           "alarm levels for r.m.s. of bunch offset [ns]", m_offsetRmsAlarmLevels);

  B2DEBUG(20, "DQMHistAnalysisTOP: Constructor done.");
}


DQMHistAnalysisTOPModule::~DQMHistAnalysisTOPModule() { }


void DQMHistAnalysisTOPModule::initialize()
{

  // check module parameters

  if (m_asicWindowsBand.size() != 2) B2ERROR("Parameter list 'asicWindowsBand' must contain two numbers");
  if (m_asicWindowsAlarmLevels.size() != 2) B2ERROR("Parameter list 'asicWindowsAlarmLevels' must contain two numbers");
  if (m_eventMonitorAlarmLevels.size() != 2) B2ERROR("Parameter list 'eventMonitorAlarmLevels' must contain two numbers");
  if (m_junkHitsAlarmLevels.size() != 2) B2ERROR("Parameter list 'junkHitsAlarmLevels' must contain two numbers");
  if (m_deadChannelsAlarmLevels.size() != 2) B2ERROR("Parameter list 'deadChannelsAlarmLevels' must contain two numbers");
  if (m_backgroundAlarmLevels.size() != 2) B2ERROR("Parameter list 'backgroundAlarmLevels' must contain two numbers");
  if (m_photonYieldsAlarmLevels.size() != 2) B2ERROR("Parameter list 'photonYieldsAlarmLevels' must contain two numbers");
  if (m_injectionBGAlarmLevels.size() != 2) B2ERROR("Parameter list 'injectionBGAlarmLevels' must contain two numbers");
  if (m_timingAlarmLevels.size() != 2) B2ERROR("Parameter list 'timingAlarmLevels' must contain two numbers");
  if (m_eventT0MeanAlarmLevels.size() != 2) B2ERROR("Parameter list 'eventT0MeanAlarmLevels' must contain two numbers");
  if (m_eventT0RmsAlarmLevels.size() != 2) B2ERROR("Parameter list 'eventT0RmsAlarmLevels' must contain two numbers");
  if (m_offsetMeanAlarmLevels.size() != 2) B2ERROR("Parameter list 'offsetMeanAlarmLevels' must contain two numbers");
  if (m_offsetRmsAlarmLevels.size() != 2) B2ERROR("Parameter list 'offsetRmsAlarmLevels' must contain two numbers");

  // make a map of boardstack names to ID's

  int id = 1;
  for (int slot = 1; slot <= 16; slot++) {
    string slotstr = to_string(slot);
    for (std::string bs : {"a", "b", "c", "d"}) {
      m_bsmap[slotstr + bs] = id;
      id++;
    }
  }

  // parse excluded boardstacks

  setIncludedBoardstacks(m_excludedBoardstacks);

  // MiraBelle monitoring

  m_monObj = getMonitoringObject("top");

  // Epics used to pass values to shifter's page (output only)

  registerEpicsPV(m_pvPrefix + "badBoardstacks", "badBoardstacks");
  registerEpicsPV(m_pvPrefix + "badCarriers", "badCarriers");
  registerEpicsPV(m_pvPrefix + "badAsics", "badAsics");
  registerEpicsPV(m_pvPrefix + "badPMTs", "badPMTs");
  registerEpicsPV(m_pvPrefix + "numExcludedBS", "numExcludedBS");
  registerEpicsPV(m_pvPrefix + "histoAlarmState", "histoAlarmState"); // to pass overall state to central alarm overview panel

  // Epics used to get limits from configuration file - override module parameters (input only)

  registerEpicsPV(m_pvPrefix + "asicWindowsBand", "asicWindowsBand");
  registerEpicsPV(m_pvPrefix + "asicWindowsAlarmLevels", "asicWindowsAlarmLevels");
  registerEpicsPV(m_pvPrefix + "eventMonitorAlarmLevels", "eventMonitorAlarmLevels");
  registerEpicsPV(m_pvPrefix + "junkHitsAlarmLevels", "junkHitsAlarmLevels");
  registerEpicsPV(m_pvPrefix + "deadChannelsAlarmLevels", "deadChannelsAlarmLevels");
  registerEpicsPV(m_pvPrefix + "backgroundAlarmLevels", "backgroundAlarmLevels"); // also output
  registerEpicsPV(m_pvPrefix + "photonYieldsAlarmLevels", "photonYieldsAlarmLevels");
  registerEpicsPV(m_pvPrefix + "excludedBoardstacks", "excludedBoardstacks");

  registerEpicsPV(m_pvPrefix + "injectionBGAlarmLevels", "injectionBGAlarmLevels"); // also output
  registerEpicsPV(m_pvPrefix + "timingAlarmLevels", "timingAlarmLevels");
  registerEpicsPV(m_pvPrefix + "eventT0MeanAlarmLevels", "eventT0MeanAlarmLevels");
  registerEpicsPV(m_pvPrefix + "eventT0RmsAlarmLevels", "eventT0RmsAlarmLevels");
  registerEpicsPV(m_pvPrefix + "offsetMeanAlarmLevels", "offsetMeanAlarmLevels");
  registerEpicsPV(m_pvPrefix + "offsetRmsAlarmLevels", "offsetRmsAlarmLevels");

  // new canvases, histograms and graphic primitives

  gROOT->cd();

  m_c_photonYields = new TCanvas("TOP/c_photonYields", "c_photonYields");
  m_c_backgroundRates = new TCanvas("TOP/c_backgroundRates", "c_backgroundRates");

  m_deadFraction = new TH1F("TOP/deadFraction", "Fraction of dead channels in included boardstacks", 16, 0.5, 16.5);
  m_deadFraction->SetXTitle("slot number");
  m_deadFraction->SetYTitle("fraction");
  m_hotFraction = new TH1F("TOP/hotFraction", "Fraction of hot channels in included boardstacks", 16, 0.5, 16.5);
  m_hotFraction->SetXTitle("slot number");
  m_hotFraction->SetYTitle("fraction");
  m_excludedFraction = new TH1F("TOP/excludedFraction", "Fraction of hot and dead channels in excluded bordstacks", 16, 0.5, 16.5);
  m_excludedFraction->SetXTitle("slot number");
  m_excludedFraction->SetYTitle("fraction");
  m_activeFraction = new TH1F("TOP/activeFraction", "Fraction of active channels", 16, 0.5, 16.5);
  m_activeFraction->SetXTitle("slot number");
  m_activeFraction->SetYTitle("fraction");
  m_c_deadAndHot = new TCanvas("TOP/c_deadAndHotChannels", "c_deadAndHotChannels");

  m_junkFraction = new TH1F("TOP/junkFraction", "Fraction of junk hits per boardstack", 64, 0.5, 16.5);
  m_junkFraction->SetXTitle("slot number");
  m_junkFraction->SetYTitle("fraction");
  // note: titles are intentionally the same since this one is plotted first
  m_excludedBSHisto = new TH1F("TOP/excludedBSHisto", "Fraction of junk hits per boardstack", 64, 0.5, 16.5);
  m_excludedBSHisto->SetXTitle("slot number");
  m_excludedBSHisto->SetYTitle("fraction");
  m_c_junkFraction = new TCanvas("TOP/c_junkFraction", "c_junkFraction");

  for (int slot = 1; slot <= 16; slot++) {
    string hname = "TOP/pmtHitRates_" + to_string(slot);
    string htitle = "PMT hits per event for slot #" + to_string(slot);
    auto* h = new TH1F(hname.c_str(), htitle.c_str(), 32, 0.5, 32.5);
    h->SetXTitle("PMT number");
    h->SetYTitle("Number of good hits per event");
    m_pmtHitRates.push_back(h);
    string cname = "TOP/c_pmtHitRates_" + to_string(slot);
    string ctitle = "c_pmtHitRates_" + to_string(slot);
    m_c_pmtHitRates.push_back(new TCanvas(cname.c_str(), ctitle.c_str()));
  }

  for (std::string name : {
         "nhitInjLER", "nhitInjHER", "nhitInjLERcut", "nhitInjHERcut",
         "eventInjLER", "eventInjHER", "eventInjLERcut", "eventInjHERcut"
       }) {
    for (std::string proj : {"_px", "_py"}) {
      std::string cname = "TOP/c_" + name + proj;
      m_c_injBGs[cname] = new TCanvas(cname.c_str(), (name + proj).c_str());
    }
  }

  m_text1 = new TPaveText(0.125, 0.8, 0.675, 0.88, "NDC");
  m_text1->SetFillColorAlpha(kWhite, 0);
  m_text1->SetBorderSize(0);
  m_text2 = new TPaveText(0.55, 0.8, 0.85, 0.89, "NDC");
  m_text2->SetFillColorAlpha(kWhite, 0);
  m_text2->SetBorderSize(0);
  m_text3 = new TPaveText(0.47, 0.8, 0.85, 0.89, "NDC");
  m_text3->SetFillColorAlpha(kWhite, 0);
  m_text3->SetBorderSize(0);
  m_text4 = new TPaveText(0.125, 0.8, 0.675, 0.88, "NDC");
  m_text4->SetFillColorAlpha(kWhite, 0);
  m_text4->SetBorderSize(0);

  for (int slot = 1; slot < 16; slot++) {
    auto* line = new TLine(slot + 0.5, 0, slot + 0.5, 1);
    line->SetLineWidth(1);
    line->SetLineStyle(2);
    m_verticalLines.push_back(line);
  }

  setAlarmLines();

  B2DEBUG(20, "DQMHistAnalysisTOP: initialized.");
}


void DQMHistAnalysisTOPModule::beginRun()
{
  m_mirabelleVariables.clear();

  B2DEBUG(20, "DQMHistAnalysisTOP: beginRun called.");
}


void DQMHistAnalysisTOPModule::event()
{
  // get type of the run (TODO: to be replaced with base class function when fixed)
  auto* rtype = findHist("DQMInfo/rtype");
  m_runType = rtype ? rtype->GetTitle() : "";
  m_IsNullRun = (m_runType == "null");

  // get number of events processed with TOPDQM module
  auto* goodHitsPerEvent = findHist("TOP/goodHitsPerEventAll");
  m_numEvents = goodHitsPerEvent ? goodHitsPerEvent->GetEntries() : 0;

  bool zeroSupp = gStyle->GetHistMinimumZero();
  gStyle->SetHistMinimumZero(true);

  // update alarm levels and other parameters from EpicsPVs
  updateLimits();

  // reset overall alarm state
  m_alarmStateOverall = c_Gray;

  // Update window_vs_slot canvas w/ alarming
  updateWindowVsSlotCanvas();

  // Update event desynchronization monitor w/ alarming
  updateEventMonitorCanvas();

  // Update number of good hits per event w/ alarming (injection BG)
  updateNGoodHitsCanvas();

  // Update event T0 w/ alarming
  updateEventT0Canvas();

  // Update bunch offset w/ alarming
  updateBunchOffsetCanvas();

  // Fraction of dead and hot channels
  const auto* activeFraction = makeDeadAndHotFractionsPlot();

  // Photon yields and background rates, corrected for dead and hot channels
  makePhotonYieldsAndBGRatePlots(activeFraction);

  // Fractions of junk hits
  makeJunkFractionPlot();

  // Set z-axis range to 3 times the average for good hits, 30 times the average for junk hits
  setZAxisRange("TOP/good_hits_xy_", 3);
  setZAxisRange("TOP/bad_hits_xy_", 30);
  setZAxisRange("TOP/good_hits_asics_", 3);
  setZAxisRange("TOP/bad_hits_asics_", 30);

  // Background subtracted time distributions (only for physics runs)
  if (m_runType == "physics") {
    auto* trackHits = (TH2F*) findHist("TOP/trackHits");
    makeBGSubtractedTimingPlot("goodHitTimes", trackHits, 0);
    for (int slot = 1; slot <= 16; slot++) {
      makeBGSubtractedTimingPlot("good_timing_" + to_string(slot), trackHits, slot);
    }
  }

  // Update timing plot w/ alarming
  updateTimingCanvas();

  // PMT hit rates
  makePMTHitRatesPlots();

  // Injection BG
  makeInjectionBGPlots();

  // Set Epics variables
  setEpicsVariables();

  gStyle->SetHistMinimumZero(zeroSupp);
}


void DQMHistAnalysisTOPModule::endRun()
{
  if (m_photonYields) delete m_photonYields;
  m_photonYields = nullptr;
  if (m_backgroundRates) delete m_backgroundRates;
  m_backgroundRates = nullptr;

  // add MiraBelle monitoring

  for (const auto& var : m_mirabelleVariables) {
    m_monObj->setVariable(var.first, var.second);
    B2DEBUG(20, var.first << " " << var.second);
  }

  B2DEBUG(20, "DQMHistAnalysisTOP : endRun called");
}


void DQMHistAnalysisTOPModule::terminate()
{
  B2DEBUG(20, "terminate called");
}


void DQMHistAnalysisTOPModule::updateWindowVsSlotCanvas()
{
  int alarmState = c_Gray;
  m_text1->Clear();

  auto* hraw = (TH2F*) findHist("TOP/window_vs_slot");
  if (hraw) {
    auto* px = hraw->ProjectionX("tmp_px");
    auto* band = hraw->ProjectionX("TOP/windowFractions", m_asicWindowsBand[0], m_asicWindowsBand[1]);
    band->Add(px, band, 1, -1);
    double total = px->Integral();
    double totalWindowFraction = (total != 0) ? band->Integral() / total : 0;
    band->Divide(band, px);
    setMiraBelleVariables("RateBadRaw_slot", band);
    m_mirabelleVariables["RateBadRaw_all"] = totalWindowFraction;
    if (total > 0) {
      alarmState = getAlarmState(totalWindowFraction, m_asicWindowsAlarmLevels);
      m_text1->AddText(Form("Fraction outside red lines: %.2f %%", totalWindowFraction * 100.0));
    }
    delete px;
    delete band;
  }

  m_alarmStateOverall = std::max(m_alarmStateOverall, alarmState);

  auto* canvas = findCanvas("TOP/c_window_vs_slot");
  if (canvas) {
    canvas->Clear();
    canvas->cd();
    if (hraw) hraw->Draw();
    m_text1->Draw();
    for (auto* line : m_asicWindowsBandLines) line->Draw();
    canvas->Pad()->SetFrameFillColor(10);
    canvas->Pad()->SetFillColor(getAlarmColor(alarmState));
    canvas->Modified();
  }
}


void DQMHistAnalysisTOPModule::updateEventMonitorCanvas()
{
  int alarmState = c_Gray;
  m_text2->Clear();

  auto* evtMonitor = (TH1F*) findHist("TOP/BoolEvtMonitor");
  if (evtMonitor) {
    double totalEvts = evtMonitor->Integral();
    double badEvts = evtMonitor->GetBinContent(2);
    if (totalEvts > 0) {
      double badRatio = badEvts / totalEvts;
      alarmState = getAlarmState(badRatio, m_eventMonitorAlarmLevels);
      m_text2->AddText(Form("Fraction: %.4f %%", badRatio * 100.0));
    }
  }

  m_alarmStateOverall = std::max(m_alarmStateOverall, alarmState);

  auto* canvas = findCanvas("TOP/c_BoolEvtMonitor");
  if (canvas) {
    canvas->cd();
    m_text2->Draw();
    canvas->Pad()->SetFrameFillColor(10);
    canvas->Pad()->SetFillColor(getAlarmColor(alarmState));
    canvas->Modified();
  }
}


void DQMHistAnalysisTOPModule::updateNGoodHitsCanvas()
{
  int alarmState = c_Gray;
  m_text4->Clear();

  double fract = 0;
  double xcut = 0;
  double ymax = 0;
  auto* h = (TH1F*) findHist("TOP/goodHitsPerEventAll");
  if (h) {
    double totalEvts = h->GetEntries();
    if (totalEvts > 1000) {
      // fraction of events with more than xcut hits - these are mostly containing injection BG
      xcut = h->GetBinCenter(h->GetMaximumBin()) + 900;
      ymax = h->GetMaximum() / 2;
      fract = h->Integral(h->FindBin(xcut), h->GetNbinsX() + 1) / totalEvts * 100; // in %
      alarmState = getAlarmState(fract, m_injectionBGAlarmLevels);
      m_text4->AddText(Form("Events w/ Injection BG: %.2f %%", fract));
    }
  }

  setEpicsPV("injectionBGAlarmLevels", fract);
  m_alarmStateOverall = std::max(m_alarmStateOverall, alarmState);

  auto* canvas = findCanvas("TOP/c_goodHitsPerEventAll");
  if (canvas) {
    canvas->cd();
    if (not m_injBGCutLine) {
      m_injBGCutLine = new TLine(xcut, 0, xcut, ymax);
      m_injBGCutLine->SetLineWidth(2);
      m_injBGCutLine->SetLineColor(kRed);
      m_injBGCutLine->Draw("same");
    } else {
      m_injBGCutLine->SetX1(xcut);
      m_injBGCutLine->SetX2(xcut);
      m_injBGCutLine->SetY2(ymax);
    }
    m_text4->Draw();
    canvas->Pad()->SetFrameFillColor(10);
    canvas->Pad()->SetFillColor(getAlarmColor(alarmState));
    canvas->Modified();
  }
}


void DQMHistAnalysisTOPModule::updateEventT0Canvas()
{
  int alarmState = c_Gray;

  auto* h = (TH1F*) findHist("TOP/eventT0");
  if (h) {
    double totalEvts = h->GetEntries();
    if (totalEvts > 100) {
      double mean = h->GetMean();
      double rms = h->GetRMS();
      alarmState = std::max(getAlarmState(fabs(mean), m_eventT0MeanAlarmLevels), getAlarmState(rms, m_eventT0RmsAlarmLevels));
    }
  }

  m_alarmStateOverall = std::max(m_alarmStateOverall, alarmState);

  auto* canvas = findCanvas("TOP/c_eventT0");
  if (canvas) {
    canvas->cd();
    canvas->Pad()->SetFrameFillColor(10);
    canvas->Pad()->SetFillColor(getAlarmColor(alarmState));
    canvas->Modified();
  }
}


void DQMHistAnalysisTOPModule::updateBunchOffsetCanvas()
{
  int alarmState = c_Gray;

  auto* h = (TH1F*) findHist("TOP/bunchOffset");
  if (h) {
    double totalEvts = h->GetEntries();
    if (totalEvts > 100) {
      double mean = h->GetMean();
      double rms = h->GetRMS();
      alarmState = std::max(getAlarmState(fabs(mean), m_offsetMeanAlarmLevels), getAlarmState(rms, m_offsetRmsAlarmLevels));
    }
  }

  m_alarmStateOverall = std::max(m_alarmStateOverall, alarmState);

  auto* canvas = findCanvas("TOP/c_bunchOffset");
  if (canvas) {
    canvas->cd();
    canvas->Pad()->SetFrameFillColor(10);
    canvas->Pad()->SetFillColor(getAlarmColor(alarmState));
    canvas->Modified();
  }
}


void DQMHistAnalysisTOPModule::updateTimingCanvas()
{
  int alarmState = c_Gray;

  auto* h = (TH1F*) findHist("TOP/goodHitTimes");
  auto* href = (TH1F*) findRefHist("TOP/goodHitTimes");
  if (h and href) {
    double n = h->Integral();
    double nref = href->Integral();
    if (n > 0 and nref > 0 and sameHistDefinition(h, href)) {
      auto* h_clone = (TH1F*) h->Clone("tmp");
      auto* href_clone = (TH1F*) href->Clone("tmpref");
      h_clone->Scale(1 / n);
      href_clone->Scale(1 / nref);
      h_clone->Add(h_clone, href_clone, 1, -1);
      double sumDiff = 0;
      double errDiff = 0;
      for (int i = 1; i <= h_clone->GetNbinsX(); i++) {
        sumDiff += fabs(h_clone->GetBinContent(i));
        errDiff += pow(h_clone->GetBinError(i), 2);
      }
      errDiff = sqrt(errDiff);
      if (sumDiff < 5 * errDiff) sumDiff = 0;  // difference not significant
      alarmState = getAlarmState(sumDiff, m_timingAlarmLevels);
      delete h_clone;
      delete href_clone;
    }
  }

  m_alarmStateOverall = std::max(m_alarmStateOverall, alarmState);

  auto* canvas = findCanvas("TOP/c_goodHitTimes");
  if (canvas) {
    canvas->cd();
    canvas->Pad()->SetFrameFillColor(10);
    canvas->Pad()->SetFillColor(getAlarmColor(alarmState));
    canvas->Modified();
  }
}

bool DQMHistAnalysisTOPModule::sameHistDefinition(TH1* h1, TH1* h2)
{
  if (h1->GetNbinsX() != h2->GetNbinsX()) return false;
  if (h1->GetXaxis()->GetXmin() != h2->GetXaxis()->GetXmin()) return false;
  if (h1->GetXaxis()->GetXmax() != h2->GetXaxis()->GetXmax()) return false;
  return true;
}

const TH1F* DQMHistAnalysisTOPModule::makeDeadAndHotFractionsPlot()
{
  m_deadFraction->Reset();
  m_hotFraction->Reset();
  m_excludedFraction->Reset();
  m_activeFraction->Reset();
  double inactiveFract = 0; // max inactive channel fraction when some boardstacks are excluded from alarming

  for (int slot = 1; slot <= 16; slot++) {
    auto* h = (TH1F*) findHist("TOP/good_channel_hits_" + std::to_string(slot));
    if (not h) continue;

    auto cuts = getDeadAndHotCuts(h);
    double deadCut = cuts.first;
    double hotCut = cuts.second;
    double deadFract = 0;
    double hotFract = 0;
    double deadFractIncl = 0;
    double hotFractIncl = 0;
    for (int chan = 0; chan < h->GetNbinsX(); chan++) {
      double y = h->GetBinContent(chan + 1);
      int bs = chan / 128 + (slot - 1) * 4;
      bool included = m_includedBoardstacks[bs];
      if (y <= deadCut) {
        deadFract += 1;
        if (included) deadFractIncl += 1;
      } else if (y > hotCut) {
        hotFract += 1;
        if (included) hotFractIncl += 1;
      }
    }
    deadFract /= h->GetNbinsX();
    hotFract /= h->GetNbinsX();
    deadFractIncl /= h->GetNbinsX();
    hotFractIncl /= h->GetNbinsX();
    m_deadFraction->SetBinContent(slot, deadFractIncl);
    m_hotFraction->SetBinContent(slot, hotFractIncl);
    m_excludedFraction->SetBinContent(slot, deadFract - deadFractIncl + hotFract - hotFractIncl);
    m_activeFraction->SetBinContent(slot, 1 - deadFract - hotFract);
    inactiveFract = std::max(inactiveFract, deadFractIncl + hotFractIncl);
  }

  setMiraBelleVariables("ActiveChannelFraction_slot", m_activeFraction);

  int alarmState = c_Gray;
  if (m_activeFraction->Integral() > 0) {
    alarmState = getAlarmState(inactiveFract, m_deadChannelsAlarmLevels);
  }

  m_alarmStateOverall = std::max(m_alarmStateOverall, alarmState);

  m_deadFraction->SetFillColor(1);
  m_deadFraction->SetLineColor(1);
  m_deadFraction->GetXaxis()->SetNdivisions(16);

  m_hotFraction->SetFillColor(2);
  m_hotFraction->SetLineColor(2);
  m_hotFraction->GetXaxis()->SetNdivisions(16);

  m_excludedFraction->SetFillColor(kGray);
  m_excludedFraction->SetLineColor(kGray);
  m_excludedFraction->GetXaxis()->SetNdivisions(16);

  m_activeFraction->SetFillColor(0);
  m_activeFraction->GetXaxis()->SetNdivisions(16);

  auto* canvas = m_c_deadAndHot;
  canvas->Clear();
  canvas->cd();
  canvas->Pad()->SetFrameFillColor(10);
  if (not m_stack) {
    m_stack = new THStack("TOP/stack", "Fraction of dead and hot channels");
    m_stack->Add(m_deadFraction);
    m_stack->Add(m_hotFraction);
    m_stack->Add(m_excludedFraction);
    m_stack->Add(m_activeFraction);
  }
  m_stack->Draw();

  for (auto* line : m_deadChannelsAlarmLines) line->Draw("same");

  if (not m_legend) {
    m_legend = new TLegend(0.8, 0.87, 0.99, 0.99);
    m_legend->AddEntry(m_hotFraction, "hot");
    m_legend->AddEntry(m_deadFraction, "dead");
    m_legend->AddEntry(m_excludedFraction, "excluded");
  }
  m_legend->Draw("same");

  canvas->Pad()->SetFillColor(getAlarmColor(alarmState));
  canvas->Modified();

  return m_activeFraction;
}


void DQMHistAnalysisTOPModule::makePhotonYieldsAndBGRatePlots(const TH1F* activeFraction)
{
  for (auto* canvas : {m_c_photonYields, m_c_backgroundRates}) {
    canvas->Clear();
    canvas->Pad()->SetFrameFillColor(10);
    canvas->Pad()->SetFillColor(getAlarmColor(c_Gray));
    canvas->Modified();
  }
  m_averageRate = 0;

  auto* signalHits = (TProfile*) findHist("TOP/signalHits");
  if (not signalHits) return;

  auto* backgroundHits = (TProfile*) findHist("TOP/backgroundHits");
  if (not backgroundHits) return;

  if (m_photonYields) delete m_photonYields;
  m_photonYields = signalHits->ProjectionX("TOP/photonYields");
  if (m_backgroundRates) delete m_backgroundRates;
  m_backgroundRates = backgroundHits->ProjectionX("TOP/backgroundRates");
  auto* activeFract = (TH1F*) activeFraction->Clone("tmp");
  for (int i = 1; i <= activeFract->GetNbinsX(); i++) activeFract->SetBinError(i, 0);

  m_photonYields->Add(m_photonYields, m_backgroundRates, 1, -1);
  m_photonYields->Divide(m_photonYields, activeFract);
  setMiraBelleVariables("PhotonsPerTrack_slot", m_photonYields);

  int alarmState = c_Gray;
  if (signalHits->GetEntries() > 0 and activeFraction->Integral() > 0) {
    double hmin = 1000;
    for (int i = 1; i <= m_photonYields->GetNbinsX(); i++) {
      if (signalHits->GetBinEntries(i) < 10) continue;
      hmin = std::min(hmin, m_photonYields->GetBinContent(i) + 3 * m_photonYields->GetBinError(i));
    }
    if (hmin < 1000) alarmState = getAlarmState(hmin, m_photonYieldsAlarmLevels, false);
  }
  m_alarmStateOverall = std::max(m_alarmStateOverall, alarmState);

  m_photonYields->SetTitle("Number of photons per track");
  m_photonYields->SetYTitle("photons per track");
  m_photonYields->SetMarkerStyle(24);
  m_photonYields->GetXaxis()->SetNdivisions(16);

  auto* canvas = m_c_photonYields;
  canvas->cd();
  m_photonYields->SetMinimum(0);
  m_photonYields->Draw();
  for (auto* line : m_photonYieldsAlarmLines) line->Draw("same");
  canvas->Pad()->SetFillColor(getAlarmColor(alarmState));
  canvas->Modified();

  m_backgroundRates->Scale(1.0 / 50.0e-3 / 32);  // measured in 50 ns window, 32 PMT's ==> rate in MHz/PMT
  m_backgroundRates->Divide(m_backgroundRates, activeFract);
  setMiraBelleVariables("BackgroundRate_slot", m_backgroundRates);

  alarmState = c_Gray;
  m_text3->Clear();
  if (backgroundHits->GetEntries() > 100 and activeFraction->Integral() > 0) {
    int status = m_backgroundRates->Fit("pol0", "Q0");
    if (status == 0) {
      auto* fun = m_backgroundRates->GetFunction("pol0");
      if (fun) {
        m_averageRate = fun->GetParameter(0);
        double error = fun->GetParError(0);
        alarmState = getAlarmState(m_averageRate - 3 * error, m_backgroundAlarmLevels);
        m_text3->AddText(Form("Average: %.2f MHz/PMT", m_averageRate));
      }
    }
  }
  m_alarmStateOverall = std::max(m_alarmStateOverall, alarmState);

  m_backgroundRates->SetTitle("Background rates");
  m_backgroundRates->SetYTitle("background rate [MHz/PMT]");
  m_backgroundRates->SetMarkerStyle(24);
  m_backgroundRates->GetXaxis()->SetNdivisions(16);

  canvas = m_c_backgroundRates;
  canvas->cd();
  m_backgroundRates->SetMinimum(0);
  m_backgroundRates->Draw();
  for (auto* line : m_backgroundAlarmLines) line->Draw("same");
  m_text3->Draw();
  canvas->Pad()->SetFillColor(getAlarmColor(alarmState));
  canvas->Modified();

  delete activeFract;
}


void DQMHistAnalysisTOPModule::makeJunkFractionPlot()
{
  m_junkFraction->Reset();
  m_excludedBSHisto->Reset();
  auto* allHits = (TH1D*) m_junkFraction->Clone("tmp");
  for (int slot = 1; slot <= 16; slot++) {
    auto* good = (TH1F*) findHist("TOP/good_channel_hits_" + std::to_string(slot));
    if (not good) continue;
    auto* bad = (TH1F*) findHist("TOP/bad_channel_hits_" + std::to_string(slot));
    if (not bad) continue;
    for (int i = 0; i < 512; i++) {
      int bs = i / 128;
      allHits->Fill(slot + bs / 4. - 0.5, good->GetBinContent(i + 1) + bad->GetBinContent(i + 1));
      m_junkFraction->Fill(slot + bs / 4. - 0.5, bad->GetBinContent(i + 1));
    }
  }

  m_junkFraction->Divide(m_junkFraction, allHits, 1, 1, "B");

  int alarmState = c_Gray;
  if (allHits->Integral() > 0) {
    double hmax = 0;
    for (size_t i = 0; i < m_includedBoardstacks.size(); i++) {
      if (m_includedBoardstacks[i]) hmax = std::max(hmax, m_junkFraction->GetBinContent(i + 1));
      else m_excludedBSHisto->SetBinContent(i + 1, 1);
    }
    alarmState = getAlarmState(hmax, m_junkHitsAlarmLevels);
  }
  delete allHits;
  m_alarmStateOverall = std::max(m_alarmStateOverall, alarmState);

  auto* canvas = m_c_junkFraction;
  canvas->Clear();
  canvas->cd();
  canvas->Pad()->SetFrameFillColor(10);
  canvas->Pad()->SetFillColor(getAlarmColor(alarmState));
  m_excludedBSHisto->SetFillColor(kGray);
  m_excludedBSHisto->SetLineColor(kGray);
  m_excludedBSHisto->GetXaxis()->SetNdivisions(16);
  m_excludedBSHisto->GetYaxis()->SetRangeUser(0, 1);
  m_excludedBSHisto->Draw();
  m_junkFraction->SetMarkerStyle(24);
  m_junkFraction->GetXaxis()->SetNdivisions(16);
  m_junkFraction->GetYaxis()->SetRangeUser(0, 1); // Note: m_junkFraction->GetMaximum() will now give 1 and not the histogram maximum!
  m_junkFraction->Draw("same");
  for (auto* line : m_verticalLines) line->Draw("same");
  for (auto* line : m_junkHitsAlarmLines) line->Draw("same");
  canvas->Modified();
}


void DQMHistAnalysisTOPModule::setZAxisRange(const std::string& name, double scale)
{
  double totalHits = 0;
  std::vector<TH2F*> histos;
  for (int slot = 1; slot <= 16; slot++) {
    TH2F* h = (TH2F*) findHist(name + std::to_string(slot));
    if (not h) continue;
    histos.push_back(h);
    totalHits += h->Integral();
  }
  if (histos.empty()) return;
  double average = totalHits / 512 / histos.size();  // per pixel or asic channel

  for (auto* h : histos) h->GetZaxis()->SetRangeUser(0, std::max(average * scale, 1.0));
}


void DQMHistAnalysisTOPModule::makeBGSubtractedTimingPlot(const std::string& name, const TH2F* trackHits, int slot)
{
  auto* canvas = findCanvas("TOP/c_" + name);
  if (not canvas) return;

  auto* h = (TH1F*) findHist("TOP/" + name);
  if (not h) return;

  auto* hb = (TH1F*) findHist("TOP/" + name + "BG");
  if (not hb) return;

  if (trackHits) {
    // use the ratio of events w/ and w/o track in the slot to scale the background
    double s = (slot == 0) ? trackHits->Integral(1, 16, 2, 2) : trackHits->GetBinContent(slot, 2);
    if (s == 0) return;
    double sb = (slot == 0) ? trackHits->Integral(1, 16, 1, 1) : trackHits->GetBinContent(slot, 1);
    if (sb == 0) return;
    h->Add(h, hb, 1, -s / sb);
  } else {
    // use the content of bins at t < 0 to scale the background
    int i0 = h->GetXaxis()->FindBin(0.); // bin at t = 0
    double s = h->Integral(1, i0);
    if (s == 0) return;
    double sb = hb->Integral(1, i0);
    if (sb == 0) return;
    if (s / sb > 1) return; // this can happen due to low statistics and is not reliable
    h->Add(h, hb, 1, -s / sb);
  }

  TString title = TString(h->GetTitle()) + " (BG subtracted)";
  h->SetTitle(title);

  canvas->Clear();
  canvas->cd();
  h->Draw();
  canvas->Modified();
}


void DQMHistAnalysisTOPModule::makePMTHitRatesPlots()
{
  auto* h0 = (TH1F*) findHist("TOP/goodHitsPerEventAll");
  if (not h0) return;
  double numEvents = h0->GetEntries();
  if (numEvents == 0) return;

  int numSlots = m_pmtHitRates.size();
  for (int slot = 1; slot <= numSlots; slot++) {
    string name = "TOP/good_hits_xy_" + to_string(slot);
    auto* hxy = (TH2F*) findHist(name);
    if (not hxy) continue;
    std::vector<double> pmts(32, 0);
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 64; col++) {
        int pmt = col / 4 + (row / 4) * 16;
        pmts[pmt] += hxy->GetBinContent(col + 1, row + 1);
      }
    }
    auto* h = m_pmtHitRates[slot - 1];
    for (size_t i = 0; i < pmts.size(); i++) {
      h->SetBinContent(i + 1, pmts[i] / numEvents);
    }
    auto* canvas = m_c_pmtHitRates[slot - 1];
    canvas->Clear();
    canvas->cd();
    h->SetMinimum(0);
    h->Draw();
    canvas->Modified();
  }
}


void DQMHistAnalysisTOPModule::makeInjectionBGPlots()
{
  for (std::string name : {"nhitInjLER", "nhitInjHER", "nhitInjLERcut", "nhitInjHERcut"}) {
    std::string hname = "TOP/" + name;
    auto* h = (TProfile2D*) findHist(hname);
    if (not h) continue;
    for (std::string proj : {"_px", "_py"}) {
      std::string cname = "TOP/c_" + name + proj;
      auto* canvas = m_c_injBGs[cname];
      if (not canvas) continue;
      canvas->Clear();
      canvas->cd();
      auto& hproj = m_profiles[cname];
      if (hproj) delete hproj;
      hproj = (proj == "_px") ? h->ProfileX((hname + proj).c_str()) : h->ProfileY((hname + proj).c_str());
      std::string xtitle = (proj == "_px") ? h->GetXaxis()->GetTitle() : h->GetYaxis()->GetTitle();
      hproj->SetXTitle(xtitle.c_str());
      hproj->SetYTitle(h->GetZaxis()->GetTitle());
      hproj->SetMinimum(0);
      hproj->Draw("hist");
      canvas->Modified();
    }
  }

  for (std::string name : {"eventInjLER", "eventInjHER", "eventInjLERcut", "eventInjHERcut"}) {
    std::string hname = "TOP/" + name;
    auto* h = (TH2F*) findHist(hname);
    if (not h) continue;
    for (std::string proj : {"_px", "_py"}) {
      std::string cname = "TOP/c_" + name + proj;
      auto* canvas = m_c_injBGs[cname];
      if (not canvas) continue;
      canvas->Clear();
      canvas->cd();
      auto& hproj = m_projections[cname];
      if (hproj) delete hproj;
      hproj = (proj == "_px") ? h->ProjectionX((hname + proj).c_str()) : h->ProjectionY((hname + proj).c_str());
      std::string xtitle = (proj == "_px") ? h->GetXaxis()->GetTitle() : h->GetYaxis()->GetTitle();
      hproj->SetXTitle(xtitle.c_str());
      hproj->SetYTitle(h->GetZaxis()->GetTitle());
      hproj->SetMinimum(0);
      hproj->Draw("hist");
      canvas->Modified();
    }
  }

}


void DQMHistAnalysisTOPModule::setMiraBelleVariables(const std::string& variableName, const TH1* histogram)
{
  for (int slot = 1; slot <= 16; slot++) {
    auto vname = variableName + std::to_string(slot);
    double value = histogram ? histogram->GetBinContent(slot) : 0;
    m_mirabelleVariables[vname] = value;
  }
}


int DQMHistAnalysisTOPModule::getAlarmState(double value, const std::vector<double>& alarmLevels, bool bigRed) const
{
  if (m_IsNullRun or m_numEvents < 1000) return c_Gray;

  if (bigRed) {
    if (value < alarmLevels[0]) return c_Green;
    else if (value < alarmLevels[1]) return c_Yellow;
    else return c_Red;
  } else {
    if (value < alarmLevels[0]) return c_Red;
    else if (value < alarmLevels[1]) return c_Yellow;
    else return c_Green;
  }
}


void DQMHistAnalysisTOPModule::setAlarmLines(const std::vector<double>& alarmLevels, double xmin, double xmax,
                                             std::vector<TLine*>& alarmLines, bool bigRed)
{
  std::vector<int> colors = {kOrange, kRed};
  if (not bigRed) std::reverse(colors.begin(), colors.end());
  for (size_t i = 0; i < std::min(colors.size(), alarmLevels.size()); i++) {
    if (i < alarmLines.size()) {
      auto* line = alarmLines[i];
      line->SetX1(xmin);
      line->SetX2(xmax);
      line->SetY1(alarmLevels[i]);
      line->SetY2(alarmLevels[i]);
    } else {
      auto* line = new TLine(xmin, alarmLevels[i], xmax, alarmLevels[i]);
      line->SetLineWidth(2);
      line->SetLineStyle(2);
      line->SetLineColor(colors[i]);
      alarmLines.push_back(line);
    }
  }
}


void DQMHistAnalysisTOPModule::setAlarmLines()
{
  for (size_t i = 0; i < m_asicWindowsBand.size(); i++) {
    double y = m_asicWindowsBand[i];
    if (i < m_asicWindowsBandLines.size()) {
      auto* line = m_asicWindowsBandLines[i];
      line->SetY1(y);
      line->SetY2(y);
    } else {
      auto* line = new TLine(0.5, y, 16.5, y);
      line->SetLineWidth(2);
      line->SetLineColor(kRed);
      m_asicWindowsBandLines.push_back(line);
    }
  }

  setAlarmLines(m_junkHitsAlarmLevels, 0.5, 16.5, m_junkHitsAlarmLines);
  setAlarmLines(m_deadChannelsAlarmLevels, 0.5, 16.5, m_deadChannelsAlarmLines);
  setAlarmLines(m_backgroundAlarmLevels, 0.5, 16.5, m_backgroundAlarmLines);
  setAlarmLines(m_photonYieldsAlarmLevels, 0.5, 16.5, m_photonYieldsAlarmLines, false);
}


std::pair<double, double> DQMHistAnalysisTOPModule::getDeadAndHotCuts(const TH1* h)
{
  std::vector<double> binContents;
  for (int k = 1; k <= h->GetNbinsY(); k++) {
    for (int i = 1; i <= h->GetNbinsX(); i++) {
      binContents.push_back(h->GetBinContent(i, k));
    }
  }

  double mean = 0;
  double rms = h->GetMaximum();
  for (int iter = 0; iter < 5; iter++) {
    double sumy = 0;
    double sumyy = 0;
    int n = 0;
    for (auto y : binContents) {
      if (y == 0 or fabs(y - mean) > 3 * rms) continue;
      sumy += y;
      sumyy += y * y;
      n++;
    }
    if (n == 0) continue;
    mean = sumy / n;
    rms = sqrt(sumyy / n - mean * mean);
  }

  return std::make_pair(mean / 5, std::max(mean * 2, mean + 6 * rms));
}


void DQMHistAnalysisTOPModule::setEpicsVariables()
{
  int badBoardstacks = 0;
  int badCarriers = 0;
  int badAsics = 0;
  for (int slot = 1; slot <= 16; slot++) {
    std::string hname = "TOP/good_hits_asics_" + to_string(slot);
    auto* h = (TH2F*) findHist(hname);
    if (not h) continue;

    auto cuts = getDeadAndHotCuts(h);
    double deadCut = cuts.first;
    double hotCut = cuts.second;
    std::vector<int> asics(64, 0);
    std::vector<int> carriers(16, 0);
    std::vector<int> boardstacks(4, 0);
    for (int asic = 0; asic < 64; asic++) {
      int carrier = asic / 4;
      int boardstack = carrier / 4;
      for (int chan = 0; chan < 8; chan++) {
        double y = h->GetBinContent(asic + 1, chan + 1);
        if (y > deadCut and y <= hotCut) {
          asics[asic]++;
          carriers[carrier]++;
          boardstacks[boardstack]++;
        }
      }
    }
    for (int n : asics) if (n == 0) badAsics++;
    for (int n : carriers) if (n == 0) badCarriers++;
    for (int n : boardstacks) if (n == 0) badBoardstacks++;
  }
  badAsics -= badCarriers * 4;
  badCarriers -= badBoardstacks * 4;

  int badPMTs = 0;
  for (int slot = 1; slot <= 16; slot++) {
    std::string hname = "TOP/good_hits_xy_" + to_string(slot);
    auto* h = (TH2F*) findHist(hname);
    if (not h) continue;

    auto cuts = getDeadAndHotCuts(h);
    double deadCut = cuts.first;
    double hotCut = cuts.second;
    std::vector<int> pmts(32, 0);
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 64; col++) {
        int pmt = col / 4 + (row / 4) * 16;
        double y = h->GetBinContent(col + 1, row + 1);
        if (y > deadCut and y <= hotCut) pmts[pmt]++;
      }
    }
    for (int n : pmts) if (n == 0) badPMTs++;
  }
  badPMTs -= badBoardstacks * 8;

  setEpicsPV("badBoardstacks", badBoardstacks);
  setEpicsPV("badCarriers", badCarriers);
  setEpicsPV("badAsics", badAsics);
  setEpicsPV("badPMTs", badPMTs);
  int numBS = 0;
  for (auto included : m_includedBoardstacks) if (not included) numBS++;
  setEpicsPV("numExcludedBS", numBS);
  setEpicsPV("histoAlarmState", getOffcialAlarmStatus(m_alarmStateOverall));
  setEpicsPV("backgroundAlarmLevels", m_averageRate);

  B2DEBUG(20, "badBoardstacks: " << badBoardstacks);
  B2DEBUG(20, "badCarriers: " << badCarriers);
  B2DEBUG(20, "badAsics: " << badAsics);
  B2DEBUG(20, "badPMTs: " << badPMTs);
  B2DEBUG(20, "excludedBS: " << numBS);
  B2DEBUG(20, "histoAlarmState: " << getOffcialAlarmStatus(m_alarmStateOverall));
  B2DEBUG(20, "backgroundAlarmLevels" << m_averageRate);
}

void DQMHistAnalysisTOPModule::updateLimits()
{
  double unused = 0;

  double yLo = m_asicWindowsBand[0];
  double yHi = m_asicWindowsBand[1];
  requestLimitsFromEpicsPVs("asicWindowsBand", yLo, unused, unused, yHi);
  m_asicWindowsBand[0] = yLo;
  m_asicWindowsBand[1] = yHi;

  requestLimitsFromEpicsPVs("asicWindowsAlarmLevels", unused, unused, m_asicWindowsAlarmLevels[0], m_asicWindowsAlarmLevels[1]);
  requestLimitsFromEpicsPVs("eventMonitorAlarmLevels", unused, unused, m_eventMonitorAlarmLevels[0], m_eventMonitorAlarmLevels[1]);
  requestLimitsFromEpicsPVs("junkHitsAlarmLevels", unused, unused, m_junkHitsAlarmLevels[0], m_junkHitsAlarmLevels[1]);
  requestLimitsFromEpicsPVs("deadChannelsAlarmLevels", unused, unused, m_deadChannelsAlarmLevels[0], m_deadChannelsAlarmLevels[1]);
  requestLimitsFromEpicsPVs("backgroundAlarmLevels", unused, unused, m_backgroundAlarmLevels[0], m_backgroundAlarmLevels[1]);
  requestLimitsFromEpicsPVs("photonYieldsAlarmLevels", m_photonYieldsAlarmLevels[0], m_photonYieldsAlarmLevels[1], unused, unused);

  requestLimitsFromEpicsPVs("injectionBGAlarmLevels", unused, unused, m_injectionBGAlarmLevels[0], m_injectionBGAlarmLevels[1]);
  requestLimitsFromEpicsPVs("timingAlarmLevels", unused, unused, m_timingAlarmLevels[0], m_timingAlarmLevels[1]);
  requestLimitsFromEpicsPVs("eventT0MeanAlarmLevels", unused, unused, m_eventT0MeanAlarmLevels[0], m_eventT0MeanAlarmLevels[1]);
  requestLimitsFromEpicsPVs("eventT0RmsAlarmLevels", unused, unused, m_eventT0RmsAlarmLevels[0], m_eventT0RmsAlarmLevels[1]);
  requestLimitsFromEpicsPVs("offsetMeanAlarmLevels", unused, unused, m_offsetMeanAlarmLevels[0], m_offsetMeanAlarmLevels[1]);
  requestLimitsFromEpicsPVs("offsetRmsAlarmLevels", unused, unused, m_offsetRmsAlarmLevels[0], m_offsetRmsAlarmLevels[1]);

  setAlarmLines();

  bool status = false;
  std::string excludedBS = getEpicsStringPV("excludedBoardstacks", status);

  if (status) {
    m_excludedBoardstacks.clear();
    std::string name;
    for (auto c : excludedBS) {
      if (isspace(c)) continue;
      else if (ispunct(c)) {
        if (not name.empty()) {
          m_excludedBoardstacks.push_back(name);
          name.clear();
        }
      } else name.push_back(c);
    }
    if (not name.empty()) {
      m_excludedBoardstacks.push_back(name);
    }
    setIncludedBoardstacks(m_excludedBoardstacks);
  }

  B2DEBUG(20, "asicWindowsBand:         [" << m_asicWindowsBand[0] << ", " << m_asicWindowsBand[1] << "]");
  B2DEBUG(20, "asicWindowsAlarmLevels:  [" << m_asicWindowsAlarmLevels[0] << ", " << m_asicWindowsAlarmLevels[1] << "]");
  B2DEBUG(20, "eventMonitorAlarmLevels: [" << m_eventMonitorAlarmLevels[0] << ", " << m_eventMonitorAlarmLevels[1] << "]");
  B2DEBUG(20, "junkHitsAlarmLevels:     [" << m_junkHitsAlarmLevels[0] << ", " << m_junkHitsAlarmLevels[1] << "]");
  B2DEBUG(20, "deadChannelsAlarmLevels: [" << m_deadChannelsAlarmLevels[0] << ", " << m_deadChannelsAlarmLevels[1] << "]");
  B2DEBUG(20, "backgroundAlarmLevels:   [" << m_backgroundAlarmLevels[0] << ", " << m_backgroundAlarmLevels[1] << "]");
  B2DEBUG(20, "photonYieldsAlarmLevels: [" << m_photonYieldsAlarmLevels[0] << ", " << m_photonYieldsAlarmLevels[1] << "]");

  B2DEBUG(20, "injectionBGAlarmLevels:     [" << m_injectionBGAlarmLevels[0] << ", " << m_injectionBGAlarmLevels[1] << "]");
  B2DEBUG(20, "timingAlarmLevels:     [" << m_timingAlarmLevels[0] << ", " << m_timingAlarmLevels[1] << "]");
  B2DEBUG(20, "eventT0MeanAlarmLevels:     [" << m_eventT0MeanAlarmLevels[0] << ", " << m_eventT0MeanAlarmLevels[1] << "]");
  B2DEBUG(20, "eventT0RmsAlarmLevels:     [" << m_eventT0RmsAlarmLevels[0] << ", " << m_eventT0RmsAlarmLevels[1] << "]");
  B2DEBUG(20, "offsetMeanAlarmLevels:     [" << m_offsetMeanAlarmLevels[0] << ", " << m_offsetMeanAlarmLevels[1] << "]");
  B2DEBUG(20, "offsetRmsAlarmLevels:     [" << m_offsetRmsAlarmLevels[0] << ", " << m_offsetRmsAlarmLevels[1] << "]");

  std::string ss;
  for (const auto& s : m_excludedBoardstacks) ss += "'" + s + "', ";
  if (ss.size() > 2)  {ss.pop_back(); ss.pop_back();}
  B2DEBUG(20, "excludedBoardstacks:     [" << ss << "]");

}

void DQMHistAnalysisTOPModule::setIncludedBoardstacks(const std::vector<std::string>& excludedBoardstacks)
{
  m_includedBoardstacks.clear();
  m_includedBoardstacks.resize(64, true);

  for (const auto& bsname : excludedBoardstacks) {
    int id = m_bsmap[bsname];
    if (id > 0) m_includedBoardstacks[id - 1] = false;
    else B2ERROR("Invalid boardstack name: " << bsname);
  }
}
