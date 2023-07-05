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
#include <TStyle.h>
#include <TColor.h>

using namespace Belle2;

REG_MODULE(DQMHistAnalysisKLM2);

DQMHistAnalysisKLM2Module::DQMHistAnalysisKLM2Module()
  : DQMHistAnalysisModule(),
    m_EklmElementNumbers{&(EKLMElementNumbers::Instance())}
{
  setDescription("Module used to analyze KLM Efficiency DQM histograms.");
  addParam("HistogramDirectoryName", m_histogramDirectoryName, "Name of histogram directory", std::string("KLMEfficiencyDQM"));
  addParam("MinEvents", m_minEvents, "Minimum events for delta histogram update", 5000000.);
  addParam("RefHistoFile", m_refFileName, "Reference histogram file name", std::string("KLM_DQM_REF_BEAM.root"));
  addParam("AlarmThreshold", m_alarmThr, "Set alarm threshold", float(0));
  addParam("Min2DEff", m_min, "2D efficiency min", float(0.5));
  addParam("Max2DEff", m_max, "2D efficiency max", float(2));
  addParam("RatioPlot", m_ratio, "2D efficiency ratio or difference plot ", bool(true));

  m_PlaneLine.SetLineColor(kMagenta);
  m_PlaneLine.SetLineWidth(1);
  m_PlaneLine.SetLineStyle(2); // dashed
  m_PlaneText.SetTextAlign(22); // centered, middle
  m_PlaneText.SetTextColor(kMagenta);
  m_PlaneText.SetTextFont(42); // Helvetica regular
  m_PlaneText.SetTextSize(0.02); // 2% of TPad's full height
}



void DQMHistAnalysisKLM2Module::initialize()
{
  m_monObj = getMonitoringObject("klm");

  //register EPICS PVs
  registerEpicsPV("KLMEff:nEffBKLMLayers", "nEffBKLMLayers");
  registerEpicsPV("KLMEff:nEffEKLMLayers", "nEffEKLMLayers");
  updateEpicsPVs(5.0);

  if (m_refFileName != "") {
    m_refFile = TFile::Open(m_refFileName.data(), "READ");
  }

  // Get Reference Histograms
  if (m_refFile && m_refFile->IsOpen()) {
    B2INFO("DQMHistAnalysisKLM2: reference root file (" << m_refFileName << ") FOUND, able to read ref histograms");

    m_ref_efficiencies_bklm = (TH1F*)m_refFile->Get((m_histogramDirectoryName + "/eff_bklm_plane").data());
    m_ref_efficiencies_bklm->SetLineColor(2);
    m_ref_efficiencies_bklm->SetOption("HIST");
    m_ref_efficiencies_bklm->SetStats(false);

    m_ref_efficiencies_eklm = (TH1F*)m_refFile->Get((m_histogramDirectoryName + "/eff_eklm_plane").data());
    m_ref_efficiencies_eklm->SetLineColor(2);
    m_ref_efficiencies_eklm->SetOption("HIST");
    m_ref_efficiencies_eklm->SetStats(false);

  } else {
    B2WARNING("KLM DQMHistAnalysis: reference root file (" << m_refFileName << ") not found, or closed");
  }
  gROOT->cd();
  m_c_eff_bklm = new TCanvas((m_histogramDirectoryName + "/c_eff_bklm_plane").data());
  m_c_eff_eklm = new TCanvas((m_histogramDirectoryName + "/c_eff_eklm_plane").data());
  m_c_eff_bklm_sector = new TCanvas((m_histogramDirectoryName + "/c_eff_bklm_sector").data());
  m_c_eff_eklm_sector = new TCanvas((m_histogramDirectoryName + "/c_eff_eklm_sector").data());

  m_c_eff2d_bklm = new TCanvas((m_histogramDirectoryName + "/c_eff2d_bklm").data());
  m_c_eff2d_eklm = new TCanvas((m_histogramDirectoryName + "/c_eff2d_eklm").data());

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

  // 2D Efficiency Histograms
  TString eff2d_hist_bklm_title;
  TString eff2d_hist_eklm_title;
  if (m_ratio) {
    eff2d_hist_bklm_title = "Plane Efficiency Ratios in BKLM";
    eff2d_hist_eklm_title = "Plane Efficiency Ratios in EKLM";
  } else {
    eff2d_hist_bklm_title = "Plane Efficiency Diffs in BKLM";
    eff2d_hist_eklm_title = "Plane Efficiency Diffs in EKLM";
  }

  // BKLM
  m_eff2d_bklm = new TH2F((m_histogramDirectoryName + "/eff2d_bklm_sector").data(), eff2d_hist_bklm_title,
                          BKLMElementNumbers::getMaximalSectorGlobalNumber(), 0.5, BKLMElementNumbers::getMaximalSectorGlobalNumber() + 0.5,
                          BKLMElementNumbers::getMaximalLayerNumber(),  0.5, BKLMElementNumbers::getMaximalLayerNumber() + 0.5);
  m_eff2d_bklm->GetXaxis()->SetTitle("Sector");
  m_eff2d_bklm->GetYaxis()->SetTitle("Layer");
  m_eff2d_bklm->SetStats(false);

  m_err_bklm = new TH2F((m_histogramDirectoryName + "/err_bklm_sector").data(), eff2d_hist_bklm_title,
                        BKLMElementNumbers::getMaximalSectorGlobalNumber(), 0.5, BKLMElementNumbers::getMaximalSectorGlobalNumber() + 0.5,
                        BKLMElementNumbers::getMaximalLayerNumber(),  0.5, BKLMElementNumbers::getMaximalLayerNumber() + 0.5);
  m_err_bklm->GetXaxis()->SetTitle("Sector");
  m_err_bklm->GetYaxis()->SetTitle("Layer");
  m_err_bklm->SetStats(false);

  for (int sec_id = 0; sec_id < BKLMElementNumbers::getMaximalSectorNumber(); sec_id++) {
    std::string BB_sec = "BB" + std::to_string(sec_id);
    m_eff2d_bklm->GetXaxis()->SetBinLabel(sec_id + 1, BB_sec.c_str());

    std::string BF_sec = "BF" + std::to_string(sec_id);
    m_eff2d_bklm->GetXaxis()->SetBinLabel(BKLMElementNumbers::getMaximalSectorNumber() + sec_id + 1, BF_sec.c_str());
  }

  for (int lay_id = 0; lay_id < BKLMElementNumbers::getMaximalLayerNumber(); lay_id++) {
    std::string B_lay = std::to_string(lay_id);
    m_eff2d_bklm->GetYaxis()->SetBinLabel(lay_id + 1, B_lay.c_str());
  }

  // EKLM
  int n_sectors_eklm = EKLMElementNumbers::getMaximalPlaneGlobalNumber() / EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder();

  m_eff2d_eklm = new TH2F((m_histogramDirectoryName + "/eff2d_eklm_sector").data(), eff2d_hist_eklm_title,
                          n_sectors_eklm, 0.5, n_sectors_eklm + 0.5,
                          EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder(),  0.5, EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder() + 0.5);
  m_eff2d_eklm->GetXaxis()->SetTitle("Sector");
  m_eff2d_eklm->GetYaxis()->SetTitle("Layer");
  m_eff2d_eklm->SetStats(false);

  m_err_eklm = new TH2F((m_histogramDirectoryName + "/err_bklm_sector").data(), eff2d_hist_eklm_title,
                        n_sectors_eklm, 0.5, n_sectors_eklm + 0.5,
                        EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder(),  0.5, EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder() + 0.5);
  m_err_eklm->GetXaxis()->SetTitle("Sector");
  m_err_eklm->GetYaxis()->SetTitle("Layer");
  m_err_eklm->SetStats(false);

  std::string name;
  const double maximalLayer = EKLMElementNumbers::getMaximalLayerGlobalNumber();
  for (int layerGlobal = 1; layerGlobal <= maximalLayer; ++layerGlobal) {
    int section, layer;
    m_EklmElementNumbers->layerNumberToElementNumbers(
      layerGlobal, &section, &layer);
    if (section == EKLMElementNumbers::c_BackwardSection)
      name = "B";
    else
      name = "F";
    name += std::to_string(layer);
    m_eff2d_eklm->GetXaxis()->SetBinLabel(layerGlobal, name.c_str());
  }

  for (int lay_id = 0; lay_id < EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder(); lay_id++) {
    std::string E_lay = std::to_string(lay_id);
    m_eff2d_eklm->GetYaxis()->SetBinLabel(lay_id + 1, E_lay.c_str());
  }


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
    UpdateCanvas(canvas->GetName(), (deltaNumer != nullptr && deltaDenom != nullptr));
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
    B2WARNING("KLMDQM2 histogram canvas " + m_histogramDirectoryName + "/c_" << histName << " is not found.");
    return;
  } else {
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
}

void DQMHistAnalysisKLM2Module::process2DEffHistogram(
  TH1* mainHist, TH1* refHist, TH2* eff2dHist, TH2* errHist, int layers, int sectors, bool ratioPlot, int* pvcount,
  TCanvas* eff2dCanv)
{

  int i = 0;
  float mainEff = 0;
  float refEff = 0;
  float mainErr = 0;
  float refErr = 0;
  float maxVal = m_max;
  float minVal = m_min;
  float eff2dVal;
  bool setAlarm = false;
  *pvcount = 0; //initialize to zero

  for (int binx = 0; binx < sectors; binx++) {

    for (int biny = 0; biny < layers; biny++) {

      mainEff = mainHist->GetBinContent(i + 1);
      mainErr = mainHist->GetBinError(i + 1);
      if (refHist) {
        refEff = refHist->GetBinContent(i + 1);
        refErr = refHist->GetBinError(i + 1);
      } else {
        refEff = 0.;
        refErr = 0.;
      }

      if ((mainEff == 0) and (refEff == 0)) {
        // empty histograms, draw blank bin
        eff2dHist->SetBinContent(binx + 1, biny + 1, 0);
      } else if (refEff == 0) {
        // no reference, set maximum value
        eff2dHist->SetBinContent(binx + 1, biny + 1, maxVal);
      } else if (mainEff == 0) {
        // no data, set zero
        eff2dHist->SetBinContent(binx + 1, biny + 1, 0);
        errHist->SetBinContent(binx + 1, biny + 1, 0);
      } else {

        if (ratioPlot) {
          eff2dVal = mainEff / refEff;
          if (eff2dVal < m_alarmThr) {errHist->SetBinContent(binx + 1, biny + 1, eff2dVal);}
        } else {
          eff2dVal = (mainEff - refEff) / pow(pow(mainErr, 2) + pow(refErr, 2), 0.5);
        }

        // main histogram
        if ((eff2dVal > minVal) and (eff2dVal < maxVal)) {
          // value within display window
          eff2dHist->SetBinContent(binx + 1, biny + 1, eff2dVal);
        } else if (eff2dVal > maxVal) {
          // value above display window
          eff2dHist->SetBinContent(binx + 1, biny + 1, maxVal);
        } else if (eff2dVal < minVal) {
          // value below display window
          eff2dHist->SetBinContent(binx + 1, biny + 1, minVal);
        }

        // set alarm
        if (eff2dVal < m_alarmThr) {
          *pvcount += 1;
          setAlarm = true;
        }

      }

      i++;
    }//end of layer loop

  }//end of sector loop

  eff2dHist->SetMinimum(m_min);
  eff2dHist->SetMaximum(m_max);

  eff2dCanv->cd();
  eff2dHist->Draw("COLZ");
  errHist->Draw("TEXT SAME");
  if (setAlarm) {
    eff2dCanv->Pad()->SetFillColor(kRed);
  }
  eff2dCanv->Modified();
  eff2dCanv->Update();
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

  /* Make Diff 2D plots */
  process2DEffHistogram(m_eff_bklm, m_ref_efficiencies_bklm, m_eff2d_bklm, m_err_bklm,
                        BKLMElementNumbers::getMaximalLayerNumber(), BKLMElementNumbers::getMaximalSectorGlobalNumber(),
                        m_ratio, &m_nEffBKLMLayers, m_c_eff2d_bklm);

  process2DEffHistogram(m_eff_eklm, m_ref_efficiencies_eklm, m_eff2d_eklm, m_err_eklm,
                        EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder(),
                        EKLMElementNumbers::getMaximalPlaneGlobalNumber() / EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder(),
                        m_ratio, &m_nEffEKLMLayers, m_c_eff2d_eklm);
  /* Set EPICS PV Values*/
  B2DEBUG(20, "Updating EPICS PVs in DQMHistAnalysisKLM2");
  setEpicsPV("nEffBKLMLayers", m_nEffBKLMLayers);
  setEpicsPV("nEffEKLMLayers", m_nEffEKLMLayers);
  updateEpicsPVs(5.0);
}


