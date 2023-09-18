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
#include <TH2.h>
#include <TF1.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TProfile.h>
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
  addParam("rawTimingBand", m_rawTimingBand,
           "lower and upper bin of a band denoting good windows", m_rawTimingBand);
  addParam("rawTimingAlarmLevels", m_rawTimingAlarmLevels,
           "alarm levels for the fraction of windows outside the band (yellow, red)", m_rawTimingAlarmLevels);
  addParam("eventMonitorAlarmLevels", m_eventMonitorAlarmLevels,
           "alarm levels for the fraction of desynchronized digits (yellow, red)", m_eventMonitorAlarmLevels);
  addParam("badHitsAlarmLevels", m_badHitsAlarmLevels,
           "alarm levels for the fraction of junk hits (yellow, red)", m_badHitsAlarmLevels);
  addParam("deadChannelsAlarmLevels", m_deadChannelsAlarmLevels,
           "alarm levels for the fraction of dead + hot channels (yellow, red)", m_deadChannelsAlarmLevels);
  addParam("backgroundAlarmLevels", m_backgroundAlarmLevels,
           "alarm levels for background rates [MHz/PMT] (yellow, red)", m_backgroundAlarmLevels);
  addParam("photonYieldsAlarmLevels", m_photonYieldsAlarmLevels,
           "alarm levels for the number of photons per track (red, yellow)", m_photonYieldsAlarmLevels);
  addParam("excludedBoardstacks", m_excludedBoardstacks,
           "boarstacks to be excluded from alarming. Names are given like '5c', '13d' etc.", m_excludedBoardstacks);

  B2DEBUG(20, "DQMHistAnalysisTOP: Constructor done.");
}


DQMHistAnalysisTOPModule::~DQMHistAnalysisTOPModule() { }


void DQMHistAnalysisTOPModule::initialize()
{

  if (m_rawTimingBand.size() != 2) B2ERROR("Parameter list 'rawTimingBand' must contain two numbers");
  if (m_rawTimingAlarmLevels.size() != 2) B2ERROR("Parameter list 'rawTimingAlarmLevels' must contain two numbers");
  if (m_eventMonitorAlarmLevels.size() != 2) B2ERROR("Parameter list 'eventMonitorAlarmLevels' must contain two numbers");
  if (m_badHitsAlarmLevels.size() != 2) B2ERROR("Parameter list 'badHitsAlarmLevels' must contain two numbers");
  if (m_deadChannelsAlarmLevels.size() != 2) B2ERROR("Parameter list 'deadChannelsAlarmLevels' must contain two numbers");
  if (m_backgroundAlarmLevels.size() != 2) B2ERROR("Parameter list 'backgroundAlarmLevels' must contain two numbers");
  if (m_photonYieldsAlarmLevels.size() != 2) B2ERROR("Parameter list 'photonYieldsAlarmLevels' must contain two numbers");

  if (not m_excludedBoardstacks.empty()) {
    std::map<std::string, int> bsmap;
    int id = 1;
    for (int slot = 1; slot <= 16; slot++) {
      string slotstr = to_string(slot);
      for (std::string bs : {"a", "b", "c", "d"}) {
        bsmap[slotstr + bs] = id;
        id++;
      }
    }
    m_excludedBS.resize(64, false);
    for (const auto& bsname : m_excludedBoardstacks) {
      id = bsmap[bsname];
      if (id > 0) m_excludedBS[id - 1] = true;
      else B2ERROR("Invalid boardstack name: " << bsname);
    }
  }

  m_monObj = getMonitoringObject("top");

  gROOT->cd();

  m_c_photonYields = new TCanvas("TOP/c_photonYields", "c_photonYields");
  m_c_backgroundRates = new TCanvas("TOP/c_backgroundRates", "c_backgroundRates");

  m_deadFraction = new TH1F("TOP/deadFraction", "Fraction of dead channels; slot number; fraction", 16, 0.5, 16.5);
  m_hotFraction = new TH1F("TOP/hotFraction", "Fraction of hot channels; slot number; fraction", 16, 0.5, 16.5);
  m_activeFraction = new TH1F("TOP/activeFraction", "Fraction of active channels; slot number; fraction", 16, 0.5, 16.5);
  m_c_deadAndHot = new TCanvas("TOP/c_deadAndHotChannels", "c_deadAndHotChannels");

  m_junkFraction = new TH1F("TOP/junkFraction", "Fraction of bad hits per boardstack; slot number; fraction", 64, 0.5, 16.5);
  m_c_junkFraction = new TCanvas("TOP/c_junkFraction", "c_junkFraction");

  for (auto y : m_rawTimingBand) {
    auto* line = new TLine(0.5, y, 16.5, y);
    line->SetLineWidth(2);
    line->SetLineColor(kRed);
    m_rawTimingBandLines.push_back(line);
  }

  m_text1 = new TPaveText(1, 435, 12, 500, "NB");
  m_text1->SetFillColorAlpha(kWhite, 0);
  m_text1->SetBorderSize(0);
  m_text2 = new TPaveText(0.55, 0.8, 0.85, 0.89, "NDC");
  m_text2->SetFillColorAlpha(kWhite, 0);
  m_text2->SetBorderSize(0);
  m_text3 = new TPaveText(0.47, 0.8, 0.85, 0.89, "NDC");
  m_text3->SetFillColorAlpha(kWhite, 0);
  m_text3->SetBorderSize(0);

  for (int slot = 1; slot < 16; slot++) {
    auto* line = new TLine(slot + 0.5, 0, slot + 0.5, 1);
    line->SetLineWidth(1);
    line->SetLineStyle(2);
    m_verticalLines.push_back(line);
  }

  setAlarmLines(m_badHitsAlarmLevels, 0.5, 16.5, m_badHitsAlarmLines);
  setAlarmLines(m_deadChannelsAlarmLevels, 0.5, 16.5, m_deadChannelsAlarmLines);
  setAlarmLines(m_backgroundAlarmLevels, 0.5, 16.5, m_backgroundAlarmLines);
  setAlarmLines(m_photonYieldsAlarmLevels, 0.5, 16.5, m_photonYieldsAlarmLines, false);

  B2DEBUG(20, "DQMHistAnalysisTOP: initialized.");
}


void DQMHistAnalysisTOPModule::beginRun()
{
  auto* h = findHist("DQMInfo/rtype");
  std::string title = h ? h->GetTitle() : "";
  m_IsNullRun = (title == "null");

  B2DEBUG(20, "DQMHistAnalysisTOP: beginRun called.");
}


void DQMHistAnalysisTOPModule::event()
{
  bool zeroSupp = gStyle->GetHistMinimumZero();
  gStyle->SetHistMinimumZero(true);

  // Update window_vs_slot canvas w/ alarming
  updateWindowVsSlotCanvas();

  // Update event desynchronization monitor w/ alarming
  updateEventMonitorCanvas();

  // Fraction of dead and hot channels
  const auto* activeFraction = makeDeadAndHotFractionsPlot();

  // Photon yields and background rates, corrected for dead and hot channels
  makePhotonYieldsAndBGRatePlots(activeFraction);

  // Fractions of junk hits
  makeJunkFractionPlot();

  // Set z-axis range to 3 times the average for good hits, 30 times the average for bad hits
  setZAxisRange("TOP/good_hits_xy_", 3);
  setZAxisRange("TOP/bad_hits_xy_", 30);
  setZAxisRange("TOP/good_hits_asics_", 3);
  setZAxisRange("TOP/bad_hits_asics_", 30);

  // Background subtracted time distributions
  makeBGSubtractedTimimgPlot("goodHitTimes");
  for (int slot = 1; slot <= 16; slot++) {
    makeBGSubtractedTimimgPlot("good_timing_" + to_string(slot));
  }

  // Set logy
  auto* canvas = findCanvas("TOP/c_goodHitsPerEventAll");
  if (canvas) canvas->SetLogy();
  canvas = findCanvas("TOP/c_badHitsPerEventAll");
  if (canvas) canvas->SetLogy();

  gStyle->SetHistMinimumZero(zeroSupp);
}


void DQMHistAnalysisTOPModule::endRun()
{
  // add MiraBelle monitoring

  setMiraBelleVariables("RateBadRaw_slot", m_windowFractions);
  m_monObj->setVariable("RateBadRaw_all", m_totalWindowFraction);
  setMiraBelleVariables("PhotonsPerTrack_slot", m_photonYields);
  setMiraBelleVariables("BackgroundRate_slot", m_backgroundRates);
  setMiraBelleVariables("ActiveChannelFraction_slot", m_activeFraction);

  B2DEBUG(20, "DQMHistAnalysisTOP : endRun called");
}


void DQMHistAnalysisTOPModule::terminate()
{
  B2DEBUG(20, "terminate called");
}


void DQMHistAnalysisTOPModule::updateWindowVsSlotCanvas()
{
  m_totalWindowFraction = 0;   // used also for MiraBelle
  m_windowFractions = nullptr; // used also for MiraBelle
  int alarmState = c_Gray;
  m_text1->Clear();

  auto* hraw = (TH2F*) findHist("TOP/window_vs_slot");
  if (hraw) {
    auto* px = hraw->ProjectionX("tmp_px");
    auto* band = hraw->ProjectionX("TOP/windowFractions", m_rawTimingBand[0], m_rawTimingBand[1]);
    band->Add(px, band, 1, -1);
    double total = px->Integral();
    m_totalWindowFraction = (total != 0) ? band->Integral() / total : 0;
    band->Divide(band, px);
    m_windowFractions = band;
    if (total > 0) {
      alarmState = getAlarmState(m_totalWindowFraction, m_rawTimingAlarmLevels);
      m_text1->AddText(Form("Fraction outside red lines: %.2f %%", m_totalWindowFraction * 100.0));
    }
    delete px;
  }

  auto* canvas = findCanvas("TOP/c_window_vs_slot");
  if (canvas) {
    canvas->cd();
    m_text1->Draw();
    for (auto* line : m_rawTimingBandLines) line->Draw();
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

  auto* canvas = findCanvas("TOP/c_BoolEvtMonitor");
  if (canvas) {
    canvas->cd();
    m_text2->Draw();
    canvas->Pad()->SetFillColor(getAlarmColor(alarmState));
    canvas->Modified();
  }
}


const TH1F* DQMHistAnalysisTOPModule::makeDeadAndHotFractionsPlot()
{
  m_deadFraction->Reset();
  m_hotFraction->Reset();
  m_activeFraction->Reset();
  double inactiveFract = 0; // max inactive channel fraction when some boardstacks are excluded from alarming

  for (int slot = 1; slot <= 16; slot++) {
    auto* h = (TH1F*) findHist("TOP/good_channel_hits_" + std::to_string(slot));
    if (not h) continue;
    double mean = 0;
    int n = 0;
    for (int chan = 0; chan < h->GetNbinsX(); chan++) {
      double y = h->GetBinContent(chan + 1);
      if (y > 0) {
        mean += y;
        n++;
      }
    }
    if (n > 0) mean /= n;
    double deadCut = mean / 10;
    double hotCut = mean * 10;
    double deadFract = 0;
    double hotFract = 0;
    double deadFractIncl = 0;
    double hotFractIncl = 0;
    for (int chan = 0; chan < h->GetNbinsX(); chan++) {
      double y = h->GetBinContent(chan + 1);
      int bs = chan / 128 + (slot - 1) * 4;
      bool included = m_excludedBS.empty() or not m_excludedBS[bs];
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
    m_deadFraction->SetBinContent(slot, deadFract);
    m_hotFraction->SetBinContent(slot, hotFract);
    m_activeFraction->SetBinContent(slot, 1 - deadFract - hotFract);
    inactiveFract = std::max(inactiveFract, (deadFractIncl + hotFractIncl) / h->GetNbinsX());
  }

  int alarmState = c_Gray;
  if (m_activeFraction->Integral() > 0) {
    alarmState = getAlarmState(1 - m_activeFraction->GetMinimum(), m_deadChannelsAlarmLevels);
    if (alarmState == 3) alarmState = std::max(getAlarmState(inactiveFract, m_deadChannelsAlarmLevels), 2);
  }

  m_deadFraction->SetFillColor(1);
  m_deadFraction->GetXaxis()->SetNdivisions(16);

  m_hotFraction->SetFillColor(2);
  m_hotFraction->GetXaxis()->SetNdivisions(16);

  m_activeFraction->SetFillColor(0);
  m_activeFraction->GetXaxis()->SetNdivisions(16);

  auto* canvas = m_c_deadAndHot;
  canvas->Clear();
  canvas->cd();
  canvas->Pad()->SetFrameFillColor(10);
  if (not m_stack) {
    m_stack = new THStack("TOP/stack", "Fraction of dead and hot channels; slot number; fraction");
    m_stack->Add(m_deadFraction);
    m_stack->Add(m_hotFraction);
    m_stack->Add(m_activeFraction);
  }
  m_stack->Draw();

  for (auto* line : m_deadChannelsAlarmLines) line->Draw("same");

  if (not m_legend) {
    m_legend = new TLegend(0.8, 0.87, 0.99, 0.99);
    m_legend->AddEntry(m_hotFraction, "hot");
    m_legend->AddEntry(m_deadFraction, "dead");
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

  auto* signalHits = (TProfile*) findHist("TOP/signalHits");
  if (not signalHits) return;

  auto* backgroundHits = (TProfile*) findHist("TOP/backgroundHits");
  if (not backgroundHits) return;

  m_photonYields = signalHits->ProjectionX("TOP/photonYields");
  m_backgroundRates = backgroundHits->ProjectionX("TOP/backgroundRates");
  auto* activeFract = (TH1F*) activeFraction->Clone("tmp");
  for (int i = 1; i <= activeFract->GetNbinsX(); i++) activeFract->SetBinError(i, 0);

  m_photonYields->Add(m_photonYields, m_backgroundRates, 1, -1);
  m_photonYields->Divide(m_photonYields, activeFract);

  int alarmState = c_Gray;
  if (signalHits->GetEntries() > 0 and activeFraction->Integral() > 0) {
    double hmin = 1000;
    for (int i = 1; i <= m_photonYields->GetNbinsX(); i++) {
      hmin = std::min(hmin, m_photonYields->GetBinContent(i) + 3 * m_photonYields->GetBinError(i));
    }
    alarmState = getAlarmState(hmin, m_photonYieldsAlarmLevels, false);
  }

  m_photonYields->SetTitle("Number of photons per track");
  m_photonYields->SetYTitle("photons per track");
  m_photonYields->SetMarkerStyle(24);
  m_photonYields->GetXaxis()->SetNdivisions(16);

  auto* canvas = m_c_photonYields;
  canvas->cd();
  m_photonYields->Draw();
  for (auto* line : m_photonYieldsAlarmLines) line->Draw("same");
  canvas->Pad()->SetFillColor(getAlarmColor(alarmState));
  canvas->Modified();

  m_backgroundRates->Scale(1.0 / 50.0e-3 / 32);  // measured in 50 ns window, 32 PMT's ==> rate in MHz/PMT
  m_backgroundRates->Divide(m_backgroundRates, activeFract);

  alarmState = c_Gray;
  m_text3->Clear();
  if (backgroundHits->GetEntries() > 0 and activeFraction->Integral() > 0) {
    int status = m_backgroundRates->Fit("pol0", "Q0");
    if (status == 0) {
      auto* fun = m_backgroundRates->GetFunction("pol0");
      if (fun) {
        double average = fun->GetParameter(0);
        double error = fun->GetParError(0);
        alarmState = getAlarmState(average - 3 * error, m_backgroundAlarmLevels);
        m_text3->AddText(Form("Average: %.2f MHz/PMT", average));
      }
    }
  }

  m_backgroundRates->SetTitle("Background rates");
  m_backgroundRates->SetYTitle("background rate [MHz/PMT]");
  m_backgroundRates->SetMarkerStyle(24);
  m_backgroundRates->GetXaxis()->SetNdivisions(16);

  canvas = m_c_backgroundRates;
  canvas->cd();
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
    alarmState = getAlarmState(m_junkFraction->GetMaximum(), m_badHitsAlarmLevels);
    if (not m_excludedBS.empty() and alarmState == 3) {
      double hmax = 0;
      for (size_t i = 0; i < m_excludedBS.size(); i++) {
        if (not m_excludedBS[i]) hmax = std::max(hmax, m_junkFraction->GetBinContent(i + 1));
      }
      alarmState = std::max(getAlarmState(hmax, m_badHitsAlarmLevels), 2);
    }
  }
  delete allHits;

  auto* canvas = m_c_junkFraction;
  canvas->Clear();
  canvas->cd();
  canvas->Pad()->SetFrameFillColor(10);
  canvas->Pad()->SetFillColor(getAlarmColor(alarmState));
  m_junkFraction->SetMarkerStyle(24);
  m_junkFraction->GetXaxis()->SetNdivisions(16);
  m_junkFraction->GetYaxis()->SetRangeUser(0, 1); // Note: m_junkFraction->GetMaximum() will now give 1 and not the histogram maximum!
  m_junkFraction->Draw();
  for (auto* line : m_verticalLines) line->Draw("same");
  for (auto* line : m_badHitsAlarmLines) line->Draw("same");
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

  for (auto* h : histos) h->GetZaxis()->SetRangeUser(0, average * scale);
}


void DQMHistAnalysisTOPModule::makeBGSubtractedTimimgPlot(const std::string& name)
{
  auto* canvas = findCanvas("TOP/c_" + name);
  if (not canvas) return;

  auto* h = (TH1F*) findHist("TOP/" + name);
  if (not h) return;

  auto* hb = (TH1F*) findHist("TOP/" + name + "BG");
  if (not hb) return;

  // use the content of bins at t < 0 to scale the background

  int i0 = h->GetXaxis()->FindBin(0.); // bin at t = 0
  double s = h->Integral(1, i0);
  if (s == 0) return;
  double sb = hb->Integral(1, i0);
  if (sb == 0) return;
  h->Add(h, hb, 1, -s / sb);
  TString title = TString(h->GetTitle()) + " (BG subtracted)";
  h->SetTitle(title);

  canvas->Clear();
  canvas->cd();
  h->Draw();
  canvas->Modified();
}


void DQMHistAnalysisTOPModule::setMiraBelleVariables(const std::string& variableName, const TH1* histogram)
{
  for (int slot = 1; slot <= 16; slot++) {
    auto vname = variableName + std::to_string(slot);
    double value = histogram ? histogram->GetBinContent(slot) : 0;
    m_monObj->setVariable(vname, value);

    B2DEBUG(20, vname << " " << value);
  }
}


int DQMHistAnalysisTOPModule::getAlarmState(double value, const std::vector<double>& alarmLevels, bool bigRed) const
{
  if (m_IsNullRun) return c_Gray;

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
