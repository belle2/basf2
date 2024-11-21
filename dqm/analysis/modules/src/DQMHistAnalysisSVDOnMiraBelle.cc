/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <numeric>
#include <limits>
#include <vxd/geometry/GeoCache.h>
#include <dqm/analysis/modules/DQMHistAnalysisSVDOnMiraBelle.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisSVDOnMiraBelle);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisSVDOnMiraBelleModule::DQMHistAnalysisSVDOnMiraBelleModule()
  : DQMHistAnalysisModule()
{
  setDescription("DQM Analysis Module that extracts monitoring variables from SVD DQM histograms and provides input to MiraBelle.");
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
  m_c_MeanSVDEventT0 = new TCanvas("svd_MeanSVDEventT0", "Mean Event T0 from SVD for all samples", 0, 0, 400, 400);

  // add canvases used to create monitoring variables to MonitoringObject
  m_monObj->addCanvas(m_c_avgEfficiency);
  m_monObj->addCanvas(m_c_avgOffOccupancy);
  m_monObj->addCanvas(m_c_MPVChargeClusterOnTrack);
  m_monObj->addCanvas(m_c_MPVSNRClusterOnTrack);
  m_monObj->addCanvas(m_c_MPVTimeClusterOnTrack);
  m_monObj->addCanvas(m_c_avgMaxBinClusterOnTrack);
  m_monObj->addCanvas(m_c_MeanSVDEventT0);

  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  //collect the list of all SVD Modules in the geometry here
  std::vector<VxdID> sensors = geo.getListOfSensors();
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = geo.getSensorInfo(aVxdID);
    // B2INFO("VXD " << aVxdID);
    if (info.getType() != VXD::SensorInfoBase::SVD) continue;
    m_SVDModules.push_back(aVxdID); // reorder, sort would be better
  }
  std::sort(m_SVDModules.begin(), m_SVDModules.end());  // back to natural order

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
  float nan = numeric_limits<float>::quiet_NaN();

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
  if (h_zs5countsV) h_zs5countsV->Draw("colz");
  m_c_avgOffOccupancy->cd(3);
  if (h_events) h_events->Draw("colz");

  int nE = 0;
  if (h_events) nE = h_events->GetEntries();  // number of events for all "clusters on track" histograms

  // setting monitoring variables
  if (h_zs5countsU == NULL || h_zs5countsV == NULL || h_events == NULL) {
    if (h_zs5countsU == NULL) {
      B2INFO("Histograms needed for Average Offline Occupancy on U side are not found");
    }
    if (h_zs5countsV == NULL) {
      B2INFO("Histograms needed for Average Offline Occupancy on V side are not found");
    }
  } else {
    // average occupancy for each layer
    std::vector<float> avgOffOccL3UV = avgOccupancyUV(3, h_zs5countsU, h_zs5countsV, 0, 14, 1, 1, nE);

    std::vector<float> avgOffOccL4UV = avgOccupancyUV(4, h_zs5countsU, h_zs5countsV, 0, 30, 15, 1, nE);

    std::vector<float> avgOffOccL5UV = avgOccupancyUV(5, h_zs5countsU, h_zs5countsV, 0, 48, 45, 1, nE);

    std::vector<float> avgOffOccL6UV = avgOccupancyUV(6, h_zs5countsU, h_zs5countsV, 0, 80, 93, 1, nE);

    // average occupancy for each layer
    std::vector<float> avgOffGrpId0OccL3UV = avgOccupancyGrpId0UV(3, nE);

    std::vector<float> avgOffGrpId0OccL4UV = avgOccupancyGrpId0UV(4, nE);

    std::vector<float> avgOffGrpId0OccL5UV = avgOccupancyGrpId0UV(5, nE);

    std::vector<float> avgOffGrpId0OccL6UV = avgOccupancyGrpId0UV(6, nE);

    // occupancy averaged over ladders
    std::vector<float> avgOffOccL3X1UV = avgOccupancyUV(3, h_zs5countsU, h_zs5countsV, 0, 7, 0, 2, nE); // L3.X.1

    std::vector<float> avgOffOccL3X2UV = avgOccupancyUV(3, h_zs5countsU, h_zs5countsV, 0, 7, 1, 2, nE); // L3.X.2

    std::vector<float> avgOffOccL4X1UV = avgOccupancyUV(4, h_zs5countsU, h_zs5countsV, 0, 10, 15, 3, nE); // L4.X.1

    std::vector<float> avgOffOccL4X2UV = avgOccupancyUV(4, h_zs5countsU, h_zs5countsV, 0, 10, 16, 3, nE); // L4.X.2

    std::vector<float> avgOffOccL4X3UV = avgOccupancyUV(4, h_zs5countsU, h_zs5countsV, 0, 10, 17, 3, nE); // L4.X.3

    std::vector<float> avgOffOccL5X1UV = avgOccupancyUV(5, h_zs5countsU, h_zs5countsV, 0, 12, 35, 4, nE); // L5.X.1

    std::vector<float> avgOffOccL5X2UV = avgOccupancyUV(5, h_zs5countsU, h_zs5countsV, 0, 12, 36, 4, nE); // L5.X.2

    std::vector<float> avgOffOccL5X3UV = avgOccupancyUV(5, h_zs5countsU, h_zs5countsV, 0, 12, 37, 4, nE); // L5.X.3

    std::vector<float> avgOffOccL5X4UV = avgOccupancyUV(5, h_zs5countsU, h_zs5countsV, 0, 12, 38, 4, nE); // L5.X.4

    std::vector<float> avgOffOccL6X1UV = avgOccupancyUV(6, h_zs5countsU, h_zs5countsV, 0, 16, 93, 5, nE); // L6.X.1

    std::vector<float> avgOffOccL6X2UV = avgOccupancyUV(6, h_zs5countsU, h_zs5countsV, 0, 16, 94, 5, nE); // L6.X.2

    std::vector<float> avgOffOccL6X3UV = avgOccupancyUV(6, h_zs5countsU, h_zs5countsV, 0, 16, 95, 5, nE); // L6.X.3

    std::vector<float> avgOffOccL6X4UV = avgOccupancyUV(6, h_zs5countsU, h_zs5countsV, 0, 16, 96, 5, nE); // L6.X.4

    std::vector<float> avgOffOccL6X5UV = avgOccupancyUV(6, h_zs5countsU, h_zs5countsV, 0, 16, 97, 5, nE); // L6.X.5

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

    // average occupancy for low DCDC
    // L3.2.1 above
    // L3.2.2 above
    std::vector<float> avgOffOccL411UV = getOccupancySensorUV(4, h_zs5countsU, h_zs5countsV, 1, 5, nE); // L4.1.1

    std::vector<float> avgOffOccL4102UV = getOccupancySensorUV(4, h_zs5countsU, h_zs5countsV, 10, 6, nE); // L4.10.2

    // average occupancy for peculiar sensors
    std::vector<float> avgOffOccL433UV = getOccupancySensorUV(4, h_zs5countsU, h_zs5countsV, 3, 7, nE); // L4.3.3

    std::vector<float> avgOffOccL513UV = getOccupancySensorUV(5, h_zs5countsU, h_zs5countsV, 1, 11, nE); // L5.1.3

    std::vector<float> avgOffOccL514UV = getOccupancySensorUV(5, h_zs5countsU, h_zs5countsV, 3, 12, nE); // L5.1.4

    std::vector<float> avgOffOccL592UV = getOccupancySensorUV(5, h_zs5countsU, h_zs5countsV, 9, 10, nE); // L5.9.2

    std::vector<float> avgOffOccL594UV = getOccupancySensorUV(5, h_zs5countsU, h_zs5countsV, 9, 12, nE); // L5.9.4

    std::vector<float> avgOffOccL643UV = getOccupancySensorUV(6, h_zs5countsU, h_zs5countsV, 4, 16, nE); // L6.4.3

    std::vector<float> avgOffOccL664UV = getOccupancySensorUV(6, h_zs5countsU, h_zs5countsV, 6, 17, nE); // L6.6.4

    std::vector<float> avgOffOccL6103UV = getOccupancySensorUV(6, h_zs5countsU, h_zs5countsV, 10, 16, nE); // L6.10.3

    std::vector<float> avgOffOccL6115UV = getOccupancySensorUV(6, h_zs5countsU, h_zs5countsV, 11, 18, nE); // L6.11.5

    std::vector<float> avgOffOccL6124UV = getOccupancySensorUV(6, h_zs5countsU, h_zs5countsV, 12, 17, nE); // L6.12.4

    m_monObj->setVariable("avgOffOccL3U", avgOffOccL3UV[0]);
    m_monObj->setVariable("avgOffOccL4U", avgOffOccL4UV[0]);
    m_monObj->setVariable("avgOffOccL5U", avgOffOccL5UV[0]);
    m_monObj->setVariable("avgOffOccL6U", avgOffOccL6UV[0]);

    m_monObj->setVariable("avgOffGrpId0OccL3U", avgOffGrpId0OccL3UV[0]);
    m_monObj->setVariable("avgOffGrpId0OccL4U", avgOffGrpId0OccL4UV[0]);
    m_monObj->setVariable("avgOffGrpId0OccL5U", avgOffGrpId0OccL5UV[0]);
    m_monObj->setVariable("avgOffGrpId0OccL6U", avgOffGrpId0OccL6UV[0]);

    m_monObj->setVariable("avgOffOccL3X1U", avgOffOccL3X1UV[0]);
    m_monObj->setVariable("avgOffOccL3X2U", avgOffOccL3X2UV[0]);
    m_monObj->setVariable("avgOffOccL4X1U", avgOffOccL4X1UV[0]);
    m_monObj->setVariable("avgOffOccL4X2U", avgOffOccL4X2UV[0]);
    m_monObj->setVariable("avgOffOccL4X3U", avgOffOccL4X3UV[0]);
    m_monObj->setVariable("avgOffOccL5X1U", avgOffOccL5X1UV[0]);
    m_monObj->setVariable("avgOffOccL5X2U", avgOffOccL5X2UV[0]);
    m_monObj->setVariable("avgOffOccL5X3U", avgOffOccL5X3UV[0]);
    m_monObj->setVariable("avgOffOccL5X4U", avgOffOccL5X4UV[0]);
    m_monObj->setVariable("avgOffOccL6X1U", avgOffOccL6X1UV[0]);
    m_monObj->setVariable("avgOffOccL6X2U", avgOffOccL6X2UV[0]);
    m_monObj->setVariable("avgOffOccL6X3U", avgOffOccL6X3UV[0]);
    m_monObj->setVariable("avgOffOccL6X4U", avgOffOccL6X4UV[0]);
    m_monObj->setVariable("avgOffOccL6X5U", avgOffOccL6X5UV[0]);
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

    m_monObj->setVariable("avgOffOccL3V", avgOffOccL3UV[1]);
    m_monObj->setVariable("avgOffOccL4V", avgOffOccL4UV[1]);
    m_monObj->setVariable("avgOffOccL5V", avgOffOccL5UV[1]);
    m_monObj->setVariable("avgOffOccL6V", avgOffOccL6UV[1]);

    m_monObj->setVariable("avgOffGrpId0OccL3V", avgOffGrpId0OccL3UV[1]);
    m_monObj->setVariable("avgOffGrpId0OccL4V", avgOffGrpId0OccL4UV[1]);
    m_monObj->setVariable("avgOffGrpId0OccL5V", avgOffGrpId0OccL5UV[1]);
    m_monObj->setVariable("avgOffGrpId0OccL6V", avgOffGrpId0OccL6UV[1]);

    m_monObj->setVariable("avgOffOccL3X1V", avgOffOccL3X1UV[1]);
    m_monObj->setVariable("avgOffOccL3X2V", avgOffOccL3X2UV[1]);
    m_monObj->setVariable("avgOffOccL4X1V", avgOffOccL4X1UV[1]);
    m_monObj->setVariable("avgOffOccL4X2V", avgOffOccL4X2UV[1]);
    m_monObj->setVariable("avgOffOccL4X3V", avgOffOccL4X3UV[1]);
    m_monObj->setVariable("avgOffOccL5X1V", avgOffOccL5X1UV[1]);
    m_monObj->setVariable("avgOffOccL5X2V", avgOffOccL5X2UV[1]);
    m_monObj->setVariable("avgOffOccL5X3V", avgOffOccL5X3UV[1]);
    m_monObj->setVariable("avgOffOccL5X4V", avgOffOccL5X4UV[1]);
    m_monObj->setVariable("avgOffOccL6X1V", avgOffOccL6X1UV[1]);
    m_monObj->setVariable("avgOffOccL6X2V", avgOffOccL6X2UV[1]);
    m_monObj->setVariable("avgOffOccL6X3V", avgOffOccL6X3UV[1]);
    m_monObj->setVariable("avgOffOccL6X4V", avgOffOccL6X4UV[1]);
    m_monObj->setVariable("avgOffOccL6X5V", avgOffOccL6X5UV[1]);
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

    // DCDC
    m_monObj->setVariable("avgOffOccL411U", avgOffOccL411UV[0]);
    m_monObj->setVariable("avgOffOccL4102U", avgOffOccL4102UV[0]);
    m_monObj->setVariable("avgOffOccL411V", avgOffOccL411UV[1]);
    m_monObj->setVariable("avgOffOccL4102V", avgOffOccL4102UV[1]);

    //peculiar
    m_monObj->setVariable("avgOffOccL433V", avgOffOccL433UV[1]);
    m_monObj->setVariable("avgOffOccL513U", avgOffOccL513UV[0]);
    m_monObj->setVariable("avgOffOccL514V", avgOffOccL514UV[1]);
    m_monObj->setVariable("avgOffOccL592V", avgOffOccL592UV[1]);
    m_monObj->setVariable("avgOffOccL594V", avgOffOccL594UV[1]);
    m_monObj->setVariable("avgOffOccL643U", avgOffOccL643UV[0]);
    m_monObj->setVariable("avgOffOccL664U", avgOffOccL664UV[0]);
    m_monObj->setVariable("avgOffOccL6103U", avgOffOccL6103UV[0]);
    m_monObj->setVariable("avgOffOccL6115U", avgOffOccL6115UV[0]);
    m_monObj->setVariable("avgOffOccL6124U", avgOffOccL6124UV[0]);
  }


  // efficiency of cluster reconstruction for U and V side
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

  // setting monitoring variables
  if (h_matched_clusU == NULL || h_matched_clusV == NULL || h_found_tracksU == NULL) {
    if (h_matched_clusU == NULL) {
      B2INFO("Histograms needed for Average Efficiency on U side are not found");
    }
    if (h_matched_clusV == NULL) {
      B2INFO("Histograms needed for Average Efficiency on V side are not found");
    }
  } else {
    // average efficiency in each layer for both side (U, V)
    std::vector<float> avgEffL3 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 7, 2, 3);

    std::vector<float> avgEffL4 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 10, 5, 7);

    std::vector<float> avgEffL5 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 12, 9, 12);

    std::vector<float> avgEffL6 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 16, 14, 18);

    // average efficiency for all layers
    std::vector<float> avgEffL3456 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 16, 2, 18);

    // average efficiency for mid plane: L3.X.1
    std::vector<float> avgEffL3X1 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 7, 2, 2);

    // average efficiency for mid plane: L3.X.2
    std::vector<float> avgEffL3X2 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 7, 3, 3);

    // average efficiency for mid plane: L4.X.1
    std::vector<float> avgEffL4X1 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 10, 5, 5);

    // average efficiency for mid plane: L4.X.2
    std::vector<float> avgEffL4X2 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 10, 6, 6);

    // average efficiency for mid plane: L4.X.3
    std::vector<float> avgEffL4X3 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 10, 7, 7);

    // average efficiency for mid plane: L5.X.1
    std::vector<float> avgEffL5X1 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 12, 9, 9);

    // average efficiency for mid plane: L5.X.2
    std::vector<float> avgEffL5X2 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 12, 10, 10);

    // average efficiency for mid plane: L5.X.3
    std::vector<float> avgEffL5X3 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 12, 11, 11);

    // average efficiency for mid plane: L5.X.4
    std::vector<float> avgEffL5X4 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 12, 12, 12);

    // average efficiency for mid plane: L6.X.1
    std::vector<float> avgEffL6X1 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 16, 14, 14);

    // average efficiency for mid plane: L6.X.2
    std::vector<float> avgEffL6X2 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 16, 15, 15);

    // average efficiency for mid plane: L6.X.3
    std::vector<float> avgEffL6X3 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 16, 16, 16);

    // average efficiency for mid plane: L6.X.4
    std::vector<float> avgEffL6X4 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 16, 17, 17);

    // average efficiency for mid plane: L6.X.5
    std::vector<float> avgEffL6X5 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 16, 18, 18);

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

    // average efficiency for low DCDC
    // L3.2.1 above
    // L3.2.2 above
    std::vector<float> avgEffL411UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 1, 5,
                                                      5); // L4.1.1

    std::vector<float> avgEffL4102UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 10, 10, 6,
                                                       6); // L4.10.2

    // average efficiency for peculiar sensors
    std::vector<float> avgEffL433UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 3, 3, 7,
                                                      7); // L4.3.3

    std::vector<float> avgEffL513UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 1, 1, 11,
                                                      11); // L5.1.3

    std::vector<float> avgEffL514UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 3, 3, 12,
                                                      12); // L5.1.4

    std::vector<float> avgEffL592UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 9, 9, 10,
                                                      10); // L5.9.2

    std::vector<float> avgEffL594UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 9, 9, 12,
                                                      12); // L5.9.4

    std::vector<float> avgEffL643UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 4, 4, 16,
                                                      16); // L6.4.3

    std::vector<float> avgEffL664UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 6, 6, 17,
                                                      17); // L6.6.4

    std::vector<float> avgEffL6103UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 10, 10, 16,
                                                       16); // L6.10.3

    std::vector<float> avgEffL6115UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 11, 11, 18,
                                                       18); // L6.11.5

    std::vector<float> avgEffL6124UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 12, 12, 17,
                                                       17); // L6.12.4

    m_monObj->setVariable("avgEffL3U", avgEffL3[0]);
    m_monObj->setVariable("avgEffL4U", avgEffL4[0]);
    m_monObj->setVariable("avgEffL5U", avgEffL5[0]);
    m_monObj->setVariable("avgEffL6U", avgEffL6[0]);
    m_monObj->setVariable("avgEffL3456U", avgEffL3456[0]);
    m_monObj->setVariable("avgEffL3X1U", avgEffL3X1[0]);
    m_monObj->setVariable("avgEffL3X2U", avgEffL3X2[0]);
    m_monObj->setVariable("avgEffL4X1U", avgEffL4X1[0]);
    m_monObj->setVariable("avgEffL4X2U", avgEffL4X2[0]);
    m_monObj->setVariable("avgEffL4X3U", avgEffL4X3[0]);
    m_monObj->setVariable("avgEffL5X1U", avgEffL5X1[0]);
    m_monObj->setVariable("avgEffL5X2U", avgEffL5X2[0]);
    m_monObj->setVariable("avgEffL5X3U", avgEffL5X3[0]);
    m_monObj->setVariable("avgEffL5X4U", avgEffL5X4[0]);
    m_monObj->setVariable("avgEffL6X1U", avgEffL6X1[0]);
    m_monObj->setVariable("avgEffL6X2U", avgEffL6X2[0]);
    m_monObj->setVariable("avgEffL6X3U", avgEffL6X3[0]);
    m_monObj->setVariable("avgEffL6X4U", avgEffL6X4[0]);
    m_monObj->setVariable("avgEffL6X5U", avgEffL6X5[0]);
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

    m_monObj->setVariable("avgEffL3V", avgEffL3[1]);
    m_monObj->setVariable("avgEffL4V", avgEffL4[1]);
    m_monObj->setVariable("avgEffL5V", avgEffL5[1]);
    m_monObj->setVariable("avgEffL6V", avgEffL6[1]);
    m_monObj->setVariable("avgEffL3456V", avgEffL3456[1]);
    m_monObj->setVariable("avgEffL3X1V", avgEffL3X1[1]);
    m_monObj->setVariable("avgEffL3X2V", avgEffL3X2[1]);
    m_monObj->setVariable("avgEffL4X1V", avgEffL4X1[1]);
    m_monObj->setVariable("avgEffL4X2V", avgEffL4X2[1]);
    m_monObj->setVariable("avgEffL4X3V", avgEffL4X3[1]);
    m_monObj->setVariable("avgEffL5X1V", avgEffL5X1[1]);
    m_monObj->setVariable("avgEffL5X2V", avgEffL5X2[1]);
    m_monObj->setVariable("avgEffL5X3V", avgEffL5X3[1]);
    m_monObj->setVariable("avgEffL5X4V", avgEffL5X4[1]);
    m_monObj->setVariable("avgEffL6X1V", avgEffL6X1[1]);
    m_monObj->setVariable("avgEffL6X2V", avgEffL6X2[1]);
    m_monObj->setVariable("avgEffL6X3V", avgEffL6X3[1]);
    m_monObj->setVariable("avgEffL6X4V", avgEffL6X4[1]);
    m_monObj->setVariable("avgEffL6X5V", avgEffL6X5[1]);
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
    // DCDC
    m_monObj->setVariable("avgEffL411U", avgEffL411UV[0]);
    m_monObj->setVariable("avgEffL4102U", avgEffL4102UV[0]);
    m_monObj->setVariable("avgEffL411V", avgEffL411UV[1]);
    m_monObj->setVariable("avgEffL4102V", avgEffL4102UV[1]);

    //peculiar
    m_monObj->setVariable("avgEffL433V", avgEffL433UV[1]);
    m_monObj->setVariable("avgEffL513U", avgEffL513UV[0]);
    m_monObj->setVariable("avgEffL514V", avgEffL514UV[1]);
    m_monObj->setVariable("avgEffL592V", avgEffL592UV[1]);
    m_monObj->setVariable("avgEffL594V", avgEffL594UV[1]);
    m_monObj->setVariable("avgEffL643U", avgEffL643UV[0]);
    m_monObj->setVariable("avgEffL664U", avgEffL664UV[0]);
    m_monObj->setVariable("avgEffL6103U", avgEffL6103UV[0]);
    m_monObj->setVariable("avgEffL6115U", avgEffL6115UV[0]);
    m_monObj->setVariable("avgEffL6124U", avgEffL6124UV[0]);
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
  float MPVClusterChargeL3U = nan;
  if (h_clusterCharge_L3U)
    if (h_clusterCharge_L3U->GetEntries() != 0)
      MPVClusterChargeL3U = xForMaxY(h_clusterCharge_L3U);
  float MPVClusterChargeL3V = nan;
  if (h_clusterCharge_L3V)
    if (h_clusterCharge_L3V->GetEntries() != 0)
      MPVClusterChargeL3V = xForMaxY(h_clusterCharge_L3V);
  float MPVClusterChargeL456U = nan;
  if (h_clusterCharge_L456U)
    if (h_clusterCharge_L456U->GetEntries() != 0)
      MPVClusterChargeL456U = xForMaxY(h_clusterCharge_L456U);
  float MPVClusterChargeL456V = nan;
  if (h_clusterCharge_L456V)
    if (h_clusterCharge_L456V->GetEntries() != 0)
      MPVClusterChargeL456V = xForMaxY(h_clusterCharge_L456V);

  if (h_clusterCharge_L3U == NULL || h_clusterCharge_L456U == NULL) {
    B2INFO("Histograms needed for MPV cluster charge on U side are not found");
  } else {
    m_monObj->setVariable("MPVClusterChargeL3U", MPVClusterChargeL3U);
    m_monObj->setVariable("MPVClusterChargeL456U", MPVClusterChargeL456U);
  }

  if (h_clusterCharge_L3V == NULL || h_clusterCharge_L456V == NULL) {
    B2INFO("Histograms needed for MPV cluster charge on V side are not found");
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

  float MPVClusterSNRL3U = nan;
  if (h_clusterSNR_L3U)
    if (h_clusterSNR_L3U->GetEntries() != 0)
      MPVClusterSNRL3U = xForMaxY(h_clusterSNR_L3U);
  float MPVClusterSNRL3V = nan;
  if (h_clusterSNR_L3V)
    if (h_clusterSNR_L3V->GetEntries() != 0)
      MPVClusterSNRL3V = xForMaxY(h_clusterSNR_L3V);
  float MPVClusterSNRL456U = nan;
  if (h_clusterSNR_L456U)
    if (h_clusterSNR_L456U->GetEntries() != 0)
      MPVClusterSNRL456U = xForMaxY(h_clusterSNR_L456U);
  float MPVClusterSNRL456V = nan;
  if (h_clusterSNR_L456V)
    if (h_clusterSNR_L456V->GetEntries() != 0)
      MPVClusterSNRL456V = xForMaxY(h_clusterSNR_L456V);

  if (h_clusterSNR_L3U == NULL || h_clusterSNR_L456U == NULL) {
    B2INFO("Histograms needed for MPV cluster SNR on U side are not found");
  } else {
    m_monObj->setVariable("MPVClusterSNRL3U", MPVClusterSNRL3U);
    m_monObj->setVariable("MPVClusterSNRL456U", MPVClusterSNRL456U);
  }

  if (h_clusterSNR_L3V == NULL || h_clusterSNR_L456V == NULL) {
    B2INFO("Histograms needed for MPV cluster SNR on V side are not found");
  } else {
    m_monObj->setVariable("MPVClusterSNRL3V", MPVClusterSNRL3V);
    m_monObj->setVariable("MPVClusterSNRL456V", MPVClusterSNRL456V);
  }


  //  MPV SVD cluster time for the clusters on track
  TH1F* h_clusterTime_L3U = (TH1F*)findHist("SVDClsTrk/SVDTRK_ClusterTimeU3");
  TH1F* h_clusterTime_L3V = (TH1F*)findHist("SVDClsTrk/SVDTRK_ClusterTimeV3");
  TH1F* h_clusterTime_L456U = (TH1F*)findHist("SVDClsTrk/SVDTRK_ClusterTimeU456");
  TH1F* h_clusterTime_L456V = (TH1F*)findHist("SVDClsTrk/SVDTRK_ClusterTimeV456");
  TH1F* h_MeanSVD3EventT0   = (TH1F*)findHist("SVDHitTime/SVD3EventT0");
  TH1F* h_MeanSVD6EventT0   = (TH1F*)findHist("SVDHitTime/SVD6EventT0");
  TH1F* h_MeanSVDEventT0    = 0x0;

  if (h_MeanSVD3EventT0)
    h_MeanSVDEventT0 = (TH1F*)h_MeanSVD3EventT0->Clone();

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

  m_c_MeanSVDEventT0->Clear();
  m_c_MeanSVDEventT0->Divide(2, 2);
  m_c_MeanSVDEventT0->cd(1);
  if (h_MeanSVD3EventT0) h_MeanSVD3EventT0->Draw();
  m_c_MeanSVDEventT0->cd(2);
  if (h_MeanSVD6EventT0) h_MeanSVD6EventT0->Draw();
  m_c_MeanSVDEventT0->cd(3);
  if (h_MeanSVDEventT0) {
    if (h_MeanSVD6EventT0)
      h_MeanSVDEventT0->Add(h_MeanSVD6EventT0);
    h_MeanSVDEventT0->Draw();
  }

  float MPVClusterTimeL3U = nan;
  if (h_clusterTime_L3U)
    if (h_clusterTime_L3U->GetEntries() != 0)
      MPVClusterTimeL3U = xForMaxY(h_clusterTime_L3U);
  float MPVClusterTimeL3V = nan;
  if (h_clusterTime_L3V)
    if (h_clusterTime_L3V->GetEntries() != 0)
      MPVClusterTimeL3V = xForMaxY(h_clusterTime_L3V);
  float MPVClusterTimeL456U = nan;
  if (h_clusterTime_L456U)
    if (h_clusterTime_L456U->GetEntries() != 0)
      MPVClusterTimeL456U = xForMaxY(h_clusterTime_L456U);
  float MPVClusterTimeL456V = nan;
  if (h_clusterTime_L456V)
    if (h_clusterTime_L456V->GetEntries() != 0)
      MPVClusterTimeL456V = xForMaxY(h_clusterTime_L456V);
  float FWHMClusterTimeL3U = nan;
  if (h_clusterTime_L3U)
    if (h_clusterTime_L3U->GetEntries() != 0)
      FWHMClusterTimeL3U = histFWHM(h_clusterTime_L3U);
  float FWHMClusterTimeL3V = nan;
  if (h_clusterTime_L3V)
    if (h_clusterTime_L3V->GetEntries() != 0)
      FWHMClusterTimeL3V = histFWHM(h_clusterTime_L3V);
  float FWHMClusterTimeL456U = nan;
  if (h_clusterTime_L456U)
    if (h_clusterTime_L456U->GetEntries() != 0)
      FWHMClusterTimeL456U = histFWHM(h_clusterTime_L456U);
  float FWHMClusterTimeL456V = nan;
  if (h_clusterTime_L456V)
    if (h_clusterTime_L456V->GetEntries() != 0)
      FWHMClusterTimeL456V = histFWHM(h_clusterTime_L456V);

  float MeanSVD3EventT0 = nan;
  if (h_MeanSVD3EventT0)
    if (h_MeanSVD3EventT0->GetEntries() != 0)
      MeanSVD3EventT0 = xForMaxY(h_MeanSVD3EventT0);

  float MeanSVD6EventT0 = nan;
  if (h_MeanSVD6EventT0)
    if (h_MeanSVD6EventT0->GetEntries() != 0)
      MeanSVD6EventT0 = xForMaxY(h_MeanSVD6EventT0);

  float MeanSVDEventT0 = nan;
  if (h_MeanSVDEventT0)
    if (h_MeanSVDEventT0->GetEntries() != 0)
      MeanSVDEventT0 = xForMaxY(h_MeanSVDEventT0);

  if (h_clusterTime_L3U == NULL || h_clusterTime_L456U == NULL) {
    B2INFO("Histograms needed for MPV cluster time on U side are not found");
  } else {
    m_monObj->setVariable("MPVClusterTimeL3U", MPVClusterTimeL3U);
    m_monObj->setVariable("MPVClusterTimeL456U", MPVClusterTimeL456U);
    m_monObj->setVariable("FWHMClusterTimeL3U", FWHMClusterTimeL3U);
    m_monObj->setVariable("FWHMClusterTimeL456U", FWHMClusterTimeL456U);
  }

  if (h_clusterTime_L3V == NULL || h_clusterTime_L456V == NULL) {
    B2INFO("Histograms needed for MPV cluster time on V side are not found");
  } else {
    m_monObj->setVariable("MPVClusterTimeL3V", MPVClusterTimeL3V);
    m_monObj->setVariable("MPVClusterTimeL456V", MPVClusterTimeL456V);
    m_monObj->setVariable("FWHMClusterTimeL3V", FWHMClusterTimeL3V);
    m_monObj->setVariable("FWHMClusterTimeL456V", FWHMClusterTimeL456V);
  }

  if (h_MeanSVD3EventT0 == NULL) {
    B2INFO("Histograms needed for SVD Event T0 (3 samples) not found");
  } else {
    m_monObj->setVariable("MeanSVD3EventT0", MeanSVD3EventT0);
  }

  if (h_MeanSVD6EventT0 == NULL) {
    B2INFO("Histograms needed for SVD Event T0 (6 samples) not found");
  } else {
    m_monObj->setVariable("MeanSVD6EventT0", MeanSVD6EventT0);
  }

  if (h_MeanSVDEventT0 == NULL) {
    B2INFO("Histograms needed for SVD Event T0 (all samples) not found");
  } else {
    m_monObj->setVariable("MeanSVDEventT0", MeanSVDEventT0);
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
  } else {
    float avgMaxBinU = h_maxBinU->GetMean();
    m_monObj->setVariable("avgMaxBinU", avgMaxBinU);
  }

  if (h_maxBinV == NULL) {
    B2INFO("Histogram needed for Average MaxBin on V side is not found");
  } else {
    float avgMaxBinV = h_maxBinV->GetMean();
    m_monObj->setVariable("avgMaxBinV", avgMaxBinV);
  }

  std::map<std::pair<int, int>, int> ladderMap =  {
    {{3, 1}, 0}, {{3, 2}, 1},
    {{4, 1}, 2}, {{4, 2}, 3}, {{4, 3}, 4},
    {{5, 1}, 5}, {{5, 2}, 6}, {{5, 3}, 7}, {{5, 4}, 8},
    {{6, 1}, 9}, {{6, 2}, 10}, {{6, 3}, 11}, {{6, 4}, 12}, {{6, 5}, 13}
  };


  for (const auto& it : ladderMap) {
    std::pair<int, int> p = it.first;
    int layer = p.first;
    int sensor = p.second;

    TString  name = Form("SVDClsTrk/SVDTRK_ClusterCharge_L%d.x.%d", layer, sensor);
    TString title = Form("MPVClusterCharge_L%d.x.%d", layer, sensor);
    TH1F* h_clusterCharge = (TH1F*)findHist(name.Data());
    float MPVClusterCharge = nan;
    if (h_clusterCharge)
      if (h_clusterCharge->GetEntries() != 0)
        MPVClusterCharge = xForMaxY(h_clusterCharge);

    if (h_clusterCharge == NULL) {
      B2INFO("Histograms needed for cluster charge not found");
    } else {
      m_monObj->setVariable(title.Data(), MPVClusterCharge);
    }

    name = Form("SVDClsTrk/SVDTRK_ClusterSNR_L%d.x.%d", layer, sensor);
    title = Form("MPVClusterSNR_L%d.x.%d", layer, sensor);
    TH1F* h_clusterSNR = (TH1F*)findHist(name.Data());
    float MPVClusterSNR = nan;
    if (h_clusterSNR)
      if (h_clusterSNR->GetEntries() != 0)
        MPVClusterSNR = xForMaxY(h_clusterSNR);

    if (h_clusterSNR == NULL) {
      B2INFO("Histograms needed for cluster SNR not found");
    } else {
      m_monObj->setVariable(title.Data(), MPVClusterSNR);
    }
  }

  for (int ladder = 1; ladder <= 2; ++ladder) {
    for (int sensor = 1; sensor <= 2; ++sensor) {

      TString  name = Form("SVDClsTrk/SVDTRK_ClusterCharge_L3.%d.%d", ladder, sensor);
      TString  title =  Form("MPVClusterCharge_L3.%d.%d", ladder, sensor);
      float MPVClusterCharge = nan;
      TH1F* h_clusterCharge = (TH1F*)findHist(name.Data());
      if (h_clusterCharge)
        if (h_clusterCharge->GetEntries() != 0)
          MPVClusterCharge = xForMaxY(h_clusterCharge);

      if (h_clusterCharge == NULL) {
        B2INFO("Histograms needed for cluster charge not found");
      } else {
        m_monObj->setVariable(title.Data(), MPVClusterCharge);
      }

      name = Form("SVDClsTrk/SVDTRK_ClusterSNR_L3.%d.%d", ladder, sensor);
      title = Form("MPVClusterSNR_L3.%d.%d", ladder, sensor);
      TH1F* h_clusterSNR = (TH1F*)findHist(name.Data());
      float MPVClusterSNR = nan;
      if (h_clusterSNR)
        if (h_clusterSNR->GetEntries() != 0)
          MPVClusterSNR = xForMaxY(h_clusterSNR);

      if (h_clusterSNR == NULL) {
        B2INFO("Histograms needed for cluster SNR not found");
      } else {
        m_monObj->setVariable(title.Data(), MPVClusterSNR);
      }
    }
  }

  B2INFO("DQMHistAnalysisSVDGeneral: endRun called");
}


void DQMHistAnalysisSVDOnMiraBelleModule::terminate()
{
  B2INFO("DQMHistAnalysisSVDOnMiraBelle: terminate called");
}


std::vector<float> DQMHistAnalysisSVDOnMiraBelleModule::getOccupancySensorUV(int iLayer, TH1F* hU, TH1F* hV, int binX, int binY,
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
  int iBin = hU->GetBin(binX, binY);

  avgOffOccUV[0] = 0;
  if (hU) avgOffOccUV[0] = hU->GetBinContent(iBin) * 1.0 / 768 / nEvents * 100;
  avgOffOccUV[1] = 0;
  if (hV) avgOffOccUV[1] = hV->GetBinContent(iBin) * 1.0 / nStripsV / nEvents * 100;
  return avgOffOccUV;
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
  avgOffOccUV[0] = 0;
  if (hU) avgOffOccUV[0] = hU->GetBinContent(iBin) * 1.0 / 768 / nEvents * 100;
  avgOffOccUV[1] = 0;
  if (hV) avgOffOccUV[1] = hV->GetBinContent(iBin) * 1.0 / nStripsV / nEvents * 100;
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

std::vector<float> DQMHistAnalysisSVDOnMiraBelleModule::avgOccupancyGrpId0UV(int iLayer, int nEvents) const
{
  int nStripsV = -1;
  if (iLayer == 3) {
    nStripsV = 768;
  } else if (iLayer >= 4 && iLayer <= 6) {
    nStripsV = 512;
  } else {
    B2DEBUG(20, "Layer out of range [3,6].");
  }

  Int_t nStripsU = 768;

  std::vector<float> avgOffOccU;
  std::vector<float> avgOffOccV;

  for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
    int tmp_layer = m_SVDModules[i].getLayerNumber();
    int tmp_ladder = m_SVDModules[i].getLadderNumber();
    int tmp_sensor = m_SVDModules[i].getSensorNumber();

    TString tmpnameGrpId0U = Form("SVDExpReco/SVDDQM_%d_%d_%d_StripCountGroupId0U", tmp_layer, tmp_ladder, tmp_sensor);
    TH1F* htmpU = (TH1F*)findHist(tmpnameGrpId0U.Data());
    if (htmpU == NULL) {
      B2INFO("Occupancy U histogram for group Id0 not found");
    } else {
      if (tmp_layer == iLayer)
        avgOffOccU.push_back(htmpU->GetEntries() / nStripsU / nEvents * 100);
    }

    TString tmpnameGrpId0V = Form("SVDExpReco/SVDDQM_%d_%d_%d_StripCountGroupId0V", tmp_layer, tmp_ladder, tmp_sensor);
    TH1F* htmpV = (TH1F*)findHist(tmpnameGrpId0V.Data());
    if (htmpV == NULL) {
      B2INFO("Occupancy V histogram for group Id0 not found");
    } else {
      if (tmp_layer == iLayer)
        avgOffOccV.push_back(htmpV->GetEntries() / nStripsV / nEvents * 100);
    }
  }

  std::vector<float> avgOffOccUV(2, 0.);

  avgOffOccUV[0] = accumulate(avgOffOccU.begin(), avgOffOccU.end(), 0.0);
  avgOffOccUV[0] /= float(avgOffOccU.size());

  avgOffOccUV[1] = accumulate(avgOffOccV.begin(), avgOffOccV.end(), 0.0);
  avgOffOccUV[1] /= float(avgOffOccV.size());

  return avgOffOccUV;
}

std::vector<float> DQMHistAnalysisSVDOnMiraBelleModule::avgEfficiencyUV(TH2F* hMCU, TH2F* hMCV, TH2F* hFTU, TH2F* hFTV, int minX,
    int maxX, int minY, int maxY) const
{
  float nan = numeric_limits<float>::quiet_NaN();

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
    avgEffUV[0] = nan;
  }
  if (sumFoundTracksUV[1] > 0) {
    avgEffUV[1] = sumMatchedClustersUV[1] / sumFoundTracksUV[1] * 100;
  } else {
    avgEffUV[1] = nan;
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
