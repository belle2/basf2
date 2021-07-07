/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdCrossTalkFinder/SVDCrossTalkFinderModule.h>
#include <svd/modules/svdCrossTalkFinder/SVDCrossTalkFinderHelperFunctions.h>

#include <vxd/geometry/GeoCache.h>

using namespace std;
using namespace Belle2;

REG_MODULE(SVDCrossTalkFinder)

SVDCrossTalkFinderModule::SVDCrossTalkFinderModule() : Module()

{
  setDescription("Detect SVDRecoDigits created from cross-talk present in the origami sensors");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDRecoDigits", m_svdRecoDigitsName,
           "SVDRecoDigit collection name", string(""));

  addParam("createCalibrationPayload", m_createCalibrationPayload,
           "Create cross-talk strip channel payload", true);

  addParam("outputFilename", m_outputFilename,
           "Filename of root file for calibration payload", std::string("crossTalkStripsCalibration.root"));

  addParam("uSideOccupancyFactor", m_uSideOccupancyFactor,
           "Multiple of the average occupancy for high occupancy strip classification", 2);

  addParam("vSideOccupancyFactor", m_vSideOccupancyFactor,
           "Multiple of the average occupancy for high occupancy strip classification", 2);

  addParam("nAPVFactor", m_nAPVFactor,
           "Number of APV chips with at least one high occupancy strips in event required for cross talk flag", 30);

}


void SVDCrossTalkFinderModule::initialize()
{

  if (m_createCalibrationPayload) {
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

  // prepare storeArray
  m_svdRecoDigits.isRequired(m_svdRecoDigitsName);
  m_svdEventInfo.isRequired();

}

void SVDCrossTalkFinderModule::event()
{

  vector<std::string> highOccChips_uSide ;
  vector<std::string> highOccChips_vSide ;
  vector<std::string> highOccChipsStripNum_uSide ;
  vector<std::string> highOccChipsStripNum_vSide ;
  vector<std::string> clusterStrips_uSide ;
  vector<std::string> clusterStrips_vSide ;
  vector<std::string> clusterChips_uSide ;
  vector<int> strips_uSide;
  vector<int> strips_vSide;

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

  }

  std::sort(clusterChips_uSide.begin(), clusterChips_uSide.end());
  clusterChips_uSide.erase(unique(clusterChips_uSide.begin(), clusterChips_uSide.end()), clusterChips_uSide.end());
  int numberOfClusterChips = std::distance(clusterChips_uSide.begin(), std::unique(clusterChips_uSide.begin(),
                                           clusterChips_uSide.end()));

//   Crosstalk events flagged using u-side
  if (numberOfClusterChips > m_nAPVFactor) {
    m_svdEventInfo->setCrossTalk(true);
    for (auto& svdRecoDigit : m_svdRecoDigits) {
      std::string sensorID = svdRecoDigit.getSensorID();
      std::string digitID = sensorID + "." + std::to_string(svdRecoDigit.isUStrip());
      std::string stripID = digitID + "." + std::to_string(svdRecoDigit.getCellID());
      if (m_createCalibrationPayload) {
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

      }

    }//reco digit loop
  }

}

void SVDCrossTalkFinderModule::terminate()
{
  B2INFO("SVDCrossTalkFinderModule::terminate");

  if (m_createCalibrationPayload) {
    m_histogramFile = new TFile(m_outputFilename.c_str(), "RECREATE");
    m_histogramFile->cd();
    std::string sensorName;
    VXD::GeoCache& geo = VXD::GeoCache::getInstance();
    for (auto& layers : geo.getLayers(VXD::SensorInfoBase::SVD)) {
      for (auto& ladders : geo.getLadders(layers)) {
        for (auto& sensors : geo.getSensors(ladders)) {
          for (int side = 0; side <= 1; side++) {

            occupancyPDFName(sensors, side, sensorName);
            auto sensorOnMap = m_sensorHistograms.at(sensorName);
            sensorOnMap->Write();

          }
        }
      }
    }
    m_histogramFile->Close();
  }
}


void SVDCrossTalkFinderModule::calculateAverage(const VxdID& sensorID, double& mean, int side)
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
