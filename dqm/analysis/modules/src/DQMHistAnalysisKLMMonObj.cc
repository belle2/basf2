/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cstddef>
#include <dqm/analysis/modules/DQMHistAnalysisKLMMonObj.h>
#include <TH2.h>
#include <iostream>
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisKLMMonObj);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisKLMMonObjModule::DQMHistAnalysisKLMMonObjModule()
  : DQMHistAnalysisModule(),
    m_IsNullRun{false},
    m_BklmElementNumbers{nullptr},
    m_bklmGeoPar{nullptr}
{
  setDescription("Module to add Monitoring Variables for KLM on Mirabelle");

  //Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir",
           std::string("DetectorOccupancies_before_filter"));
  addParam("lookbackWindow", m_lookbackWindow, "Lookback window in seconds for hit rate calculation (default: 10.4 microseconds)",
           10.4e-6);
}

DQMHistAnalysisKLMMonObjModule::~DQMHistAnalysisKLMMonObjModule()
{
  delete m_BklmElementNumbers;
}

void DQMHistAnalysisKLMMonObjModule::initialize()
{

  m_klmMonObj = getMonitoringObject("klm");

  // Initialize singleton instances
  m_BklmElementNumbers = new BKLMElementNumbers();
  m_bklmGeoPar = bklm::GeometryPar::instance();

}

void DQMHistAnalysisKLMMonObjModule::beginRun()
{
  // Get run flags for PVs
  m_IsPhysicsRun = (getRunType() == "physics");
  m_IsNullRun = (getRunType() == "null");
}

void DQMHistAnalysisKLMMonObjModule::event()
{
  B2DEBUG(20, "DQMHistAnalysisKLMMonObj: event called.");
}

void DQMHistAnalysisKLMMonObjModule::CalculateKLMHitRate(TH2* hist, int layer, Double_t totalEvents,
                                                         Double_t layerArea, Double_t& hitRate, Double_t& hitRateErr)
{
  if (!hist || hist->GetDimension() != 2 || layer <= 0 || layerArea <= 0 || totalEvents <= 0) {
    B2ERROR("CalculateKLMHitRate: Invalid input parameters.");
    hitRate = 0.0;
    hitRateErr = 0.0;
    return;
  }

  // Sum entries for the specific layer across all time bins (Y-axis)
  // Layer number is on X-axis, so we sum over all Y bins for a given X bin (layer)
  Double_t numDigits = 0.0;

  int nBinsX = hist->GetNbinsX();
  int nBinsY = hist->GetNbinsY();

  // Layer corresponds to X-axis bins
  if (layer > 0 && layer <= nBinsX) {
    for (int binY = 1; binY <= nBinsY; binY++) {
      Double_t binContent = hist->GetBinContent(layer, binY);
      numDigits += binContent;
    }
  } else {
    B2ERROR("CalculateKLMHitRate: Layer number out of range.");
    hitRate = 0.0;
    hitRateErr = 0.0;
    return;
  }

  // Calculate hit rate in Hz/cm²: numDigits / (numEvents(delayed Physics == 1) * lookbackWindow * layerArea)
  Double_t denominator = totalEvents * m_lookbackWindow * layerArea;

  if (denominator > 0 && numDigits > 0) {
    hitRate = numDigits / denominator; // Hit rate in Hz/cm²

    // Calculate relative error:
    Double_t relErr = sqrt(1.0 / numDigits + 1.0 / totalEvents);

    // Absolute error: hitRate * relErr
    hitRateErr = hitRate * relErr;
  } else {
    hitRate = 0.0;
    hitRateErr = 0.0;
  }
}

void DQMHistAnalysisKLMMonObjModule::endRun()
{
  // Calculate hit rates for each layer from 2D histograms
  // BKLM layer types:
  //   - Layers 1-2: Scintillator
  //   - Layers 3-15: RPC

  std::string histPrefix = "/bklm_plane_trg_occupancy";
  TH2F* bklm_trg[2];
  for (size_t i = 0; i < 2; i++) {
    bklm_trg[i] = (TH2F*)findHist(m_histogramDirectoryName + histPrefix + "_" + m_tag[i]);
  }

  auto* background_trigger_count = findHist("KLM/event_background_trigger_summary");
  if (not background_trigger_count) {
    B2ERROR("Cannot find histogram: KLM/event_background_trigger_summary");
    return;
  }

  Double_t totalEventsTrg = background_trigger_count->GetBinContent(1); // TTYP_DPHY == 1
  std::string prefix = "KLM_";
  std::string suffix = "hitRate";

  for (size_t i = 0; i < 2; i++) {
    if (not bklm_trg[i]) {
      B2ERROR("Cannot find histogram: " << m_histogramDirectoryName + histPrefix + "_" + m_tag[i]);
      continue;
    }
    for (int layerGlobal = 0; layerGlobal < 240; layerGlobal++) {
      int section, sector, layer;
      m_BklmElementNumbers->layerGlobalNumberToElementNumbers(
        layerGlobal, &section, &sector, &layer);
      Double_t layerAreaTrg = m_bklmGeoPar->getBKLMLayerArea(section, sector, layer); // in cm^2
      Double_t hitRate, hitRateErr;
      CalculateKLMHitRate(bklm_trg[i], layer, totalEventsTrg, layerAreaTrg, hitRate, hitRateErr);
      // Naming consistent with KLM2: B/F for section, 0-based sector, 1-based layer
      std::string varName = prefix + "B";
      varName += (section == 0) ? "B" : "F";
      varName += std::to_string((sector - 1) % BKLMElementNumbers::getMaximalSectorNumber());
      varName += "_layer" + std::to_string(layer) + "_trg_" + suffix + "_" + m_tag[i];
      m_klmMonObj->setVariable(varName, hitRate, hitRateErr);
    }
  }
}

