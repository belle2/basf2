/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * SVD Data Quality Monitor creates monitoring variables and pack them to *
 * the Monitoring Objects, which are used as input to the Mirabelle       *
 * on the online system to show run by run dynamic of the variables.      *
 * This module provides higher level analyses on collected SVD DQM        *
 *  histograms:                                                           *
 * - SVDExpReco/SVDDQM_StripCountsU, SVDExpReco/SVDDQM_StripCountsV       *
 * - SVDExpReco/SVDDQM_nEvents                                            *
 * - SVDEfficiency/TrackHitsU, SVDEfficiency/MatchedHitsU                 *
 * - SVDEfficiency/TrackHitsV, SVDEfficiency/MatchedHitsV                 *
 * - SVDClsTrk/SVDTRK_ClusterChargeU3, SVDClsTrk/SVDTRK_ClusterChargeV3   *
 * - SVDClsTrk/SVDTRK_ClusterChargeU456                                   *
 * - SVDClsTrk/SVDTRK_ClusterChargeV456                                   *
 * - SVDClsTrk/SVDTRK_ClusterSNRU3, SVDClsTrk/SVDTRK_ClusterSNRV3         *
 * - SVDClsTrk/SVDTRK_ClusterSNRU456, SVDClsTrk/SVDTRK_ClusterSNRV456     *
 * - SVDClsTrk/SVDTRK_ClusterTimeU3, SVDClsTrk/SVDTRK_ClusterTimeV3       *
 * - SVDClsTrk/SVDTRK_ClusterSNRU456, SVDClsTrk/SVDTRK_ClusterSNRV456     *
 * - SVDClsTrk/SVDTRK_StripMaxBinUAll, SVDClsTrk/SVDTRK_StripMaxBinVAll   *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Karol Adamczyk (Karol.Adamczyk@ifj.edu.pl)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisSVDOnMiraBelle.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisSVDOnMiraBelle)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisSVDOnMiraBelleModule::DQMHistAnalysisSVDOnMiraBelleModule()
  : DQMHistAnalysisModule()
{
  setDescription("Extract monitoring variables from SVD DQM histograms");
  setPropertyFlags(c_ParallelProcessingCertified);
  B2DEBUG(20, "DQMHistAnalysisSVDOnMiraBelle: Constructor done.");
}

DQMHistAnalysisSVDOnMiraBelleModule::~DQMHistAnalysisSVDOnMiraBelleModule() { }

void DQMHistAnalysisSVDOnMiraBelleModule::initialize()
{
  gROOT->cd();

  // add MonitoringObject
  m_monObj = getMonitoringObject("svd");

  // list of canvases to be added to MonitoringObject
  m_c_avgEfficiency = new TCanvas("svd_avgEfficiency", "matched clusters and found tracks", 0, 0, 800, 600);
  m_c_avgOffOccupancy = new TCanvas("svd_avgOffOccupancy", "strips", 0, 0, 800, 600);
  m_c_MPVChargeClusterOnTrack = new TCanvas("svd_MPVChargeClusterOnTrack", "charge from Clusters on Track Charge", 0, 0, 400, 400);
  m_c_MPVSNRClusterOnTrack = new TCanvas("svd_MPVSNRClusterOnTrack", "SNR from Clusters on Track Charge", 0, 0, 400, 400);
  m_c_MPVTimeClusterOnTrack = new TCanvas("svd_MPVTimeClusterOnTrack", "time from Clusters on Track Charge", 0, 0, 400, 400);
  m_c_avgMaxBinClusterOnTrack = new TCanvas("svd_avgMaxBin", "average MaxBin", 0, 0, 800, 600);

  // add canvases used to create monitoring variables to MonitoringObject
  m_monObj->addCanvas(m_c_avgEfficiency);
  m_monObj->addCanvas(m_c_avgOffOccupancy);
  m_monObj->addCanvas(m_c_MPVChargeClusterOnTrack);
  m_monObj->addCanvas(m_c_MPVSNRClusterOnTrack);
  m_monObj->addCanvas(m_c_MPVTimeClusterOnTrack);
  m_monObj->addCanvas(m_c_avgMaxBinClusterOnTrack);

  B2DEBUG(20, "DQMHistAnalysisSVDOnMiraBelle: initialized.");
}

void DQMHistAnalysisSVDOnMiraBelleModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisSVDOnMiraBelle: beginRun called.");
}

void DQMHistAnalysisSVDOnMiraBelleModule::event()
{
  B2DEBUG(20, "DQMHistAnalysisSVDOnMiraBelle: event called.");
}

void DQMHistAnalysisSVDOnMiraBelleModule::endRun()
{
  // offline occupancy - integrated number of ZS5 fired strips
  TH1F* h_zs5countsU = (TH1F*)findHist("SVDExpReco/SVDDQM_StripCountsU"); // made by SVDDQMExperssRecoModule
  TH1F* h_zs5countsV = (TH1F*)findHist("SVDExpReco/SVDDQM_StripCountsV");
  TH1F* h_events = (TH1F*)findHist("SVDExpReco/SVDDQM_nEvents");

  // adding histograms to canvas
  m_c_avgOffOccupancy->Clear();
  m_c_avgOffOccupancy->Divide(2, 2);
  m_c_avgOffOccupancy->cd(1);
  if (h_zs5countsU) h_zs5countsU->Draw("colz");
  m_c_avgOffOccupancy->cd(2);
  if (h_zs5countsV) h_zs5countsU->Draw("colz");
  m_c_avgOffOccupancy->cd(3);
  if (h_events) h_events->Draw("colz");

  int nE = h_events->GetEntries(); // number of events for all "clusters on track" histograms

  // average occupancy for each layer
  std::vector<float> avgOffOccL3UV = avgOccupancyUV(3, h_zs5countsU, h_zs5countsV, 0, 14, 1, 1, nE);

  std::vector<float> avgOffOccL4UV = avgOccupancyUV(4, h_zs5countsU, h_zs5countsV, 0, 30, 15, 1, nE);

  std::vector<float> avgOffOccL5UV = avgOccupancyUV(5, h_zs5countsU, h_zs5countsV, 0, 48, 45, 1, nE);

  std::vector<float> avgOffOccL6UV = avgOccupancyUV(6, h_zs5countsU, h_zs5countsV, 0, 80, 93, 1, nE);

  // average occupancy for middle plane sensors
  std::vector<float> avgOffOccL4X2UV = avgOccupancyUV(4, h_zs5countsU, h_zs5countsV, 0, 10, 16, 3, nE); // L4.X.2

  std::vector<float> avgOffOccL5X2UV = avgOccupancyUV(5, h_zs5countsU, h_zs5countsV, 0, 12, 36, 4, nE); // L5.X.2

  std::vector<float> avgOffOccL5X3UV = avgOccupancyUV(5, h_zs5countsU, h_zs5countsV, 0, 12, 37, 4, nE); // L5.X.3

  std::vector<float> avgOffOccL6X2UV = avgOccupancyUV(6, h_zs5countsU, h_zs5countsV, 0, 16, 94, 5, nE); // L6.X.2

  std::vector<float> avgOffOccL6X3UV = avgOccupancyUV(6, h_zs5countsU, h_zs5countsV, 0, 16, 95, 5, nE); // L6.X.3

  std::vector<float> avgOffOccL6X4UV = avgOccupancyUV(6, h_zs5countsU, h_zs5countsV, 0, 16, 96, 5, nE); // L6.X.4

  // average occupancy for high occupancy sensors
  std::vector<float> avgOffOccL311UV = highOccupancySensor(3, h_zs5countsU, h_zs5countsV, 1, nE); // L3.1.1

  std::vector<float> avgOffOccL312UV = highOccupancySensor(3, h_zs5countsU, h_zs5countsV, 2, nE); // L3.1.2

  std::vector<float> avgOffOccL321UV = highOccupancySensor(3, h_zs5countsU, h_zs5countsV, 3, nE); // L3.2.1

  std::vector<float> avgOffOccL322UV = highOccupancySensor(3, h_zs5countsU, h_zs5countsV, 4, nE); // L3.2.2

  std::vector<float> avgOffOccL461UV = highOccupancySensor(4, h_zs5countsU, h_zs5countsV, 30, nE); // L4.6.1

  std::vector<float> avgOffOccL462UV = highOccupancySensor(4, h_zs5countsU, h_zs5countsV, 31, nE); // L4.6.2

  std::vector<float> avgOffOccL581UV = highOccupancySensor(5, h_zs5countsU, h_zs5countsV, 73, nE); // L5.8.1

  std::vector<float> avgOffOccL582UV = highOccupancySensor(5, h_zs5countsU, h_zs5countsV, 74, nE); // L5.8.2

  std::vector<float> avgOffOccL6101UV = highOccupancySensor(6, h_zs5countsU, h_zs5countsV, 138, nE); // L6.10.1

  std::vector<float> avgOffOccL6102UV = highOccupancySensor(6, h_zs5countsU, h_zs5countsV, 139, nE); // L6.10.2

  // setting monitoring variables
  if (h_zs5countsU == NULL || h_events == NULL) {
    B2INFO("Histograms needed for Average Offline Occupancy on U side are not found");
    m_monObj->setVariable("avgOffOccL3U", -1);
    m_monObj->setVariable("avgOffOccL4U", -1);
    m_monObj->setVariable("avgOffOccL5U", -1);
    m_monObj->setVariable("avgOffOccL6U", -1);
    m_monObj->setVariable("avgOffOccL4X2U", -1);
    m_monObj->setVariable("avgOffOccL5X2U", -1);
    m_monObj->setVariable("avgOffOccL5X3U", -1);
    m_monObj->setVariable("avgOffOccL6X2U", -1);
    m_monObj->setVariable("avgOffOccL6X3U", -1);
    m_monObj->setVariable("avgOffOccL6X4U", -1);
    m_monObj->setVariable("avgOffOccL311U", -1);
    m_monObj->setVariable("avgOffOccL312U", -1);
    m_monObj->setVariable("avgOffOccL321U", -1);
    m_monObj->setVariable("avgOffOccL322U", -1);
    m_monObj->setVariable("avgOffOccL461U", -1);
    m_monObj->setVariable("avgOffOccL462U", -1);
    m_monObj->setVariable("avgOffOccL581U", -1);
    m_monObj->setVariable("avgOffOccL582U", -1);
    m_monObj->setVariable("avgOffOccL6101U", -1);
    m_monObj->setVariable("avgOffOccL6102U", -1);
  } else {
    m_monObj->setVariable("avgOffOccL3U", avgOffOccL3UV[0]);
    m_monObj->setVariable("avgOffOccL4U", avgOffOccL4UV[0]);
    m_monObj->setVariable("avgOffOccL5U", avgOffOccL5UV[0]);
    m_monObj->setVariable("avgOffOccL6U", avgOffOccL6UV[0]);
    m_monObj->setVariable("avgOffOccL4X2U", avgOffOccL4X2UV[0]);
    m_monObj->setVariable("avgOffOccL5X2U", avgOffOccL5X2UV[0]);
    m_monObj->setVariable("avgOffOccL5X3U", avgOffOccL5X3UV[0]);
    m_monObj->setVariable("avgOffOccL6X2U", avgOffOccL6X2UV[0]);
    m_monObj->setVariable("avgOffOccL6X3U", avgOffOccL6X3UV[0]);
    m_monObj->setVariable("avgOffOccL6X4U", avgOffOccL6X4UV[0]);
    m_monObj->setVariable("avgOffOccL311U", avgOffOccL311UV[0]);
    m_monObj->setVariable("avgOffOccL312U", avgOffOccL312UV[0]);
    m_monObj->setVariable("avgOffOccL321U", avgOffOccL321UV[0]);
    m_monObj->setVariable("avgOffOccL322U", avgOffOccL322UV[0]);
    m_monObj->setVariable("avgOffOccL461U", avgOffOccL461UV[0]);
    m_monObj->setVariable("avgOffOccL462U", avgOffOccL462UV[0]);
    m_monObj->setVariable("avgOffOccL581U", avgOffOccL581UV[0]);
    m_monObj->setVariable("avgOffOccL582U", avgOffOccL582UV[0]);
    m_monObj->setVariable("avgOffOccL6101U", avgOffOccL6101UV[0]);
    m_monObj->setVariable("avgOffOccL6102U", avgOffOccL6102UV[0]);
  }

  if (h_zs5countsV == NULL || h_events == NULL) {
    B2INFO("Histograms needed for Average Offline Occupancy on V side are not found");
    m_monObj->setVariable("avgOffOccL3V", -1);
    m_monObj->setVariable("avgOffOccL4V", -1);
    m_monObj->setVariable("avgOffOccL5V", -1);
    m_monObj->setVariable("avgOffOccL6V", -1);
    m_monObj->setVariable("avgOffOccL4X2V", -1);
    m_monObj->setVariable("avgOffOccL5X2V", -1);
    m_monObj->setVariable("avgOffOccL5X3V", -1);
    m_monObj->setVariable("avgOffOccL6X2V", -1);
    m_monObj->setVariable("avgOffOccL6X3V", -1);
    m_monObj->setVariable("avgOffOccL6X4V", -1);
    m_monObj->setVariable("avgOffOccL311V", -1);
    m_monObj->setVariable("avgOffOccL312V", -1);
    m_monObj->setVariable("avgOffOccL321V", -1);
    m_monObj->setVariable("avgOffOccL322V", -1);
    m_monObj->setVariable("avgOffOccL461V", -1);
    m_monObj->setVariable("avgOffOccL462V", -1);
    m_monObj->setVariable("avgOffOccL581V", -1);
    m_monObj->setVariable("avgOffOccL582V", -1);
    m_monObj->setVariable("avgOffOccL6101V", -1);
    m_monObj->setVariable("avgOffOccL6102V", -1);
  } else {
    m_monObj->setVariable("avgOffOccL3V", avgOffOccL3UV[1]);
    m_monObj->setVariable("avgOffOccL4V", avgOffOccL4UV[1]);
    m_monObj->setVariable("avgOffOccL5V", avgOffOccL5UV[1]);
    m_monObj->setVariable("avgOffOccL6V", avgOffOccL6UV[1]);
    m_monObj->setVariable("avgOffOccL4X2V", avgOffOccL4X2UV[1]);
    m_monObj->setVariable("avgOffOccL5X2V", avgOffOccL5X2UV[1]);
    m_monObj->setVariable("avgOffOccL5X3V", avgOffOccL5X3UV[1]);
    m_monObj->setVariable("avgOffOccL6X2V", avgOffOccL6X2UV[1]);
    m_monObj->setVariable("avgOffOccL6X3V", avgOffOccL6X3UV[1]);
    m_monObj->setVariable("avgOffOccL6X4V", avgOffOccL6X4UV[1]);
    m_monObj->setVariable("avgOffOccL311V", avgOffOccL311UV[1]);
    m_monObj->setVariable("avgOffOccL312V", avgOffOccL312UV[1]);
    m_monObj->setVariable("avgOffOccL321V", avgOffOccL321UV[1]);
    m_monObj->setVariable("avgOffOccL322V", avgOffOccL322UV[1]);
    m_monObj->setVariable("avgOffOccL461V", avgOffOccL461UV[1]);
    m_monObj->setVariable("avgOffOccL462V", avgOffOccL462UV[1]);
    m_monObj->setVariable("avgOffOccL581V", avgOffOccL581UV[1]);
    m_monObj->setVariable("avgOffOccL582V", avgOffOccL582UV[1]);
    m_monObj->setVariable("avgOffOccL6101V", avgOffOccL6101UV[1]);
    m_monObj->setVariable("avgOffOccL6102V", avgOffOccL6102UV[1]);
  }


  // efficiency of cluster recontruction for U and V side
  TH2F* h_found_tracksU = (TH2F*)findHist("SVDEfficiency/TrackHitsU");
  TH2F* h_matched_clusU = (TH2F*)findHist("SVDEfficiency/MatchedHitsU");
  TH2F* h_found_tracksV = (TH2F*)findHist("SVDEfficiency/TrackHitsV");
  TH2F* h_matched_clusV = (TH2F*)findHist("SVDEfficiency/MatchedHitsV");

  m_c_avgEfficiency->Clear();
  m_c_avgEfficiency->Divide(2, 2);
  m_c_avgEfficiency->cd(1);
  if (h_found_tracksU) h_found_tracksU->Draw("colz");
  m_c_avgEfficiency->cd(2);
  if (h_found_tracksV) h_found_tracksV->Draw("colz");
  m_c_avgEfficiency->cd(3);
  if (h_matched_clusU) h_matched_clusU->Draw("colz");
  m_c_avgEfficiency->cd(4);
  if (h_matched_clusV) h_matched_clusV->Draw("colz");

  // average efficiency in each layer for both side (U, V)
  std::vector<float> avgEffL3 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 7, 2, 3);

  std::vector<float> avgEffL4 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 10, 5, 7);

  std::vector<float> avgEffL5 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 12, 9, 12);

  std::vector<float> avgEffL6 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 16, 14, 18);

  // average efficiency
  std::vector<float> avgEffL3456 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 16, 2, 18);

  // average efficiency for mid plane +x L3.1.1, L3.1.2
  std::vector<float> avgEffL31X = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 1, 2, 3);

  // average efficiency for mid plane +x L3.2.1, L3.2.2
  std::vector<float> avgEffL32X = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 2, 2, 2, 3);

  // average efficiency for mid plane: L4.X.2
  std::vector<float> avgEffL4X2 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 10, 6, 6);

  // average efficiency for mid plane: L5.X.2
  std::vector<float> avgEffL5X2 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 12, 10, 10);

  // average efficiency for mid plane: L5.X.3
  std::vector<float> avgEffL5X3 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 12, 11, 11);

  // average efficiency for mid plane: L6.X.2
  std::vector<float> avgEffL6X2 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 16, 15, 15);

  // average efficiency for mid plane: L6.X.3
  std::vector<float> avgEffL6X3 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 16, 16, 16);

  // average efficiency for mid plane: L6.X.4
  std::vector<float> avgEffL6X4 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 16, 17, 17);

  // average efficiency for high occupancy sensors

  std::vector<float> avgEffL311UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 1, 2,
                                                    2); // L3.1.1

  std::vector<float> avgEffL312UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 1, 3,
                                                    3); // L3.1.2

  std::vector<float> avgEffL321UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 2, 2, 2,
                                                    2); // L3.2.1

  std::vector<float> avgEffL322UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 2, 2, 3,
                                                    3); // L3.2.2

  std::vector<float> avgEffL461UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 6, 6, 5,
                                                    5); // L4.6.1

  std::vector<float> avgEffL462UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 6, 6, 6,
                                                    6); // L4.6.2

  std::vector<float> avgEffL581UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 8, 8, 9,
                                                    9); // L5.8.1

  std::vector<float> avgEffL582UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 8, 8, 10,
                                                    10); // L5.8.2

  std::vector<float> avgEffL6101UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 10, 10, 14,
                                                     14); // L6.10.1

  std::vector<float> avgEffL6102UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 10, 10, 15,
                                                     15); // L6.10.2

  // setting monitoring variables
  if (h_matched_clusU == NULL || h_found_tracksU == NULL) {
    B2INFO("Histograms needed for Average Efficiency on U side are not found");
    m_monObj->setVariable("avgEffL3U", -1);
    m_monObj->setVariable("avgEffL4U", -1);
    m_monObj->setVariable("avgEffL5U", -1);
    m_monObj->setVariable("avgEffL6U", -1);
    m_monObj->setVariable("avgEffL3456U", -1);
    m_monObj->setVariable("avgEffL31XU", -1);
    m_monObj->setVariable("avgEffL32XU", -1);
    m_monObj->setVariable("avgEffL4X2U", -1);
    m_monObj->setVariable("avgEffL5X2U", -1);
    m_monObj->setVariable("avgEffL5X3U", -1);
    m_monObj->setVariable("avgEffL6X2U", -1);
    m_monObj->setVariable("avgEffL6X3U", -1);
    m_monObj->setVariable("avgEffL6X4U", -1);
    m_monObj->setVariable("avgEffL311U", -1);
    m_monObj->setVariable("avgEffL312U", -1);
    m_monObj->setVariable("avgEffL321U", -1);
    m_monObj->setVariable("avgEffL322U", -1);
    m_monObj->setVariable("avgEffL461U", -1);
    m_monObj->setVariable("avgEffL462U", -1);
    m_monObj->setVariable("avgEffL581U", -1);
    m_monObj->setVariable("avgEffL582U", -1);
    m_monObj->setVariable("avgEffL6101U", -1);
    m_monObj->setVariable("avgEffL6102U", -1);
  } else {
    m_monObj->setVariable("avgEffL3U", avgEffL3[0]);
    m_monObj->setVariable("avgEffL4U", avgEffL4[0]);
    m_monObj->setVariable("avgEffL5U", avgEffL5[0]);
    m_monObj->setVariable("avgEffL6U", avgEffL6[0]);
    m_monObj->setVariable("avgEffL3456U", avgEffL3456[0]);
    m_monObj->setVariable("avgEffL31XU", avgEffL31X[0]);
    m_monObj->setVariable("avgEffL32XU", avgEffL32X[0]);
    m_monObj->setVariable("avgEffL4X2U", avgEffL4X2[0]);
    m_monObj->setVariable("avgEffL5X2U", avgEffL5X2[0]);
    m_monObj->setVariable("avgEffL5X3U", avgEffL5X3[0]);
    m_monObj->setVariable("avgEffL6X2U", avgEffL6X2[0]);
    m_monObj->setVariable("avgEffL6X3U", avgEffL6X3[0]);
    m_monObj->setVariable("avgEffL6X4U", avgEffL6X4[0]);
    m_monObj->setVariable("avgEffL311U", avgEffL311UV[0]);
    m_monObj->setVariable("avgEffL312U", avgEffL312UV[0]);
    m_monObj->setVariable("avgEffL321U", avgEffL321UV[0]);
    m_monObj->setVariable("avgEffL322U", avgEffL322UV[0]);
    m_monObj->setVariable("avgEffL461U", avgEffL461UV[0]);
    m_monObj->setVariable("avgEffL462U", avgEffL462UV[0]);
    m_monObj->setVariable("avgEffL581U", avgEffL581UV[0]);
    m_monObj->setVariable("avgEffL582U", avgEffL582UV[0]);
    m_monObj->setVariable("avgEffL6101U", avgEffL6101UV[0]);
    m_monObj->setVariable("avgEffL6102U", avgEffL6102UV[0]);
  }

  if (h_matched_clusV == NULL || h_found_tracksV == NULL) {
    B2INFO("Histograms needed for Average Efficiency on V side are not found");
    m_monObj->setVariable("avgEffL3V", -1);
    m_monObj->setVariable("avgEffL4V", -1);
    m_monObj->setVariable("avgEffL5V", -1);
    m_monObj->setVariable("avgEffL6V", -1);
    m_monObj->setVariable("avgEffL3456V", -1);
    m_monObj->setVariable("avgEffL31XV", -1);
    m_monObj->setVariable("avgEffL32XV", -1);
    m_monObj->setVariable("avgEffL4X2V", -1);
    m_monObj->setVariable("avgEffL5X2V", -1);
    m_monObj->setVariable("avgEffL5X3V", -1);
    m_monObj->setVariable("avgEffL6X2V", -1);
    m_monObj->setVariable("avgEffL6X3V", -1);
    m_monObj->setVariable("avgEffL6X4V", -1);
    m_monObj->setVariable("avgEffL311V", -1);
    m_monObj->setVariable("avgEffL312V", -1);
    m_monObj->setVariable("avgEffL321V", -1);
    m_monObj->setVariable("avgEffL322V", -1);
    m_monObj->setVariable("avgEffL461V", -1);
    m_monObj->setVariable("avgEffL462V", -1);
    m_monObj->setVariable("avgEffL581V", -1);
    m_monObj->setVariable("avgEffL582V", -1);
    m_monObj->setVariable("avgEffL6101V", -1);
    m_monObj->setVariable("avgEffL6102V", -1);
  } else {
    m_monObj->setVariable("avgEffL3V", avgEffL3[1]);
    m_monObj->setVariable("avgEffL4V", avgEffL4[1]);
    m_monObj->setVariable("avgEffL5V", avgEffL5[1]);
    m_monObj->setVariable("avgEffL6V", avgEffL6[1]);
    m_monObj->setVariable("avgEffL3456V", avgEffL3456[1]);
    m_monObj->setVariable("avgEffL31XV", avgEffL31X[1]);
    m_monObj->setVariable("avgEffL32XV", avgEffL32X[1]);
    m_monObj->setVariable("avgEffL4X2V", avgEffL4X2[1]);
    m_monObj->setVariable("avgEffL5X2V", avgEffL5X2[1]);
    m_monObj->setVariable("avgEffL5X3V", avgEffL5X3[1]);
    m_monObj->setVariable("avgEffL6X2V", avgEffL6X2[1]);
    m_monObj->setVariable("avgEffL6X3V", avgEffL6X3[1]);
    m_monObj->setVariable("avgEffL6X4V", avgEffL6X4[1]);
    m_monObj->setVariable("avgEffL311V", avgEffL311UV[1]);
    m_monObj->setVariable("avgEffL312V", avgEffL312UV[1]);
    m_monObj->setVariable("avgEffL321V", avgEffL321UV[1]);
    m_monObj->setVariable("avgEffL322V", avgEffL322UV[1]);
    m_monObj->setVariable("avgEffL461V", avgEffL461UV[1]);
    m_monObj->setVariable("avgEffL462V", avgEffL462UV[1]);
    m_monObj->setVariable("avgEffL581V", avgEffL581UV[1]);
    m_monObj->setVariable("avgEffL582V", avgEffL582UV[1]);
    m_monObj->setVariable("avgEffL6101V", avgEffL6101UV[1]);
    m_monObj->setVariable("avgEffL6102V", avgEffL6102UV[1]);
  }


  // MPV cluster charge for clusters on track
  TH1F* h_clusterCharge_L3U = (TH1F*)findHist("SVDClsTrk/SVDTRK_ClusterChargeU3");
  TH1F* h_clusterCharge_L3V = (TH1F*)findHist("SVDClsTrk/SVDTRK_ClusterChargeV3");
  TH1F* h_clusterCharge_L456U = (TH1F*)findHist("SVDClsTrk/SVDTRK_ClusterChargeU456");
  TH1F* h_clusterCharge_L456V = (TH1F*)findHist("SVDClsTrk/SVDTRK_ClusterChargeV456");

  m_c_MPVChargeClusterOnTrack->Clear();
  m_c_MPVChargeClusterOnTrack->Divide(2, 2);
  m_c_MPVChargeClusterOnTrack->cd(1);
  if (h_clusterCharge_L3U) h_clusterCharge_L3U->Draw();
  m_c_MPVChargeClusterOnTrack->cd(2);
  if (h_clusterCharge_L3V) h_clusterCharge_L3V->Draw();
  m_c_MPVChargeClusterOnTrack->cd(3);
  if (h_clusterCharge_L456U) h_clusterCharge_L456U->Draw();
  m_c_MPVChargeClusterOnTrack->cd(4);
  if (h_clusterCharge_L456U) h_clusterCharge_L456U->Draw();

  // find abscissa of max Y in histograms
  float MPVClusterChargeL3U = xForMaxY(h_clusterCharge_L3U);
  float MPVClusterChargeL3V = xForMaxY(h_clusterCharge_L3V);
  float MPVClusterChargeL456U = xForMaxY(h_clusterCharge_L456U);
  float MPVClusterChargeL456V = xForMaxY(h_clusterCharge_L456V);

  if (h_clusterCharge_L3U == NULL || h_clusterCharge_L456U == NULL) {
    B2INFO("Histograms needed for MPV cluster charge on U side are not found");
    m_monObj->setVariable("MPVClusterChargeL3U", -1);
    m_monObj->setVariable("MPVClusterChargeL456U", -1);
  } else {
    m_monObj->setVariable("MPVClusterChargeL3U", MPVClusterChargeL3U);
    m_monObj->setVariable("MPVClusterChargeL456U", MPVClusterChargeL456U);
  }

  if (h_clusterCharge_L3V == NULL || h_clusterCharge_L456V == NULL) {
    B2INFO("Histograms needed for MPV cluster charge on V side are not found");
    m_monObj->setVariable("MPVClusterChargeL3V", -1);
    m_monObj->setVariable("MPVClusterChargeL456V", -1);
  } else {
    m_monObj->setVariable("MPVClusterChargeL3V", MPVClusterChargeL3V);
    m_monObj->setVariable("MPVClusterChargeL456V", MPVClusterChargeL456V);
  }


  // MPV SNR for the clusters on track
  TH1F* h_clusterSNR_L3U = (TH1F*)findHist("SVDClsTrk/SVDTRK_ClusterSNRU3");
  TH1F* h_clusterSNR_L3V = (TH1F*)findHist("SVDClsTrk/SVDTRK_ClusterSNRV3");
  TH1F* h_clusterSNR_L456U = (TH1F*)findHist("SVDClsTrk/SVDTRK_ClusterSNRU456");
  TH1F* h_clusterSNR_L456V = (TH1F*)findHist("SVDClsTrk/SVDTRK_ClusterSNRV456");

  m_c_MPVSNRClusterOnTrack->Clear();
  m_c_MPVSNRClusterOnTrack->Divide(2, 2);
  m_c_MPVSNRClusterOnTrack->cd(1);
  if (h_clusterSNR_L3U) h_clusterSNR_L3U->Draw();
  m_c_MPVSNRClusterOnTrack->cd(2);
  if (h_clusterSNR_L3V) h_clusterSNR_L3V->Draw();
  m_c_MPVSNRClusterOnTrack->cd(3);
  if (h_clusterSNR_L456U) h_clusterSNR_L456U->Draw();
  m_c_MPVSNRClusterOnTrack->cd(4);
  if (h_clusterSNR_L456V) h_clusterSNR_L456V->Draw();

  float MPVClusterSNRL3U = xForMaxY(h_clusterSNR_L3U);
  float MPVClusterSNRL3V = xForMaxY(h_clusterSNR_L3V);
  float MPVClusterSNRL456U = xForMaxY(h_clusterSNR_L456U);
  float MPVClusterSNRL456V = xForMaxY(h_clusterSNR_L456V);

  if (h_clusterSNR_L3U == NULL || h_clusterSNR_L456U == NULL) {
    B2INFO("Histograms needed for MPV cluster SNR on U side are not found");
    m_monObj->setVariable("MPVClusterSNRL3U", -1);
    m_monObj->setVariable("MPVClusterSNRL456U", -1);
  } else {
    m_monObj->setVariable("MPVClusterSNRL3U", MPVClusterSNRL3U);
    m_monObj->setVariable("MPVClusterSNRL456U", MPVClusterSNRL456U);
  }

  if (h_clusterSNR_L3V == NULL || h_clusterSNR_L456V == NULL) {
    B2INFO("Histograms needed for MPV cluster SNR on V side are not found");
    m_monObj->setVariable("MPVClusterSNRL3V", -1);
    m_monObj->setVariable("MPVClusterSNRL456V", -1);
  } else {
    m_monObj->setVariable("MPVClusterSNRL3V", MPVClusterSNRL3V);
    m_monObj->setVariable("MPVClusterSNRL456V", MPVClusterSNRL456V);
  }


  //  MPV SVD cluster time for the clusters on track
  TH1F* h_clusterTime_L3U = (TH1F*)findHist("SVDClsTrk/SVDTRK_ClusterTimeU3");
  TH1F* h_clusterTime_L3V = (TH1F*)findHist("SVDClsTrk/SVDTRK_ClusterTimeV3");
  TH1F* h_clusterTime_L456U = (TH1F*)findHist("SVDClsTrk/SVDTRK_ClusterTimeU456");
  TH1F* h_clusterTime_L456V = (TH1F*)findHist("SVDClsTrk/SVDTRK_ClusterTimeV456");

  m_c_MPVTimeClusterOnTrack->Clear();
  m_c_MPVTimeClusterOnTrack->Divide(2, 2);
  m_c_MPVTimeClusterOnTrack->cd(1);
  if (h_clusterTime_L3U) h_clusterTime_L3U->Draw();
  m_c_MPVTimeClusterOnTrack->cd(2);
  if (h_clusterTime_L3V) h_clusterTime_L3V->Draw();
  m_c_MPVTimeClusterOnTrack->cd(3);
  if (h_clusterTime_L456U) h_clusterTime_L456U->Draw();
  m_c_MPVTimeClusterOnTrack->cd(4);
  if (h_clusterTime_L456V) h_clusterTime_L456V->Draw();

  float MPVClusterTimeL3U = xForMaxY(h_clusterTime_L3U);
  float MPVClusterTimeL3V = xForMaxY(h_clusterTime_L3V);
  float MPVClusterTimeL456U = xForMaxY(h_clusterTime_L456U);
  float MPVClusterTimeL456V = xForMaxY(h_clusterTime_L456V);
  float FWHMClusterTimeL3U = histFWHM(h_clusterTime_L3U);
  float FWHMClusterTimeL3V = histFWHM(h_clusterTime_L3V);
  float FWHMClusterTimeL456U = histFWHM(h_clusterTime_L456U);
  float FWHMClusterTimeL456V = histFWHM(h_clusterTime_L456V);

  if (h_clusterTime_L3U == NULL || h_clusterTime_L456U == NULL) {
    B2INFO("Histograms needed for MPV cluster time on U side are not found");
    m_monObj->setVariable("MPVClusterTimeL3U", -1);
    m_monObj->setVariable("MPVClusterTimeL456U", -1);
    m_monObj->setVariable("FWHMClusterTimeL3U", -1);
    m_monObj->setVariable("FWHMClusterTimeL456U", -1);
  } else {
    m_monObj->setVariable("MPVClusterTimeL3U", MPVClusterTimeL3U);
    m_monObj->setVariable("MPVClusterTimeL456U", MPVClusterTimeL456U);
    m_monObj->setVariable("FWHMClusterTimeL3U", FWHMClusterTimeL3U);
    m_monObj->setVariable("FWHMClusterTimeL456U", FWHMClusterTimeL456U);
  }

  if (h_clusterTime_L3V == NULL || h_clusterTime_L456V == NULL) {
    B2INFO("Histograms needed for MPV cluster time on V side are not found");
    m_monObj->setVariable("MPVClusterTimeL3V", -1);
    m_monObj->setVariable("MPVClusterTimeL456V", -1);
    m_monObj->setVariable("FWHMClusterTimeL3V", -1);
    m_monObj->setVariable("FWHMClusterTimeL456V", -1);
  } else {
    m_monObj->setVariable("MPVClusterTimeL3V", MPVClusterTimeL3V);
    m_monObj->setVariable("MPVClusterTimeL456V", MPVClusterTimeL456V);
    m_monObj->setVariable("FWHMClusterTimeL3V", FWHMClusterTimeL3V);
    m_monObj->setVariable("FWHMClusterTimeL456V", FWHMClusterTimeL456V);
  }


  // average maxBin for clusters on track
  TH1F* h_maxBinU = (TH1F*)findHist("SVDClsTrk/SVDTRK_StripMaxBinUAll");
  TH1F* h_maxBinV = (TH1F*)findHist("SVDClsTrk/SVDTRK_StripMaxBinVAll");

  m_c_avgMaxBinClusterOnTrack->Clear();
  m_c_avgMaxBinClusterOnTrack->Divide(2, 1);
  m_c_avgMaxBinClusterOnTrack->cd(1);
  if (h_maxBinU) h_maxBinU->Draw();
  m_c_avgMaxBinClusterOnTrack->cd(2);
  if (h_maxBinV) h_maxBinV->Draw();

  if (h_maxBinU == NULL) {
    B2INFO("Histogram needed for Average MaxBin on U side is not found");
    m_monObj->setVariable("avgMaxBinU", -1);
  } else {
    float avgMaxBinU = h_maxBinU->GetMean();
    m_monObj->setVariable("avgMaxBinU", avgMaxBinU);
  }

  if (h_maxBinV == NULL) {
    B2INFO("Histogram needed for Average MaxBin on V side is not found");
    m_monObj->setVariable("avgMaxBinV", -1);
  } else {
    float avgMaxBinV = h_maxBinV->GetMean();
    m_monObj->setVariable("avgMaxBinV", avgMaxBinV);
  }

  B2INFO("DQMHistAnalysisSVDGeneral: endRun called");
}


void DQMHistAnalysisSVDOnMiraBelleModule::terminate()
{
  B2INFO("DQMHistAnalysisSVDOnMiraBelle: terminate called");
}


std::vector<float> DQMHistAnalysisSVDOnMiraBelleModule::highOccupancySensor(int iLayer, TH1F* hU, TH1F* hV, int iBin,
    int nEvents) const
{
  int nStripsV = -1;
  if (iLayer == 3) {
    nStripsV = 768;
  } else if (iLayer >= 4 && iLayer <= 6) {
    nStripsV = 512;
  } else {
    B2DEBUG(20, "Layer out of range [3,6].");
  }
  std::vector<float> avgOffOccUV(2, 0.0);
  avgOffOccUV[0] = hU->GetBinContent(iBin) / 768 / nEvents * 100;
  avgOffOccUV[1] = hV->GetBinContent(iBin) / nStripsV / nEvents * 100;
  return avgOffOccUV;
}


std::vector<float> DQMHistAnalysisSVDOnMiraBelleModule::avgOccupancyUV(int iLayer, TH1F* hU, TH1F* hV, int min, int max, int offset,
    int step, int nEvents) const
{
  int nStripsV = -1;
  if (iLayer == 3) {
    nStripsV = 768;
  } else if (iLayer >= 4 && iLayer <= 6) {
    nStripsV = 512;
  } else {
    B2DEBUG(20, "Layer out of range [3,6].");
  }
  std::vector<float> avgOffOccUV(2, 0.0);
  for (int bin = min; bin < max; bin++) {
    avgOffOccUV[0] += hU->GetBinContent(offset + step * bin) / 768 * 100;
    avgOffOccUV[1] += hV->GetBinContent(offset + step * bin) / nStripsV * 100;
  }
  avgOffOccUV[0] /= ((max - min) * nEvents);
  avgOffOccUV[1] /= ((max - min) * nEvents);
  return avgOffOccUV;
}


std::vector<float> DQMHistAnalysisSVDOnMiraBelleModule::avgEfficiencyUV(TH2F* hMCU, TH2F* hMCV, TH2F* hFTU, TH2F* hFTV, int minX,
    int maxX, int minY, int maxY) const
{
  std::vector<float> avgEffUV(2, 0.0);
  std::vector<float> sumMatchedClustersUV(2, 0.0);
  std::vector<float> sumFoundTracksUV(2, 0.0);
  for (int binX = minX; binX < maxX + 1; binX++) {
    for (int binY = minY; binY < maxY + 1; binY++) {
      int binXY = hMCV->GetBin(binX, binY);
      sumMatchedClustersUV[0] += hMCU->GetBinContent(binXY);
      sumMatchedClustersUV[1] += hMCV->GetBinContent(binXY);
      sumFoundTracksUV[0] += hFTU->GetBinContent(binXY);
      sumFoundTracksUV[1] += hFTV->GetBinContent(binXY);
    }
  }
  if (sumFoundTracksUV[0] > 0) {
    avgEffUV[0] = sumMatchedClustersUV[0] / sumFoundTracksUV[0] * 100;
  } else {
    avgEffUV[0] = -1;
  }
  if (sumFoundTracksUV[1] > 0) {
    avgEffUV[1] = sumMatchedClustersUV[1] / sumFoundTracksUV[1] * 100;
  } else {
    avgEffUV[1] = -1;
  }
  return avgEffUV;
}

float DQMHistAnalysisSVDOnMiraBelleModule::xForMaxY(TH1F* h) const
{
  int maxY = h->GetMaximumBin();
  float xMaxY = h->GetXaxis()->GetBinCenter(maxY);
  return xMaxY;
}

float DQMHistAnalysisSVDOnMiraBelleModule::histFWHM(TH1F* h) const
{
  int bin1 = h->FindFirstBinAbove(h->GetMaximum() / 2);
  int bin2 = h->FindLastBinAbove(h->GetMaximum() / 2);
  float fwhm = h->GetBinCenter(bin2) - h->GetBinCenter(bin1);
  return fwhm;
}
