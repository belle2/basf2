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
#include "TROOT.h"
#include <TProfile.h>
#include <THStack.h>
#include <TLegend.h>

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
  setDescription("Histogram analysis module for TOP DQM");

  // Add parameters
  addParam("rawTimingBand", m_rawTimingBand,
           "lower and upper bin of a band denoting good windows", m_rawTimingBand);
  addParam("rawTimingAlarmBorders", m_rawTimingAlarmBorders,
           "alarm borders for the fraction of windows outside the band", m_rawTimingAlarmBorders);
  addParam("eventMonitorAlarmBorders", m_eventMonitorAlarmBorders,
           "alarm borders for the fraction of desynchronized digits", m_eventMonitorAlarmBorders);
  addParam("badHitsAlarmBorders", m_badHitsAlarmBorders,
           "alarm borders for the fraction of junk hits", m_badHitsAlarmBorders);
  addParam("deadChannelsAlarmBorders", m_deadChannelsAlarmBorders,
           "alarm borders for the fraction of dead + hot channels", m_deadChannelsAlarmBorders);

  B2DEBUG(20, "DQMHistAnalysisTOP: Constructor done.");
}


DQMHistAnalysisTOPModule::~DQMHistAnalysisTOPModule() { }


void DQMHistAnalysisTOPModule::initialize()
{

  if (m_rawTimingBand.size() != 2) B2ERROR("Parameter list 'rawTimingBand' must contain two numbers");
  if (m_rawTimingAlarmBorders.size() != 2) B2ERROR("Parameter list 'rawTimingAlarmBorders' must contain two numbers");
  if (m_eventMonitorAlarmBorders.size() != 2) B2ERROR("Parameter list 'eventMonitorAlarmBorders' must contain two numbers");
  if (m_badHitsAlarmBorders.size() != 2) B2ERROR("Parameter list 'badHitsAlarmBorders' must contain two numbers");
  if (m_deadChannelsAlarmBorders.size() != 2) B2ERROR("Parameter list 'deadChannelsAlarmBorders' must contain two numbers");

  m_monObj = getMonitoringObject("top");

  gROOT->cd();

  m_c_photonYields = new TCanvas("TOP/c_photonYields");
  m_c_backgroundRates = new TCanvas("TOP/c_backgroundRates");

  m_deadFraction = new TH1F("TOP/deadFraction", "Fraction of dead channels; slot number; fraction", 16, 0.5, 16.5);
  m_hotFraction = new TH1F("TOP/hotFraction", "Fraction of hot channels; slot number; fraction", 16, 0.5, 16.5);
  m_activeFraction = new TH1F("TOP/activeFraction", "Fraction of active channels; slot number; fraction", 16, 0.5, 16.5);
  m_c_deadAndHot = new TCanvas("TOP/c_deadAndHotChannels");

  m_junkFraction = new TH1F("TOP/junkFraction", "Fraction of bad hits; slot number; fraction", 64, 0.5, 16.5);
  m_c_junkFraction = new TCanvas("TOP/c_junkFraction");

  // using c2_Name to avoid an overlap on default c_Name
  for (int slot = 1; slot <= 16; slot++) {
    m_c_good_hits_xy.push_back(new TCanvas(Form("TOP/c2_good_hits_xy_%d", slot)));
    m_c_bad_hits_xy.push_back(new TCanvas(Form("TOP/c2_bad_hits_xy_%d", slot)));
    m_c_good_hits_asics.push_back(new TCanvas(Form("TOP/c2_good_hits_asics_%d", slot)));
    m_c_bad_hits_asics.push_back(new TCanvas(Form("TOP/c2_bad_hits_asics_%d", slot)));
  }

  m_line1 = new TLine(0.5, m_rawTimingBand[0], 16.5, m_rawTimingBand[0]);
  m_line2 = new TLine(0.5, m_rawTimingBand[1], 16.5, m_rawTimingBand[1]);
  m_line1->SetLineWidth(2);
  m_line2->SetLineWidth(2);
  m_line1->SetLineColor(kRed);
  m_line2->SetLineColor(kRed);

  m_text1 = new TPaveText(1, 435, 10, 500, "NB");
  m_text1->SetFillColorAlpha(kWhite, 0);
  m_text1->SetBorderSize(0);
  m_text2 = new TPaveText(0.55, 0.8, 0.85, 0.89, "NDC");
  m_text2->SetFillColorAlpha(kWhite, 0);
  m_text2->SetBorderSize(0);

  for (int slot = 1; slot < 16; slot++) {
    auto* line = new TLine(slot + 0.5, 0, slot + 0.5, 1);
    line->SetLineWidth(1);
    line->SetLineStyle(2);
    m_verticalLines.push_back(line);
  }

  std::vector<int> colors = {kYellow, kRed};
  for (size_t i = 0; i < colors.size(); i++) {
    auto* line = new TLine(0.5, m_badHitsAlarmBorders[i], 16.5, m_badHitsAlarmBorders[i]);
    line->SetLineWidth(2);
    line->SetLineStyle(2);
    line->SetLineColor(colors[i]);
    m_badHitsAlarmLines.push_back(line);
  }
  for (size_t i = 0; i < colors.size(); i++) {
    auto* line = new TLine(0.5, m_deadChannelsAlarmBorders[i], 16.5, m_deadChannelsAlarmBorders[i]);
    line->SetLineWidth(2);
    line->SetLineStyle(2);
    line->SetLineColor(colors[i]);
    m_deadChannelsAlarmLines.push_back(line);
  }

  B2DEBUG(20, "DQMHistAnalysisTOP: initialized.");
}


void DQMHistAnalysisTOPModule::beginRun()
{
  m_RunType = findHist("DQMInfo/rtype");
  m_RunTypeString = m_RunType ? m_RunType->GetTitle() : "";
  m_IsNullRun = (m_RunTypeString == "null");

  B2DEBUG(20, "DQMHistAnalysisTOP: beginRun called.");
}


void DQMHistAnalysisTOPModule::event()
{

  TCanvas* canvas = nullptr;
  int alarmColor = 0;

  // window_vs_slot

  auto* hraw = (TH2F*) findHist("TOP/window_vs_slot");
  m_totalWindowFraction = 0;
  if (hraw) {
    auto* px = hraw->ProjectionX("tmp_px");
    auto* band = hraw->ProjectionX("TOP/windowFractions", m_rawTimingBand[0], m_rawTimingBand[1]);
    band->Add(px, band, 1, -1);
    double total = px->Integral();
    m_totalWindowFraction = (total != 0) ? band->Integral() / total : 0;
    band->Divide(band, px);
    m_windowFractions = band;
    delete px;
  }

  canvas = findCanvas("TOP/c_window_vs_slot");
  if (canvas) {
    canvas->cd();
    m_text1->Clear();
    m_text1->AddText(Form("Fraction outside red lines: %.2f %%", m_totalWindowFraction * 100.0));
    m_text1->Draw();
    m_line1->Draw();
    m_line2->Draw();
    alarmColor = getAlarmColor(m_totalWindowFraction, m_rawTimingAlarmBorders);
    canvas->Pad()->SetFillColor(alarmColor);
    canvas->Modified();
  }

  // Event desynchronization monitor

  auto* evtMonitor = (TH1F*) findHist("TOP/BoolEvtMonitor");
  double badRatio = 0;
  if (evtMonitor) {
    double totalEvts = evtMonitor->GetEntries();
    double badEvts = evtMonitor->GetBinContent(2);
    if (totalEvts > 0) badRatio = badEvts / totalEvts;
  }

  canvas = findCanvas("TOP/c_BoolEvtMonitor");
  if (canvas) {
    canvas->cd();
    m_text2->Clear();
    m_text2->AddText(Form("Fraction: %.4f %%", badRatio * 100.0));
    m_text2->Draw();
    alarmColor = getAlarmColor(badRatio, m_eventMonitorAlarmBorders);
    canvas->Pad()->SetFillColor(alarmColor);
    canvas->Modified();
  }

  // Fraction of dead and hot channels

  m_deadFraction->Reset();
  m_hotFraction->Reset();
  m_activeFraction->Reset();

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
    for (int chan = 0; chan < h->GetNbinsX(); chan++) {
      double y = h->GetBinContent(chan + 1);
      if (y <= deadCut) {
        m_deadFraction->Fill(slot);
      } else if (y > hotCut) {
        m_hotFraction->Fill(slot);
      } else {
        m_activeFraction->Fill(slot);
      }
    }
  }

  m_deadFraction->Scale(1.0 / 512, "nosw2");
  m_deadFraction->SetFillColor(1);
  m_deadFraction->GetXaxis()->SetNdivisions(16);
  m_deadFraction->GetYaxis()->SetRangeUser(0, 1);

  m_hotFraction->Scale(1.0 / 512, "nosw2");
  m_hotFraction->SetFillColor(2);
  m_hotFraction->GetXaxis()->SetNdivisions(16);
  m_hotFraction->GetYaxis()->SetRangeUser(0, 1);

  m_activeFraction->Scale(1.0 / 512, "nosw2");
  m_activeFraction->SetFillColor(0);
  m_activeFraction->GetXaxis()->SetNdivisions(16);
  m_activeFraction->GetYaxis()->SetRangeUser(0, 1);

  auto* stack = new THStack("TOP/stack", "Fraction of dead and hot channels; slot number; fraction");
  stack->Add(m_deadFraction);
  stack->Add(m_hotFraction);
  stack->Add(m_activeFraction);

  canvas = m_c_deadAndHot;
  canvas->Clear();
  canvas->cd();
  canvas->Pad()->SetFrameFillColor(10);
  stack->Draw();

  for (auto* line : m_deadChannelsAlarmLines) line->Draw("same");

  auto* legend = new TLegend(0.8, 0.87, 0.99, 0.99);
  legend->AddEntry(m_hotFraction, "hot");
  legend->AddEntry(m_deadFraction, "dead");
  legend->Draw("same");

  alarmColor = getAlarmColor(1 - m_activeFraction->GetMinimum(), m_deadChannelsAlarmBorders);
  canvas->Pad()->SetFillColor(alarmColor);
  canvas->Modified();

  // Photon yields and background rates

  auto* signalHits = (TProfile*) findHist("TOP/signalHits");
  auto* backgroundHits = (TProfile*) findHist("TOP/backgroundHits");
  if (signalHits and backgroundHits) {
    m_photonYields = signalHits->ProjectionX("TOP/photonYields");
    m_backgroundRates = backgroundHits->ProjectionX("TOP/backgroundRates");
    auto* activeFract = (TH1F*) m_activeFraction->Clone("tmp");
    for (int i = 1; i <= activeFract->GetNbinsX(); i++) activeFract->SetBinError(i, 0);

    m_photonYields->Add(m_photonYields, m_backgroundRates, 1, -1);
    m_photonYields->SetTitle("Number of photons per track");
    m_photonYields->SetYTitle("photons per track");
    m_photonYields->SetMarkerStyle(24);
    m_photonYields->GetXaxis()->SetNdivisions(16);
    m_photonYields->Divide(m_photonYields, activeFract);
    canvas = m_c_photonYields;
    canvas->Clear();
    canvas->cd();
    m_photonYields->Draw();
    canvas->Modified();

    m_backgroundRates->Scale(1.0 / 50.0e-3 / 32);  // measured in 50 ns window, 32 PMT's ==> rate in MHz/PMT
    m_backgroundRates->Divide(m_backgroundRates, activeFract);
    m_backgroundRates->SetTitle("Background rates");
    m_backgroundRates->SetYTitle("background rate [MHz/PMT]");
    m_backgroundRates->SetMarkerStyle(24);
    m_backgroundRates->GetXaxis()->SetNdivisions(16);
    canvas = m_c_backgroundRates;
    canvas->Clear();
    canvas->cd();
    m_backgroundRates->Draw();
    canvas->Modified();

    delete activeFract;
  }

  // Fractions of junk hits

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
  delete allHits;

  canvas = m_c_junkFraction;
  canvas->Clear();
  canvas->cd();
  canvas->Pad()->SetFrameFillColor(10);
  m_junkFraction->SetMarkerStyle(24);
  m_junkFraction->GetXaxis()->SetNdivisions(16);
  m_junkFraction->GetYaxis()->SetRangeUser(0, 1);
  m_junkFraction->Draw();
  for (auto* line : m_verticalLines) line->Draw("same");
  for (auto* line : m_badHitsAlarmLines) line->Draw("same");

  alarmColor = getAlarmColor(m_junkFraction->GetMaximum(), m_badHitsAlarmBorders);
  canvas->Pad()->SetFillColor(alarmColor);
  canvas->Modified();

  // Remake plots of 2D hit distributions: set z-axis range to 3 times the average for good hits, 30 times the average for bad hits

  remake2DHistograms("TOP/good_hits_xy_", m_c_good_hits_xy, 3);
  remake2DHistograms("TOP/bad_hits_xy_", m_c_bad_hits_xy, 30);
  remake2DHistograms("TOP/good_hits_asics_", m_c_good_hits_asics, 3);
  remake2DHistograms("TOP/bad_hits_asics_", m_c_bad_hits_asics, 30);

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


void DQMHistAnalysisTOPModule::remake2DHistograms(const std::string& name, const std::vector<TCanvas*>& canvases, double scale)
{

  double totalHits = 0;
  std::vector<TH2F*> histos;
  for (int slot = 1; slot <= 16; slot++) {
    TH2F* h = (TH2F*) findHist(name + std::to_string(slot));
    histos.push_back(h);
    if (not h) continue;
    totalHits += h->Integral();
  }
  double average = totalHits / 512 / 16;  // per pixel or asic channel

  if (histos.size() != canvases.size()) {
    B2ERROR("Number of histograms not the same as the number of canvases");
    return;
  }

  for (size_t i = 0; i < histos.size(); i++) {
    auto* canvas = canvases[i];
    canvas->Clear();
    canvas->cd();
    auto* h = histos[i];
    if (h and  average > 0) {
      h->GetZaxis()->SetRangeUser(0, average * scale);
      h->SetDrawOption("COLZ");
      h->Draw("COLZ");
    }
    canvas->Modified();
  }

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


int DQMHistAnalysisTOPModule::getAlarmColor(double value, const std::vector<double>& alarmBorders) const
{
  if (m_IsNullRun) return kWhite;

  if (value < alarmBorders[0]) return kGreen;
  else if (value < alarmBorders[1]) return kYellow;
  else return kRed;
}
