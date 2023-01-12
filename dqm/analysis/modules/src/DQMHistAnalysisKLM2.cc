/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <dqm/analysis/modules/DQMHistAnalysisKLM2.h>

/* Belle 2 headers. */
#include <klm/dataobjects/KLMChannelIndex.h>

/* ROOT headers. */
#include <TROOT.h>


using namespace Belle2;

REG_MODULE(DQMHistAnalysisKLM2);

DQMHistAnalysisKLM2Module::DQMHistAnalysisKLM2Module()
  : DQMHistAnalysisModule(),
    m_EklmElementNumbers{&(EKLMElementNumbers::Instance())}
{
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
  m_c_eff_bklm = new TCanvas("KLMEfficiencyDQM/c_eff_bklm_plane");
  m_c_eff_eklm = new TCanvas("KLMEfficiencyDQM/c_eff_eklm_plane");
  m_c_eff_bklm_sector = new TCanvas("KLMEfficiencyDQM/c_eff_bklm_sector");
  m_c_eff_eklm_sector = new TCanvas("KLMEfficiencyDQM/c_eff_eklm_sector");

  m_eff_bklm = new TH1F("KLMEfficiencyDQM/eff_bklm_plane",
                        "Plane Efficiency in BKLM",
                        BKLMElementNumbers::getMaximalLayerGlobalNumber(), 0.5, 0.5 + BKLMElementNumbers::getMaximalLayerGlobalNumber());
  m_eff_bklm->GetXaxis()->SetTitle("Layer number");
  m_eff_bklm->SetStats(false);
  m_eff_bklm->SetOption("HIST");

  m_eff_eklm = new TH1F("KLMEfficiencyDQM/eff_eklm_plane",
                        "Plane Efficiency in EKLM",
                        EKLMElementNumbers::getMaximalPlaneGlobalNumber(), 0.5, 0.5 + EKLMElementNumbers::getMaximalPlaneGlobalNumber());
  m_eff_eklm->GetXaxis()->SetTitle("Plane number");
  m_eff_eklm->SetStats(false);
  m_eff_eklm->SetOption("HIST");

  m_eff_bklm_sector = new TH1F("KLMEfficiencyDQM/eff_bklm_sector",
                               "Sector Efficiency in BKLM",
                               BKLMElementNumbers::getMaximalSectorGlobalNumber(), 0.5, BKLMElementNumbers::getMaximalSectorGlobalNumber() + 0.5);
  m_eff_bklm_sector->GetXaxis()->SetTitle("Sector number");
  m_eff_bklm_sector->SetStats(false);
  m_eff_bklm_sector->SetOption("HIST");

  m_eff_eklm_sector = new TH1F("KLMEfficiencyDQM/eff_eklm_sector",
                               "Sector Efficiency in EKLM",
                               EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder(), 0.5, EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder() + 0.5);
  m_eff_eklm_sector->GetXaxis()->SetTitle("Sector number");
  m_eff_eklm_sector->SetStats(false);
  m_eff_eklm_sector->SetOption("HIST");

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



void DQMHistAnalysisKLM2Module::processPlaneHistogram(
  const std::string& histName, TH1* histogram)
{
  std::string name;
  TCanvas* canvas = findCanvas("KLMEfficiencyDQM/c_" + histName);
  if (canvas == NULL) {
    B2ERROR("KLMDQM2 histogram canvas KLMEfficiencyDQM/c_" << histName << " is not found.");
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

  /* Reset efficiency histograms*/
  m_eff_bklm->Reset();
  m_eff_eklm->Reset();
  m_eff_bklm_sector->Reset();
  m_eff_eklm_sector->Reset();

  /* Obtain plots necessary for efficiency plots */
  TH1F* all_ext_bklm = (TH1F*)findHist("KLMEfficiencyDQM/all_ext_hitsBKLM");
  TH1F* matched_hits_bklm = (TH1F*)findHist("KLMEfficiencyDQM/matched_hitsBKLM");

  TH1F* all_ext_eklm = (TH1F*)findHist("KLMEfficiencyDQM/all_ext_hitsEKLM");
  TH1F* matched_hits_eklm = (TH1F*)findHist("KLMEfficiencyDQM/matched_hitsEKLM");


  TH1F* all_ext_bklm_sector = (TH1F*)findHist("KLMEfficiencyDQM/all_ext_hitsBKLMSector");
  TH1F* matched_hits_bklm_sector = (TH1F*)findHist("KLMEfficiencyDQM/matched_hitsBKLMSector");

  TH1F* all_ext_eklm_sector = (TH1F*)findHist("KLMEfficiencyDQM/all_ext_hitsEKLMSector");
  TH1F* matched_hits_eklm_sector = (TH1F*)findHist("KLMEfficiencyDQM/matched_hitsEKLMSector");

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

  /* Check if efficiency histograms exist*/
  m_eff_bklm->Divide(matched_hits_bklm,
                     all_ext_bklm, 1, 1, "B");
  m_eff_eklm->Divide(matched_hits_eklm,
                     all_ext_eklm, 1, 1, "B");

  m_eff_bklm_sector->Divide(matched_hits_bklm_sector,
                            all_ext_bklm_sector, 1, 1, "B");
  m_eff_eklm_sector->Divide(matched_hits_eklm_sector,
                            all_ext_eklm_sector, 1, 1, "B");

  /* Draw histograms onto canvases*/
  m_c_eff_bklm->cd();
  m_eff_bklm->Draw();
  m_c_eff_bklm->Modified();

  m_c_eff_eklm->cd();
  m_eff_eklm->Draw();
  m_c_eff_eklm->Modified();

  m_c_eff_bklm_sector->cd();
  m_eff_bklm_sector->Draw();
  m_c_eff_bklm_sector->Modified();

  m_c_eff_eklm_sector->cd();
  m_eff_eklm_sector->Draw();
  m_c_eff_eklm_sector->Modified();

  processPlaneHistogram("eff_bklm_plane", m_eff_bklm);
  processPlaneHistogram("eff_eklm_plane", m_eff_eklm);

}


