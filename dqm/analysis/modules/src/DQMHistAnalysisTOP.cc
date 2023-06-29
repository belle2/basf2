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
  B2DEBUG(20, "DQMHistAnalysisTOP: initialized.");

  m_c_photonYields = new TCanvas("TOP/c_photonYields");
  m_c_backgroundRates = new TCanvas("TOP/c_backgroundRates");

  m_deadFraction = new TH1F("TOP/deadFraction", "Fraction of dead channels; slot number; fraction", 16, 0.5, 16.5);
  m_hotFraction = new TH1F("TOP/hotFraction", "Fraction of hot channels; slot number; fraction", 16, 0.5, 16.5);
  m_activeFraction = new TH1F("TOP/activeFraction", "Fraction of active channels; slot number; fraction", 16, 0.5, 16.5);
  m_c_deadAndHot = new TCanvas("TOP/c_deadAndHotChannels");

  //using c2_Name to avoid an overlap on default c_Name
  for (int i = 1; i <= 16; i++) {
    m_c_good_hits_xy.push_back(new TCanvas(Form("TOP/c2_good_hits_xy_%d", i)));
    m_c_bad_hits_xy.push_back(new TCanvas(Form("TOP/c2_bad_hits_xy_%d", i)));
    m_c_good_hits_asics.push_back(new TCanvas(Form("TOP/c2_good_hits_asics_%d", i)));
    m_c_bad_hits_asics.push_back(new TCanvas(Form("TOP/c2_bad_hits_asics_%d", i)));
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
}


void DQMHistAnalysisTOPModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisTOP: beginRun called.");
  m_RunType = findHist("DQMInfo/rtype");
  m_RunTypeString = m_RunType ? m_RunType->GetTitle() : "";
  m_IsNullRun = (m_RunTypeString == "null");
}


void DQMHistAnalysisTOPModule::event()
{
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

  TCanvas* c2 = findCanvas("TOP/c_window_vs_slot");
  if (c2) {
    c2->cd();
    m_text1->Clear();
    m_text1->AddText(Form("Ratio of entries outside of red lines: %.2f %%", m_totalWindowFraction * 100.0));
    m_text1->Draw();
    m_line1->Draw();
    m_line2->Draw();
    int alarmColor = m_IsNullRun ? kWhite : getAlarmColor(m_totalWindowFraction, m_rawTimingAlarmBorders);
    c2->Pad()->SetFillColor(alarmColor);
    c2->Modified();
  }

  // Event desynchronization monitor

  auto* evtMonitor = (TH1F*) findHist("TOP/BoolEvtMonitor");
  double badRatio = 0;
  if (evtMonitor) {
    double totalEvts = evtMonitor->GetEntries();
    double badEvts = evtMonitor->GetBinContent(2);
    if (totalEvts > 0) badRatio = badEvts / totalEvts;
  }

  TCanvas* c3 = findCanvas("TOP/c_BoolEvtMonitor");
  if (c3) {
    c3->cd();
    int alarmColor = m_IsNullRun ? kWhite : getAlarmColor(badRatio, m_eventMonitorAlarmBorders);
    c3->Pad()->SetFillColor(alarmColor);
    c3->Modified();
  }

  // Fraction of dead and hot channels

  m_deadFraction->Reset();
  m_hotFraction->Reset();
  m_activeFraction->Reset();

  for (int slot = 1; slot <= 16; slot++) {
    TString hname = "TOP/good_channel_hits_";
    hname += slot;
    auto* h = (TH1F*) findHist(std::string(hname));
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
  m_deadFraction->GetYaxis()->SetRangeUser(0, 1);

  m_hotFraction->Scale(1.0 / 512, "nosw2");
  m_hotFraction->SetFillColor(2);
  m_hotFraction->GetYaxis()->SetRangeUser(0, 1);

  m_activeFraction->Scale(1.0 / 512, "nosw2");
  m_activeFraction->SetFillColor(0);
  m_activeFraction->GetYaxis()->SetRangeUser(0, 1);

  auto* stack = new THStack("TOP/stack", "Fraction of dead and hot channels; slot number; fraction");
  stack->Add(m_deadFraction);
  stack->Add(m_hotFraction);
  stack->Add(m_activeFraction);

  m_c_deadAndHot->cd();
  m_c_deadAndHot->Pad()->SetFrameFillColor(10);
  stack->Draw();

  auto* legend = new TLegend(0.8, 0.87, 0.99, 0.99);
  legend->AddEntry(m_hotFraction, "hot");
  legend->AddEntry(m_deadFraction, "dead");
  legend->Draw("same");

  int alarmColor = m_IsNullRun ? kWhite : getAlarmColor(1 - m_activeFraction->GetMinimum(), m_deadChannelsAlarmBorders);
  m_c_deadAndHot->Pad()->SetFillColor(alarmColor);
  m_c_deadAndHot->Modified();

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
    m_photonYields->Divide(m_photonYields, activeFract);
    m_c_photonYields->cd();
    m_photonYields->Draw();
    m_c_photonYields->Modified();

    m_backgroundRates->Scale(1.0 / 50.0e-3 / 32);  // measured in 50 ns window, 32 PMT's ==> rate in MHz/PMT
    m_backgroundRates->Divide(m_backgroundRates, activeFract);
    m_backgroundRates->SetTitle("Background rates");
    m_backgroundRates->SetYTitle("background rate [MHz/PMT]");
    m_backgroundRates->SetMarkerStyle(24);
    m_c_backgroundRates->cd();
    m_backgroundRates->Draw();
    m_c_backgroundRates->Modified();

    delete activeFract;
  }

  // Remake plots of 2D hit distributions: set z-axis range to 3 times the average for good hits, 30 times the average for bad hits

  remake2DHistograms("TOP/good_hits_xy_", m_c_good_hits_xy, 3);
  remake2DHistograms("TOP/bad_hits_xy_", m_c_bad_hits_xy, 30);
  remake2DHistograms("TOP/good_hits_asics_", m_c_good_hits_asics, 3);
  remake2DHistograms("TOP/bad_hits_asics_", m_c_bad_hits_asics, 30);

}


void DQMHistAnalysisTOPModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisTOP : endRun called");

  // add MiraBelle monitoring

  setMiraBelleVariables("RateBadRaw_slot", m_windowFractions);
  m_monObj->setVariable("RateBadRaw_all", m_totalWindowFraction);
  B2INFO("RateBadRaw_all " << m_totalWindowFraction); //TODO rm
  setMiraBelleVariables("PhotonsPerTrack_slot", m_photonYields);
  setMiraBelleVariables("BackgroundRate_slot", m_backgroundRates);
  setMiraBelleVariables("ActiveChannelFraction_slot", m_activeFraction);

}


void DQMHistAnalysisTOPModule::terminate()
{
  B2DEBUG(20, "terminate called");
}


void DQMHistAnalysisTOPModule::remake2DHistograms(const TString& name, const std::vector<TCanvas*>& canvases, double scale)
{

  double totalHits = 0;
  std::vector<TH2F*> histos;
  for (int module = 1; module <= 16; module++) {
    TString hname = name;
    hname += module;
    TH2F* h = (TH2F*) findHist(std::string(hname));
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


void DQMHistAnalysisTOPModule::setMiraBelleVariables(const TString& variableName, const TH1* histogram)
{
  for (int slot = 1; slot <= 16; slot++) {
    TString vname = variableName + TString::Format("%d", slot);
    double value = histogram ? histogram->GetBinContent(slot) : 0;
    m_monObj->setVariable(std::string(vname), value);
    B2DEBUG(20, vname << " " << value);

    B2INFO(vname << " " << value); //TODO rm
  }
}


int DQMHistAnalysisTOPModule::getAlarmColor(double value, const std::vector<double>& alarmBorders) const
{
  if (value < alarmBorders[0]) return kGreen;
  else if (value < alarmBorders[1]) return kYellow;
  else return kRed;
}
