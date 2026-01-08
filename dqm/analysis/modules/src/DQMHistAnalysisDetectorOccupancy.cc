/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisDetectorOccupancy.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisDetectorOccupancy);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisDetectorOccupancyModule::DQMHistAnalysisDetectorOccupancyModule()
  : DQMHistAnalysisModule()
{
  setDescription("Module to manipulate histograms in DetectorOccupancy");

  //Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir",
           std::string("DetectorOccupancies_before_filter"));
}

DQMHistAnalysisDetectorOccupancyModule::~DQMHistAnalysisDetectorOccupancyModule() { }

void DQMHistAnalysisDetectorOccupancyModule::initialize()
{

  m_klmMonObj = getMonitoringObject("klm");

}

void DQMHistAnalysisDetectorOccupancyModule::beginRun()
{
  // Get run flags for PVs
  m_IsPhysicsRun = (getRunType() == "physics");
  m_IsNullRun = (getRunType() == "null");
}

void DQMHistAnalysisDetectorOccupancyModule::event()
{
  B2DEBUG(20, "DQMHistAnalysisEcmsMonObj: event called.");
}

void DQMHistAnalysisDetectorOccupancyModule::KLMOccupancyProcessing(TH1* totalHist, TH1* trgHist,
    Double_t& backRatio, Double_t& backErrRatio,
    Double_t& forwardRatio, Double_t& forwardErrRatio)
{
  int nBins = totalHist->GetNbinsX();

  if (nBins != trgHist->GetNbinsX())
  {B2ERROR("In KLMOccupancyProcessing, inputted totalHist and trgHist have a range mismatch.");}


  // Interested in occupancy in forward/backward region, so we need halfway point
  int halfwayPoint = 120; // half way value for BKLM

  // check halfway index
  std::string histName = totalHist->GetName();
  if (histName.find("eklm") != std::string::npos) {
    halfwayPoint = 96; // 12*8 for backward EKLM
  }

  //backward calculation
  HistToRatio(totalHist,  trgHist, 1, halfwayPoint, backRatio, backErrRatio);

  //forward calculation
  HistToRatio(totalHist,  trgHist, halfwayPoint + 1, nBins, forwardRatio, forwardErrRatio);

}

void DQMHistAnalysisDetectorOccupancyModule::HistToRatio(TH1* totalHist,  TH1* trgHist, int bin1, int bin2,
                                                         Double_t& ratio, Double_t& ratioErr)
{
  Double_t totalErr; Double_t trgErr;
  Double_t totalArea = totalHist->IntegralAndError(bin1, bin2, totalErr);
  Double_t trgArea   = trgHist->IntegralAndError(bin1, bin2, trgErr);

  ratio = trgArea / totalArea;
  ratioErr = ratio * sqrt((trgErr / trgArea) * (trgErr / trgArea) + (totalErr / totalArea) * (totalErr / totalArea));


}

void DQMHistAnalysisDetectorOccupancyModule::endRun()
{
  // for MiraBelle, calculate ratios and errors and send to
  /* Obtain plots necessary for occupancy ratios */
  std::string histPrefix = "/bklm_plane_occupancy";
  TH1F* bklm_total[2];
  for (size_t i = 0; i < 2; i++) {
    bklm_total[i] = (TH1F*)findHist(m_histogramDirectoryName + histPrefix + "_" + m_tag[i]);
  }

  histPrefix = "/bklm_plane_trg_occupancy";
  TH1F* bklm_trg[2];
  for (size_t i = 0; i < 2; i++) {
    bklm_trg[i] = (TH1F*)findHist(m_histogramDirectoryName + histPrefix + "_" + m_tag[i]);
  }

  Double_t bklmBackRatio[2]; Double_t bklmBackRatioErr[2];
  Double_t bklmForwardRatio[2]; Double_t bklmForwardRatioErr[2];

  histPrefix = "/eklm_plane_occupancy";
  TH1F* eklm_total[2];
  for (size_t i = 0; i < 2; i++) {
    eklm_total[i] = (TH1F*)findHist(m_histogramDirectoryName + histPrefix + "_" + m_tag[i]);
  }

  histPrefix = "/eklm_plane_trg_occupancy";
  TH1F* eklm_trg[2];
  for (size_t i = 0; i < 2; i++) {
    eklm_trg[i] = (TH1F*)findHist(m_histogramDirectoryName + histPrefix + "_" + m_tag[i]);
  }

  Double_t eklmBackRatio[2]; Double_t eklmBackRatioErr[2];
  Double_t eklmForwardRatio[2]; Double_t eklmForwardRatioErr[2];
  std::string prefix = "klm_";
  std::string suffix = "occupancy_ratio";
  for (size_t i = 0; i < 2; i++) {
    // for bklm
    KLMOccupancyProcessing(bklm_total[i], bklm_trg[i], bklmBackRatio[i], bklmBackRatioErr[i], bklmForwardRatio[i],
                           bklmForwardRatioErr[i]);
    m_klmMonObj->setVariable(prefix + "bb_" + suffix + m_tag[i], bklmBackRatio[i], bklmBackRatioErr[i]);
    m_klmMonObj->setVariable(prefix + "bf_" + suffix + m_tag[i], bklmForwardRatio[i],  bklmForwardRatioErr[i]);

    // for eklm
    KLMOccupancyProcessing(eklm_total[i], eklm_trg[i], eklmBackRatio[i], eklmBackRatioErr[i], eklmForwardRatio[i],
                           eklmForwardRatioErr[i]);
    m_klmMonObj->setVariable(prefix + "eb_" + suffix + m_tag[i], eklmBackRatio[i], eklmBackRatioErr[i]);
    m_klmMonObj->setVariable(prefix + "ef_" + suffix + m_tag[i], eklmForwardRatio[i], eklmForwardRatioErr[i]);
  }



}



