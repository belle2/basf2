/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdCrossTalkCalibrationsCollector/SVDCrossTalkCalibrationsCollectorModule.h>

#include <svd/modules/svdCrossTalkFinder/SVDCrossTalkFinderHelperFunctions.h>

#include <vxd/geometry/GeoCache.h>

#include <TH1F.h>

#include <iostream>


using namespace std;
using namespace Belle2;


REG_MODULE(SVDCrossTalkCalibrationsCollector)

SVDCrossTalkCalibrationsCollectorModule::SVDCrossTalkCalibrationsCollectorModule() : CalibrationCollectorModule()
{
  setDescription("This module collects the list of channels exhibiting crossTalk readout.");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDShaperDigits", m_svdShaperDigitsName,
           "SVDShaperDigit collection name", string(""));

  addParam("uSideOccupancyFactor", m_uSideOccupancyFactor,
           "Multiple of the average occupancy for high occupancy strip classification", 2);

  addParam("vSideOccupancyFactor", m_vSideOccupancyFactor,
           "Multiple of the average occupancy for high occupancy strip classification", 2);

  addParam("nAPVFactor", m_nAPVFactor,
           "Number of APV chips with at least one high occupancy strips in event required for cross talk flag", 30);

}

void SVDCrossTalkCalibrationsCollectorModule::prepare()
{

  m_svdShaperDigits.isRequired(m_svdShaperDigitsName);


  m_histogramTree = new TTree("tree", "tree");
  m_histogramTree->Branch("hist", "TH1F", &m_hist, 32000, 0);
  m_histogramTree->Branch("layer", &m_layer, "layer/I");
  m_histogramTree->Branch("ladder", &m_ladder, "ladder/I");
  m_histogramTree->Branch("sensor", &m_sensor, "sensor/I");
  m_histogramTree->Branch("side", &m_side, "side/I");

  registerObject<TTree>("HTreeCrossTalkCalib", m_histogramTree);


}

void SVDCrossTalkCalibrationsCollectorModule::startRun()
{

  std::string sensorName;
  TH1F* sensorHist;
  //Prepare histograms for payload.
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (auto& layers : geo.getLayers(VXD::SensorInfoBase::SVD)) {
    for (auto& ladders : geo.getLadders(layers)) {
      for (auto& sensors : geo.getSensors(ladders)) {
        for (int side = 0; side <= 1; side++) {
          occupancyPDFName(sensors, side, sensorName);
          if (m_sensorHistograms.count(sensorName) == 0) {
            if (layers.getLayerNumber() == 3 or side == 1) {
              sensorHist  = new TH1F(sensorName.c_str(), "", 768, 0, 768);
            } else {
              sensorHist  = new TH1F(sensorName.c_str(), "", 512, 0, 512);
            }
            m_sensorHistograms[sensorName] = sensorHist;

          }

        }
      }
    }
  }

}

void SVDCrossTalkCalibrationsCollectorModule::collect()
{
  //Vectors to hold event info.
  vector<std::string> highOccChips_uSide ;
  vector<std::string> highOccChips_vSide ;
  vector<std::string> highOccChipsStripNum_uSide ;
  vector<std::string> highOccChipsStripNum_vSide ;
  vector<std::string> clusterStrips_uSide ;
  vector<std::string> clusterStrips_vSide ;
  vector<std::string> clusterChips_uSide ;
  vector<int> strips_uSide;
  vector<int> strips_vSide;

  //loop over ShaperDigits
  for (auto& svdShaperDigit : m_svdShaperDigits) {
    //Remove L3 and +fw sensors, not affected by cross-talk
    if (svdShaperDigit.getSensorID().getLayerNumber() == 3 or svdShaperDigit.getSensorID().getSensorNumber() == 1) {
      continue;
    }

    int side = svdShaperDigit.isUStrip();
    std::string sensorName;
    occupancyPDFName(svdShaperDigit.getSensorID(), side, sensorName);

    double sensorAverage = 0.;
    calculateAverage(svdShaperDigit.getSensorID(), sensorAverage, side);
    int stripID = svdShaperDigit.getCellID();
    std::string sensorStripNum = sensorName + "." + std::to_string(stripID);
    double stripOccupancy = m_OccupancyCal.getOccupancy(svdShaperDigit.getSensorID(), side, stripID);
    //Clustering only works assuming digits are ordered.//
    if (side == 1 && stripOccupancy > (m_uSideOccupancyFactor * sensorAverage)) {

      int adjacentStrip = 0;
      if (!strips_uSide.empty()) {
        adjacentStrip = stripID - strips_uSide.back();
      }
      strips_uSide.push_back(stripID);
      if (!highOccChips_uSide.empty() && sensorName == highOccChips_uSide.back() && adjacentStrip == 1) {
        clusterChips_uSide.push_back(sensorName);
        if (clusterStrips_uSide.empty() || clusterStrips_uSide.back() != sensorStripNum) {
          clusterStrips_uSide.push_back(highOccChipsStripNum_uSide.back());
        }
        clusterStrips_uSide.push_back(sensorStripNum);
      }
      highOccChipsStripNum_uSide.push_back(sensorStripNum);
      highOccChips_uSide.push_back(sensorName);
    }

    if (side == 0 && stripOccupancy > (m_vSideOccupancyFactor * sensorAverage)) {
      int adjacentStrip = 0;
      if (!strips_vSide.empty()) {
        adjacentStrip = stripID - strips_vSide.back();
      }
      strips_vSide.push_back(stripID);
      if (!highOccChips_vSide.empty() && sensorName == highOccChips_vSide.back() && adjacentStrip == 1) {
        if (clusterStrips_vSide.empty() || clusterStrips_vSide.back() != sensorStripNum) {
          clusterStrips_vSide.push_back(highOccChipsStripNum_vSide.back());
        }
        clusterStrips_vSide.push_back(sensorStripNum);
      }
      highOccChipsStripNum_vSide.push_back(sensorStripNum);
      highOccChips_vSide.push_back(sensorName);
    }

  } //ShaperDigit loop

  std::sort(clusterChips_uSide.begin(), clusterChips_uSide.end());
  clusterChips_uSide.erase(unique(clusterChips_uSide.begin(), clusterChips_uSide.end()), clusterChips_uSide.end());
  int numberOfClusterChips = std::distance(clusterChips_uSide.begin(), std::unique(clusterChips_uSide.begin(),
                                           clusterChips_uSide.end()));

//   Crosstalk events flagged using u-side
  if (numberOfClusterChips > m_nAPVFactor) {
    for (auto& svdShaperDigit : m_svdShaperDigits) {
      std::string sensorID = svdShaperDigit.getSensorID();
      std::string digitID = sensorID + "." + std::to_string(svdShaperDigit.isUStrip());
      std::string stripID = digitID + "." + std::to_string(svdShaperDigit.getCellID());
      if (std::find(clusterStrips_uSide.begin(), clusterStrips_uSide.end(), stripID) != clusterStrips_uSide.end()) {
        std::string sensorName;
        occupancyPDFName(svdShaperDigit.getSensorID(), svdShaperDigit.isUStrip(), sensorName);
        auto xTalkStrip = m_sensorHistograms.at(sensorName);
        xTalkStrip->Fill(svdShaperDigit.getCellID(), 1);
      }
      if (std::find(clusterStrips_vSide.begin(), clusterStrips_vSide.end(), stripID) != clusterStrips_vSide.end()) {
        std::string sensorName;
        occupancyPDFName(svdShaperDigit.getSensorID(), svdShaperDigit.isUStrip(), sensorName);
        auto xTalkStrip = m_sensorHistograms.at(sensorName);
        xTalkStrip->Fill(svdShaperDigit.getCellID(), 1);
      }


    }//shaper digit loop
  }
} //Collector loop


void SVDCrossTalkCalibrationsCollectorModule::finish()
{}

void SVDCrossTalkCalibrationsCollectorModule::closeRun()
{
  std::string sensorName;
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (auto& layers : geo.getLayers(VXD::SensorInfoBase::SVD)) {
    for (auto& ladders : geo.getLadders(layers)) {
      for (auto& sensors : geo.getSensors(ladders)) {
        for (int side = 0; side <= 1; side++) {

          occupancyPDFName(sensors, side, sensorName);
          auto sensorOnMap = m_sensorHistograms.at(sensorName);
          m_hist = sensorOnMap;
          m_layer = sensors.getLayerNumber();
          m_ladder = sensors.getLadderNumber();
          m_sensor = sensors.getSensorNumber();
          m_side = side;

          getObjectPtr<TTree>("HTreeCrossTalkCalib")->Fill();

          sensorOnMap->Delete();
        }
      }
    }
  }

}


void SVDCrossTalkCalibrationsCollectorModule::calculateAverage(const VxdID& sensorID, double& mean, int side)
{
  double nBins = 0;
  if (side == 1) nBins = 768; //U-side 768 channels
  else nBins = 512;
  double count = 0;
  for (int i = 0; i < nBins; i++) {
    count += m_OccupancyCal.getOccupancy(sensorID, side, i);
  }
  mean = count / nBins;
}

