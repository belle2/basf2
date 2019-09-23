/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: James Webb                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdCrossTalkCalibrationsCollector/SVDCrossTalkCalibrationsCollectorModule.h>

using namespace std;
using namespace Belle2;


REG_MODULE(SVDCrossTalkCalibrationsCollector)

SVDCrossTalkCalibrationsCollectorModule::SVDCrossTalkCalibrationsCollectorModule() : CalibrationCollectorModule()
{
  setDescription("This module collects the list of channels exhibiting crossTalk readout.");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDRecoDigits", m_svdRecoDigitsName,
           "SVDRecoDigit collection name", string(""));

  addParam("HistogramTree", m_treeName, "Name of the tree in which the histograms are saved", std::string("tree"));

  addParam("uSideOccupancyFactor", m_uSideOccupancyFactor,
           "Multiple of the average occupancy for high occupancy strip classification", 2);

  addParam("vSideOccupancyFactor", m_vSideOccupancyFactor,
           "Multiple of the average occupancy for high occupancy strip classification", 2);

  addParam("nAPVFactor", m_nAPVFactor,
           "Number of APV chips with at least one high occupancy strips in event required for cross talk flag", 30);

}

void SVDCrossTalkCalibrationsCollectorModule::prepare()
{

  m_svdRecoDigits.isRequired(m_svdRecoDigitsName);

//  TH1F hCrossTalkStrips768("CrossTalkStrips768_L@LayerL@ladderS@sensor@side","",0,768);
//  TH1F hCrossTalkStrips512("CrossTalkStrips768_L@LayerL@ladderS@sensor@side","",0,512);

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
          occupancyPDFName(sensors, side, sensorName); //How am i going to go about naming?
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
  vector<std::string> clusterChips_vSide ;
  vector<int> strips_uSide;
  vector<int> strips_vSide;

  //loop over RecoDigits

  for (auto& svdRecoDigit : m_svdRecoDigits) {
    //Remove L3 and +fw sensors, not affected by cross-talk
    if (svdRecoDigit.getSensorID().getLayerNumber() == 3 or svdRecoDigit.getSensorID().getSensorNumber() == 1) {
      continue;
    }

    int side = svdRecoDigit.isUStrip();
    std::string sensorName;
    occupancyPDFName(svdRecoDigit.getSensorID(), side, sensorName);

    double sensorAverage = 0.;
    calculateAverage(svdRecoDigit.getSensorID(), sensorAverage, side);
    int stripID = svdRecoDigit.getCellID();
    std::string sensorStripNum = sensorName + "." + std::to_string(stripID);
    double stripOccupancy = m_OccupancyCal.getOccupancy(svdRecoDigit.getSensorID(), side, stripID);
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

  } //RecoDigit loop

  std::sort(clusterChips_uSide.begin(), clusterChips_uSide.end());
  clusterChips_uSide.erase(unique(clusterChips_uSide.begin(), clusterChips_uSide.end()), clusterChips_uSide.end());
  int numberOfClusterChips = std::distance(clusterChips_uSide.begin(), std::unique(clusterChips_uSide.begin(),
                                           clusterChips_uSide.end()));

//   Crosstalk events flagged using u-side
  if (numberOfClusterChips > m_nAPVFactor) {
//    m_svdEventInfo->setCrossTalk(true);
    for (auto& svdRecoDigit : m_svdRecoDigits) {
      std::string sensorID = svdRecoDigit.getSensorID();
      std::string digitID = sensorID + "." + std::to_string(svdRecoDigit.isUStrip());
      std::string stripID = digitID + "." + std::to_string(svdRecoDigit.getCellID());
//      if (m_createCalibrationPayload) {
      if (std::find(clusterStrips_uSide.begin(), clusterStrips_uSide.end(), stripID) != clusterStrips_uSide.end()) {
        std::string sensorName;
        occupancyPDFName(svdRecoDigit.getSensorID(), svdRecoDigit.isUStrip(), sensorName);
        auto xTalkStrip = m_sensorHistograms.at(sensorName);
        //Only fill bin once
        if (xTalkStrip->GetBinContent(svdRecoDigit.getCellID()) < 1.) xTalkStrip->Fill(svdRecoDigit.getCellID(), true);
      }
      if (std::find(clusterStrips_vSide.begin(), clusterStrips_vSide.end(), stripID) != clusterStrips_vSide.end()) {
        std::string sensorName;
        occupancyPDFName(svdRecoDigit.getSensorID(), svdRecoDigit.isUStrip(), sensorName);
        auto xTalkStrip = m_sensorHistograms.at(sensorName);
        if (xTalkStrip->GetBinContent(svdRecoDigit.getCellID()) < 1.) xTalkStrip->Fill(svdRecoDigit.getCellID(), true);
      }

//      }

    }//reco digit loop
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
//            sensorOnMap->Write();

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

