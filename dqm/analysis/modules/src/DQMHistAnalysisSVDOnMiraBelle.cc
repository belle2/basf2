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
#include <vxd/dataobjects/VxdID.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoTools.h>

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

  m_gTools = VXD::GeoCache::getInstance().getGeoTools();

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
    std::pair<float, float> avgOffOccL3UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 3);
    SetVariable(avgOffOccL3UV);

    std::pair<float, float> avgOffOccL4UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 4);
    SetVariable(avgOffOccL4UV);

    std::pair<float, float> avgOffOccL5UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 5);
    SetVariable(avgOffOccL5UV);

    std::pair<float, float> avgOffOccL6UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 6);
    SetVariable(avgOffOccL6UV);

    // average occupancy for each layer
    std::pair<float, float> avgOffGrpId0OccL3UV = avgOccupancyGrpId0UV(3, nE);
    SetVariable(avgOffGrpId0OccL3UV);

    std::pair<float, float> avgOffGrpId0OccL4UV = avgOccupancyGrpId0UV(4, nE);
    SetVariable(avgOffGrpId0OccL4UV);

    std::pair<float, float> avgOffGrpId0OccL5UV = avgOccupancyGrpId0UV(5, nE);
    SetVariable(avgOffGrpId0OccL5UV);

    std::pair<float, float> avgOffGrpId0OccL6UV = avgOccupancyGrpId0UV(6, nE);
    SetVariable(avgOffGrpId0OccL6UV);

    // occupancy averaged over ladders
    std::pair<float, float> avgOffOccL3X1UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 3, -1, 1); // L3.X.1
    SetVariable(avgOffOccL3X1UV);

    std::pair<float, float> avgOffOccL3X2UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 3, -1, 2); // L3.X.2
    SetVariable(avgOffOccL3X2UV);

    std::pair<float, float> avgOffOccL4X1UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 4, -1, 1); // L4.X.1
    SetVariable(avgOffOccL4X1UV);

    std::pair<float, float> avgOffOccL4X2UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 4, -1, 2); // L4.X.2
    SetVariable(avgOffOccL4X2UV);

    std::pair<float, float> avgOffOccL4X3UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 4, -1, 3); // L4.X.3
    SetVariable(avgOffOccL4X3UV);

    std::pair<float, float> avgOffOccL5X1UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 5, -1, 1); // L5.X.1
    SetVariable(avgOffOccL5X1UV);

    std::pair<float, float> avgOffOccL5X2UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 5, -1, 2); // L5.X.2
    SetVariable(avgOffOccL5X2UV);

    std::pair<float, float> avgOffOccL5X3UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 5, -1, 3); // L5.X.3
    SetVariable(avgOffOccL5X3UV);

    std::pair<float, float> avgOffOccL5X4UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 5, -1, 4); // L5.X.4
    SetVariable(avgOffOccL5X4UV);

    std::pair<float, float> avgOffOccL6X1UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 6, -1, 1); // L6.X.1
    SetVariable(avgOffOccL6X1UV);

    std::pair<float, float> avgOffOccL6X2UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 6, -1, 2); // L6.X.2
    SetVariable(avgOffOccL6X2UV);

    std::pair<float, float> avgOffOccL6X3UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 6, -1, 3); // L6.X.3
    SetVariable(avgOffOccL6X3UV);

    std::pair<float, float> avgOffOccL6X4UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 6, -1, 4); // L6.X.4
    SetVariable(avgOffOccL6X4UV);

    std::pair<float, float> avgOffOccL6X5UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 6, -1, 5); // L6.X.5
    SetVariable(avgOffOccL6X5UV);

    // average occupancy for high occupancy sensors
    std::pair<float, float> avgOffOccL311UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 3, 1, 1); // L3.1.1
    SetVariable(avgOffOccL311UV);

    std::pair<float, float> avgOffOccL312UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 3, 1, 2); // L3.1.2
    SetVariable(avgOffOccL312UV);

    std::pair<float, float> avgOffOccL321UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 3, 2, 1); // L3.2.1
    SetVariable(avgOffOccL321UV);

    std::pair<float, float> avgOffOccL322UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 3, 2, 2); // L3.2.2
    SetVariable(avgOffOccL322UV);

    std::pair<float, float> avgOffOccL461UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 4, 6, 1); // L4.6.1
    SetVariable(avgOffOccL461UV);

    std::pair<float, float> avgOffOccL462UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 4, 6, 2); // L4.6.2
    SetVariable(avgOffOccL462UV);

    std::pair<float, float> avgOffOccL581UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 5, 8, 1); // L5.8.1
    SetVariable(avgOffOccL581UV);

    std::pair<float, float> avgOffOccL582UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 5, 8, 2); // L5.8.2
    SetVariable(avgOffOccL582UV);

    std::pair<float, float> avgOffOccL6101UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 6, 10, 1); // L6.10.1
    SetVariable(avgOffOccL6101UV);

    std::pair<float, float> avgOffOccL6102UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 6, 10, 2); // L6.10.2
    SetVariable(avgOffOccL6102UV);

    // average occupancy for low DCDC
    // L3.2.1 above
    // L3.2.2 above
    std::pair<float, float> avgOffOccL411UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 4, 1, 1); // L4.1.1
    SetVariable(avgOffOccL411UV);

    std::pair<float, float> avgOffOccL4102UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 4, 10, 2); // L4.10.2
    SetVariable(avgOffOccL4102UV);

    // average occupancy for peculiar sensors
    std::pair<float, float> avgOffOccL433UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 4, 3, 3); // L4.3.3
    SetVariable(avgOffOccL433UV);

    std::pair<float, float> avgOffOccL513UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 5, 1, 3); // L5.1.3
    SetVariable(avgOffOccL513UV);

    std::pair<float, float> avgOffOccL514UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 5, 1, 4); // L5.1.4
    SetVariable(avgOffOccL514UV);

    std::pair<float, float> avgOffOccL592UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 5, 9, 2); // L5.9.2
    SetVariable(avgOffOccL592UV);

    std::pair<float, float> avgOffOccL594UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 5, 9, 4); // L5.9.4
    SetVariable(avgOffOccL594UV);

    std::pair<float, float> avgOffOccL643UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 6, 4, 3); // L6.4.3
    SetVariable(avgOffOccL643UV);

    std::pair<float, float> avgOffOccL664UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 6, 6, 4); // L6.6.4
    SetVariable(avgOffOccL664UV);

    std::pair<float, float> avgOffOccL6103UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 6, 10, 3); // L6.10.3
    SetVariable(avgOffOccL6103UV);

    std::pair<float, float> avgOffOccL6115UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 6, 11, 5); // L6.11.5
    SetVariable(avgOffOccL6115UV);

    std::pair<float, float> avgOffOccL6124UV = avgOccupancyUV(h_zs5countsU, h_zs5countsV, nE, 6, 12, 4); // L6.12.4
    SetVariable(avgOffOccL6124UV);
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
    std::pair<float, float> avgEffL3 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 3);
    SetVariable(avgEffL3);

    std::pair<float, float> avgEffL4 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 4);
    SetVariable(avgEffL4);

    std::pair<float, float> avgEffL5 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 5);
    SetVariable(avgEffL5);

    std::pair<float, float> avgEffL6 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 6);
    SetVariable(avgEffL6);

    // average efficiency for all layers
    std::pair<float, float> avgEffL3456 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV);
    SetVariable(avgEffL3456);

    // average efficiency for mid plane: L3.X.1
    std::pair<float, float> avgEffL3X1 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 3, -1, 1);
    SetVariable(avgEffL3X1);

    // average efficiency for mid plane: L3.X.2
    std::pair<float, float> avgEffL3X2 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 3, -1, 2);
    SetVariable(avgEffL3X2);

    // average efficiency for mid plane: L4.X.1
    std::pair<float, float> avgEffL4X1 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 4, -1, 1);
    SetVariable(avgEffL4X1);

    // average efficiency for mid plane: L4.X.2
    std::pair<float, float> avgEffL4X2 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 4, -1, 2);
    SetVariable(avgEffL4X2);

    // average efficiency for mid plane: L4.X.3
    std::pair<float, float> avgEffL4X3 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 4, -1, 3);
    SetVariable(avgEffL4X3);

    // average efficiency for mid plane: L5.X.1
    std::pair<float, float> avgEffL5X1 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 5, -1, 1);
    SetVariable(avgEffL5X1);

    // average efficiency for mid plane: L5.X.2
    std::pair<float, float> avgEffL5X2 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 5, -1, 2);
    SetVariable(avgEffL5X2);

    // average efficiency for mid plane: L5.X.3
    std::pair<float, float> avgEffL5X3 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 5, -1, 3);
    SetVariable(avgEffL5X3);

    // average efficiency for mid plane: L5.X.4
    std::pair<float, float> avgEffL5X4 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 5, -1, 4);
    SetVariable(avgEffL5X4);

    // average efficiency for mid plane: L6.X.1
    std::pair<float, float> avgEffL6X1 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 6, -1, 1);
    SetVariable(avgEffL6X1);

    // average efficiency for mid plane: L6.X.2
    std::pair<float, float> avgEffL6X2 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 6, -1, 2);
    SetVariable(avgEffL6X2);

    // average efficiency for mid plane: L6.X.3
    std::pair<float, float> avgEffL6X3 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 6, -1, 3);
    SetVariable(avgEffL6X3);

    // average efficiency for mid plane: L6.X.4
    std::pair<float, float> avgEffL6X4 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 6, -1, 4);
    SetVariable(avgEffL6X4);

    // average efficiency for mid plane: L6.X.5
    std::pair<float, float> avgEffL6X5 = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 6, -1, 5);
    SetVariable(avgEffL6X5);

    // average efficiency for high occupancy sensors
    std::pair<float, float> avgEffL311UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 3, 1,
                                                           1); // L3.1.1
    SetVariable(avgEffL311UV);

    std::pair<float, float> avgEffL312UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 3, 1,
                                                           2); // L3.1.2
    SetVariable(avgEffL312UV);

    std::pair<float, float> avgEffL321UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 3, 2,
                                                           1); // L3.2.1
    SetVariable(avgEffL321UV);

    std::pair<float, float> avgEffL322UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 3, 2,
                                                           2); // L3.2.2
    SetVariable(avgEffL322UV);

    std::pair<float, float> avgEffL461UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 4, 6,
                                                           1); // L4.6.1
    SetVariable(avgEffL461UV);

    std::pair<float, float> avgEffL462UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 4, 6,
                                                           2); // L4.6.2
    SetVariable(avgEffL462UV);

    std::pair<float, float> avgEffL581UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 5, 8,
                                                           1); // L5.8.1
    SetVariable(avgEffL581UV);

    std::pair<float, float> avgEffL582UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 5, 8,
                                                           2); // L5.8.2
    SetVariable(avgEffL582UV);

    std::pair<float, float> avgEffL6101UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 6, 10,
                                                            1); // L6.10.1
    SetVariable(avgEffL6101UV);

    std::pair<float, float> avgEffL6102UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 6, 10,
                                                            2); // L6.10.2
    SetVariable(avgEffL6102UV);

    // average efficiency for low DCDC
    // L3.2.1 above
    // L3.2.2 above
    std::pair<float, float> avgEffL411UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 4, 1,
                                                           1); // L4.1.1
    SetVariable(avgEffL411UV);

    std::pair<float, float> avgEffL4102UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 4, 10,
                                                            2); // L4.10.2
    SetVariable(avgEffL4102UV);

    // average efficiency for peculiar sensors
    std::pair<float, float> avgEffL433UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 4, 3,
                                                           3); // L4.3.3
    SetVariable(avgEffL433UV);

    std::pair<float, float> avgEffL513UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 5, 1,
                                                           3); // L5.1.3
    SetVariable(avgEffL513UV);

    std::pair<float, float> avgEffL514UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 5, 1,
                                                           4); // L5.1.4
    SetVariable(avgEffL514UV);

    std::pair<float, float> avgEffL592UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 5, 9,
                                                           2); // L5.9.2
    SetVariable(avgEffL592UV);

    std::pair<float, float> avgEffL594UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 5, 9,
                                                           4); // L5.9.4
    SetVariable(avgEffL594UV);

    std::pair<float, float> avgEffL643UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 6, 4,
                                                           3); // L6.4.3
    SetVariable(avgEffL643UV);

    std::pair<float, float> avgEffL664UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 6, 6,
                                                           4); // L6.6.4
    SetVariable(avgEffL664UV);

    std::pair<float, float> avgEffL6103UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 6, 10,
                                                            3);  // L6.10.3
    SetVariable(avgEffL6103UV);

    std::pair<float, float> avgEffL6115UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 6, 11,
                                                            5); // L6.11.5
    SetVariable(avgEffL6115UV);

    std::pair<float, float> avgEffL6124UV = avgEfficiencyUV(h_matched_clusU, h_matched_clusV, h_found_tracksU, h_found_tracksV, 6, 12,
                                                            4); // L6.12.4
    SetVariable(avgEffL6124UV);
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


  B2INFO("DQMHistAnalysisSVDGeneral: endRun called");
}


void DQMHistAnalysisSVDOnMiraBelleModule::terminate()
{
  B2INFO("DQMHistAnalysisSVDOnMiraBelle: terminate called");
}

std::pair<float, float> DQMHistAnalysisSVDOnMiraBelleModule::avgOccupancyUV(TH1F* hU, TH1F* hV,  int nEvents,
    int layer, int ladder, int sensor) const
{
  int nStripsV = -1;
  if (layer == 3) {
    nStripsV = 768;
  } else if (layer >= 4 && layer <= 6) {
    nStripsV = 512;
  } else {
    B2DEBUG(20, "Layer out of range [3,6].");
  }
  std::pair<float, float> avgOffOccUV(0.0, 0.0);

  int minLayer = (layer != -1) ? layer : m_gTools->getFirstSVDLayer();
  int maxLayer = (layer != -1) ? layer : m_gTools->getLastSVDLayer();
  int sensorsN = 0;

  if (ladder == 0) ladder = -1;

  for (int layerId = minLayer; layerId < maxLayer + 1; ++layerId) {
    int minLadder = (ladder != -1) ? ladder : 1;
    int maxLadder = (ladder != -1) ? ladder : getNumberOfLadders(layerId);

    int minSensor = (sensor != -1) ? sensor : 1;
    int maxSensor = (sensor != -1) ? sensor : getNumberOfSensors(layerId);

    for (int sensorId = minSensor; sensorId < maxSensor + 1; ++sensorId) {

      for (int ladderId = minLadder; ladderId < maxLadder + 1; ++ladderId) {
        int bin = m_gTools->getSVDSensorIndex(layerId, ladderId, sensorId) + 1;

        avgOffOccUV.first += hU->GetBinContent(bin) / 768 * 100;
        avgOffOccUV.second += hV->GetBinContent(bin) / nStripsV * 100;
        sensorsN++;
      }
    }
  }

  avgOffOccUV.first /= (sensorsN * nEvents);
  avgOffOccUV.second /= (sensorsN * nEvents);

  return avgOffOccUV;
}

std::pair<float, float> DQMHistAnalysisSVDOnMiraBelleModule::avgOccupancyGrpId0UV(int iLayer, int nEvents) const
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

  std::pair<float, float> avgOffOccUV(0., 0.);

  avgOffOccUV.first = accumulate(avgOffOccU.begin(), avgOffOccU.end(), 0.0);
  avgOffOccUV.first /= float(avgOffOccU.size());

  avgOffOccUV.second = accumulate(avgOffOccV.begin(), avgOffOccV.end(), 0.0);
  avgOffOccUV.second /= float(avgOffOccV.size());

  return avgOffOccUV;
}

std::pair<float, float> DQMHistAnalysisSVDOnMiraBelleModule::avgEfficiencyUV(TH2F* hMCU, TH2F* hMCV, TH2F* hFTU, TH2F* hFTV,
    int layer,
    int ladder, int sensor) const
{
  float nan = numeric_limits<float>::quiet_NaN();
  std::pair<float, float> avgEffUV(0.0, 0.0);
  std::pair<float, float> sumMatchedClustersUV(0.0, 0.0);
  std::pair<float, float> sumFoundTracksUV(0.0, 0.0);

  int minLayer = (layer != -1) ? layer : m_gTools->getFirstSVDLayer();
  int maxLayer = (layer != -1) ? layer : m_gTools->getLastSVDLayer();

  if (ladder == 0) ladder = -1;

  for (int layerId = minLayer; layerId < maxLayer + 1; ++layerId) {
    int minLadder = (ladder != -1) ? ladder : 1;
    int maxLadder = (ladder != -1) ? ladder : getNumberOfLadders(layerId);

    int minSensor = (sensor != -1) ? sensor : 1;
    int maxSensor = (sensor != -1) ? sensor : getNumberOfSensors(layerId);

    for (int sensorId = minSensor; sensorId < maxSensor + 1; ++sensorId) {

      for (int ladderId = minLadder; ladderId < maxLadder + 1; ++ladderId) {
        int binY = findBinY(layerId, sensorId);
        int binXY = hMCV->FindBin(ladderId, binY);

        sumMatchedClustersUV.first += hMCU->GetBinContent(binXY);
        sumMatchedClustersUV.second += hMCV->GetBinContent(binXY);
        sumFoundTracksUV.first += hFTU->GetBinContent(binXY);
        sumFoundTracksUV.second += hFTV->GetBinContent(binXY);
      }
    }
  }

  if (sumFoundTracksUV.first > 0) {
    avgEffUV.first = sumMatchedClustersUV.first / sumFoundTracksUV.first * 100;
  } else {
    avgEffUV.first = nan;
  }

  if (sumFoundTracksUV.second > 0) {
    avgEffUV.second = sumMatchedClustersUV.second / sumFoundTracksUV.second * 100;
  } else {
    avgEffUV.second = nan;
  }

  return avgEffUV;
}


void DQMHistAnalysisSVDOnMiraBelleModule::addVariable(string name, pair<float, float>& varUV)
{
  size_t pos = name.find("UV");

  if (pos != string::npos)
    name.replace(pos, 2, "");

  m_monObj->setVariable(Form("%sU", name.c_str()), varUV.first);
  m_monObj->setVariable(Form("%sV", name.c_str()), varUV.second);
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
