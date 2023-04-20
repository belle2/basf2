/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <dqm/analysis/modules/DQMHistAnalysisKLM2.h>

/* Basf2 headers. */
#include <klm/dataobjects/KLMChannelIndex.h>

/* ROOT headers. */
#include <TROOT.h>



using namespace Belle2;

REG_MODULE(DQMHistAnalysisKLM2);

DQMHistAnalysisKLM2Module::DQMHistAnalysisKLM2Module()
  : DQMHistAnalysisModule(),
    m_EklmElementNumbers{&(EKLMElementNumbers::Instance())}
{
  setDescription("Module used to analyze KLM Efficiency DQM histograms.");
  addParam("HistogramDirectoryName", m_histogramDirectoryName, "Name of histogram directory", std::string("KLMEfficiencyDQM"));
  addParam("MinEvents", m_minEvents, "Minimum events for delta histogram update", 50000.);
  m_PlaneLine.SetLineColor(kMagenta);
  m_PlaneLine.SetLineWidth(1);
  m_PlaneLine.SetLineStyle(2); // dashed
  m_PlaneText.SetTextAlign(22); // centered, middle
  m_PlaneText.SetTextColor(kMagenta);
  m_PlaneText.SetTextFont(42); // Helvetica regular
  m_PlaneText.SetTextSize(0.02); // 2% of TPad's full height
}

DQMHistAnalysisKLM2Module::~DQMHistAnalysisKLM2Module()
{
}

void DQMHistAnalysisKLM2Module::initialize()
{
  m_monObj = getMonitoringObject("klm");

  gROOT->cd();
  m_c_eff_bklm = new TCanvas((m_histogramDirectoryName + "/c_eff_bklm_plane").data());
  m_c_eff_eklm = new TCanvas((m_histogramDirectoryName + "/c_eff_eklm_plane").data());
  m_c_eff_bklm_sector = new TCanvas((m_histogramDirectoryName + "/c_eff_bklm_sector").data());
  m_c_eff_eklm_sector = new TCanvas((m_histogramDirectoryName + "/c_eff_eklm_sector").data());

  m_eff_bklm = new TH1F((m_histogramDirectoryName + "/eff_bklm_plane").data(),
                        "Plane Efficiency in BKLM",
                        BKLMElementNumbers::getMaximalLayerGlobalNumber(), 0.5, 0.5 + BKLMElementNumbers::getMaximalLayerGlobalNumber());
  m_eff_bklm->GetXaxis()->SetTitle("Layer number");
  m_eff_bklm->SetStats(false);
  m_eff_bklm->SetOption("HIST");

  m_eff_eklm = new TH1F((m_histogramDirectoryName + "/eff_eklm_plane").data(),
                        "Plane Efficiency in EKLM",
                        EKLMElementNumbers::getMaximalPlaneGlobalNumber(), 0.5, 0.5 + EKLMElementNumbers::getMaximalPlaneGlobalNumber());
  m_eff_eklm->GetXaxis()->SetTitle("Plane number");
  m_eff_eklm->SetStats(false);
  m_eff_eklm->SetOption("HIST");

  m_eff_bklm_sector = new TH1F((m_histogramDirectoryName + "/eff_bklm_sector").data(),
                               "Sector Efficiency in BKLM",
                               BKLMElementNumbers::getMaximalSectorGlobalNumber(), 0.5, BKLMElementNumbers::getMaximalSectorGlobalNumber() + 0.5);
  m_eff_bklm_sector->GetXaxis()->SetTitle("Sector number");
  m_eff_bklm_sector->SetStats(false);
  m_eff_bklm_sector->SetOption("HIST");

  m_eff_eklm_sector = new TH1F((m_histogramDirectoryName + "/eff_eklm_sector").data(),
                               "Sector Efficiency in EKLM",
                               EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder(), 0.5, EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder() + 0.5);
  m_eff_eklm_sector->GetXaxis()->SetTitle("Sector number");
  m_eff_eklm_sector->SetStats(false);
  m_eff_eklm_sector->SetOption("HIST");

  /* register plots for delta histogramming */
  // all ext hits
  addDeltaPar(m_histogramDirectoryName, "all_ext_hitsBKLM", HistDelta::c_Events, m_minEvents, 1);
  addDeltaPar(m_histogramDirectoryName, "all_ext_hitsEKLM", HistDelta::c_Events, m_minEvents, 1);
  addDeltaPar(m_histogramDirectoryName, "all_ext_hitsBKLMSector", HistDelta::c_Events, m_minEvents, 1);
  addDeltaPar(m_histogramDirectoryName, "all_ext_hitsEKLMSector", HistDelta::c_Events, m_minEvents, 1);

  // matched hits
  addDeltaPar(m_histogramDirectoryName, "matched_hitsBKLM", HistDelta::c_Events, m_minEvents, 1);
  addDeltaPar(m_histogramDirectoryName, "matched_hitsEKLM", HistDelta::c_Events, m_minEvents, 1);
  addDeltaPar(m_histogramDirectoryName, "matched_hitsBKLMSector", HistDelta::c_Events, m_minEvents, 1);
  addDeltaPar(m_histogramDirectoryName, "matched_hitsEKLMSector", HistDelta::c_Events, m_minEvents, 1);


}


void DQMHistAnalysisKLM2Module::beginRun()
{
  m_RunType = findHist("DQMInfo/rtype");
  m_RunTypeString = m_RunType ? m_RunType->GetTitle() : "";
  m_IsPhysicsRun = (m_RunTypeString == "physics");
}

void DQMHistAnalysisKLM2Module::endRun()
{
  std::string name;

  // Looping over the sectors
  for (int bin = 0; bin < m_eff_bklm_sector->GetXaxis()->GetNbins(); bin++) {
    name = "eff_B";
    if (bin < 8)
      name += "B";
    else
      name += "F";
    name += std::to_string(bin % 8);
    m_monObj->setVariable(name, m_eff_bklm_sector->GetBinContent(bin + 1));
  }

  for (int bin = 0; bin < m_eff_eklm_sector->GetXaxis()->GetNbins(); bin++) {
    name = "eff_E";
    if (bin < 4)
      name += "B";
    else
      name += "F";
    name += std::to_string(bin % 4);
    m_monObj->setVariable(name, m_eff_eklm_sector->GetBinContent(bin + 1));
  }

  // Looping over the planes
  for (int layer = 0; layer < m_eff_bklm->GetXaxis()->GetNbins(); layer++) {
    name = "eff_B";
    if (layer / 15 < 8) {
      name += "B";
    } else {
      name += "F";
    }
    name += std::to_string(int(layer / 15) % 8) + "_layer" + std::to_string(1 + (layer % 15));
    m_monObj->setVariable(name, m_eff_bklm->GetBinContent(layer + 1));
  }
  for (int layer = 0; layer < m_eff_eklm->GetXaxis()->GetNbins(); layer++) {
    name = "eff_E";
    if (layer / 8 < 12)
      name += "B" + std::to_string(layer / 8 + 1);
    else
      name += "F" + std::to_string(layer / 8 - 11);
    name +=  + "_num" + std::to_string(((layer) % 8) + 1);
    m_monObj->setVariable(name, m_eff_eklm->GetBinContent(layer + 1));

  }
}

void DQMHistAnalysisKLM2Module::processEfficiencyHistogram(TH1* effHist, TH1* denominator, TH1* numerator, TCanvas* canvas)
{
  effHist->Reset();
  TH1* effClone = (TH1*)effHist->Clone(); //will be useful for delta plots
  if (denominator != nullptr && numerator != nullptr) {
    canvas->cd();
    effHist->Divide(numerator, denominator, 1, 1, "B");
    effHist->Draw();
    canvas->Modified();
    canvas->Update();

    /* delta component */
    auto deltaDenom = getDelta("", denominator->GetName());
    auto deltaNumer = getDelta("", numerator->GetName());

    //both histograms should have the same update condition but checking both should be okay?
    UpdateCanvas(canvas->GetName(), deltaNumer != nullptr);
    if ((deltaNumer != nullptr) && (deltaDenom != nullptr)) {
      effClone->Divide(deltaNumer, deltaDenom, 1, 1, "B");
      effClone->Draw("SAME");
      canvas->Modified();
      canvas->Update();
    }
  }

}

void DQMHistAnalysisKLM2Module::processPlaneHistogram(
  const std::string& histName, TH1* histogram)
{
  std::string name;
  TCanvas* canvas = findCanvas(m_histogramDirectoryName + "/c_" + histName);
  if (canvas == NULL) {
    B2ERROR("KLMDQM2 histogram canvas " + m_histogramDirectoryName + "/c_" << histName << " is not found.");
    return;
  }
  canvas->Clear();
  canvas->cd();
  histogram->SetStats(false);
  histogram->Draw();
  double histMin = gPad->GetUymin();
  double histMax = gPad->GetUymax();
  double histRange = histMax - histMin;
  if (histName.find("bklm") != std::string::npos) {
    /* First draw the vertical lines and the sector names. */
    const int maximalLayer = BKLMElementNumbers::getMaximalLayerNumber();
    for (int sector = 0; sector < BKLMElementNumbers::getMaximalSectorGlobalNumber(); ++sector) {
      int bin = maximalLayer * sector + 1;
      double xLine = histogram->GetXaxis()->GetBinLowEdge(bin);
      double xText = histogram->GetXaxis()->GetBinLowEdge(bin + maximalLayer / 2);
      double yText = histMin + 0.98 * histRange;
      if (sector > 0)
        m_PlaneLine.DrawLine(xLine, histMin, xLine, histMin + histRange);
      name = "B";
      if (sector < 8)
        name += "B";
      else
        name += "F";
      name += std::to_string(sector % 8);
      m_PlaneText.DrawText(xText, yText, name.c_str());
    }

  } else {
    /* First draw the vertical lines and the sector names. */
    const double maximalLayer = EKLMElementNumbers::getMaximalLayerGlobalNumber();
    const double maxPlane = EKLMElementNumbers::getMaximalPlaneNumber() * EKLMElementNumbers::getMaximalSectorNumber();
    for (int layerGlobal = 1; layerGlobal <= maximalLayer; ++layerGlobal) {
      int bin = maxPlane * layerGlobal + 1;
      double xLine = histogram->GetXaxis()->GetBinLowEdge(bin);
      double xText = histogram->GetXaxis()->GetBinLowEdge(bin - maxPlane / 2);
      double yText = histMin + 0.98 * histRange;
      if (layerGlobal < maximalLayer)
        m_PlaneLine.DrawLine(xLine, histMin, xLine, histMin + histRange);
      int section, layer;
      m_EklmElementNumbers->layerNumberToElementNumbers(
        layerGlobal, &section, &layer);
      if (section == EKLMElementNumbers::c_BackwardSection)
        name = "B";
      else
        name = "F";
      name += std::to_string(layer);
      m_PlaneText.DrawText(xText, yText, name.c_str());
    }
  }
  canvas->Modified();
  canvas->Update();
}

void DQMHistAnalysisKLM2Module::event()
{
  /* If KLM is not included, stop here and return. */
  TH1* daqInclusion = findHist("KLM/daq_inclusion");
  if (not(daqInclusion == NULL)) {
    int isKlmIncluded = daqInclusion->GetBinContent(daqInclusion->GetXaxis()->FindBin("Yes"));
    if (isKlmIncluded == 0)
      return;
  }


  /* Obtain plots necessary for efficiency plots */
  TH1F* all_ext_bklm = (TH1F*)findHist(m_histogramDirectoryName + "/all_ext_hitsBKLM");
  TH1F* matched_hits_bklm = (TH1F*)findHist(m_histogramDirectoryName + "/matched_hitsBKLM");

  TH1F* all_ext_eklm = (TH1F*)findHist(m_histogramDirectoryName + "/all_ext_hitsEKLM");
  TH1F* matched_hits_eklm = (TH1F*)findHist(m_histogramDirectoryName + "/matched_hitsEKLM");


  TH1F* all_ext_bklm_sector = (TH1F*)findHist(m_histogramDirectoryName + "/all_ext_hitsBKLMSector");
  TH1F* matched_hits_bklm_sector = (TH1F*)findHist(m_histogramDirectoryName + "/matched_hitsBKLMSector");

  TH1F* all_ext_eklm_sector = (TH1F*)findHist(m_histogramDirectoryName + "/all_ext_hitsEKLMSector");
  TH1F* matched_hits_eklm_sector = (TH1F*)findHist(m_histogramDirectoryName + "/matched_hitsEKLMSector");

  /* Check if efficiency histograms exist*/
  if ((all_ext_bklm == nullptr || matched_hits_bklm == nullptr) && (m_IsPhysicsRun)) {
    B2INFO("Histograms needed for BKLM plane efficiency computation are not found");
  }

  if ((all_ext_eklm == nullptr || matched_hits_eklm == nullptr) && (m_IsPhysicsRun)) {
    B2INFO("Histograms needed for EKLM plane efficiency computation are not found");
  }
  if ((all_ext_bklm_sector == nullptr || matched_hits_bklm_sector == nullptr) && (m_IsPhysicsRun)) {
    B2INFO("Histograms needed for BKLM sector efficiency computation are not found");
  }
  if ((all_ext_eklm_sector == nullptr || matched_hits_eklm_sector == nullptr) && (m_IsPhysicsRun)) {
    B2INFO("Histograms needed for EKLM sector efficiency computation are not found");
  }


  /* Draw histograms onto canvases*/
  processEfficiencyHistogram(m_eff_bklm, all_ext_bklm, matched_hits_bklm, m_c_eff_bklm);
  processEfficiencyHistogram(m_eff_eklm, all_ext_eklm, matched_hits_eklm, m_c_eff_eklm);

  processEfficiencyHistogram(m_eff_bklm_sector, all_ext_bklm_sector, matched_hits_bklm_sector, m_c_eff_bklm_sector);
  processEfficiencyHistogram(m_eff_eklm_sector, all_ext_eklm_sector, matched_hits_eklm_sector, m_c_eff_eklm_sector);

  processPlaneHistogram("eff_bklm_plane", m_eff_bklm);
  processPlaneHistogram("eff_eklm_plane", m_eff_eklm);

}


