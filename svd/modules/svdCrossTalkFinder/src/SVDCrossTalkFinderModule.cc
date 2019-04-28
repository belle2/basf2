/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: James Webb                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdCrossTalkFinder/SVDCrossTalkFinderModule.h>

using namespace std;
using namespace Belle2;

REG_MODULE(SVDCrossTalkFinder)

SVDCrossTalkFinderModule::SVDCrossTalkFinderModule() : Module()

{
  setDescription("Detect SVDRecoDigits created from cross-talk in the detector");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDRecoDigits", m_svdRecoDigitsName,
           "SVDRecoDigit collection name", string(""));

  addParam("inputFilePath", m_inputFilePath,
           "Path containing occupancy root file", std::string("/data/svd/occupancyMap.root"));

  addParam("uSideOccupancyFactor", m_uSideOccupancyFactor,
           "Multiple of the average occupancy for high occupancy strip classification", 2);

  addParam("vSideOccupancyFactor", m_vSideOccupancyFactor,
           "Multiple of the average occupancy for high occupancy strip classification", 2);

  addParam("nAPVFactor", m_nAPVFactor,
           "Number of APV chips with at least one high occupancy strips in event required for cross talk flag", 30);

}


void SVDCrossTalkFinderModule::initialize()
{

  if (m_inputFilePath.empty()) {
    B2ERROR("Path to occupancy file not set!");
  } else {
    std::string fullPath = FileSystem::findFile(m_inputFilePath);
    if (fullPath.empty()) {
      B2ERROR("Occupancy file:" << m_inputFilePath << "not located! Check filename input matches name of occupancy file!");
    }
    m_inputFilePath = fullPath;
  }

  m_calibrationFile = new TFile(m_inputFilePath.c_str(), "READ");

  if (!m_calibrationFile->IsOpen())
    B2FATAL("Couldn't open occupancy file:" << m_inputFilePath);

  // prepare storeArray
  m_svdRecoDigits.isRequired(m_svdRecoDigitsName);

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
  vector<std::string> clusterChips_vSide ;
  vector<int> strips_uSide;
  vector<int> strips_vSide;

  for (auto& svdRecoDigit : m_svdRecoDigits) {
    std::string sensorName = svdRecoDigit.getSensorID();
    //Remove L3 and +fw sensors, not affected by cross-talk
    if (svdRecoDigit.getSensorID().getLayerNumber() == 3 or svdRecoDigit.getSensorID().getSensorNumber() == 1) {
      continue;
    }

    int side = svdRecoDigit.isUStrip();

    std::string completeName = sensorName + "." + std::to_string(side);

    TH1F* occupancy = nullptr;
    m_calibrationFile->GetObject(completeName.c_str(), occupancy);
    double sensorAverage = 0;
    calculateAverage(occupancy, sensorAverage);
    int stripID = svdRecoDigit.getCellID();
    std::string stripNum = completeName + "." + std::to_string(stripID);
    //Cluster only works assuming digits are ordered.//

    if (side == 1 && occupancy->GetBinContent(stripID) > (m_uSideOccupancyFactor * sensorAverage)) {

      int adjacentStrip = 0;
      if (!strips_uSide.empty()) {
        adjacentStrip = stripID - strips_uSide.back();
      }
      strips_uSide.push_back(stripID);
      if (!highOccChips_uSide.empty() && completeName == highOccChips_uSide.back() && adjacentStrip == 1) {
        clusterChips_uSide.push_back(completeName);
        if (clusterStrips_uSide.empty() || clusterStrips_uSide.back() != stripNum) {
          clusterStrips_uSide.push_back(highOccChipsStripNum_uSide.back());
        }
        clusterStrips_uSide.push_back(stripNum);
      }
      highOccChipsStripNum_uSide.push_back(stripNum);
      highOccChips_uSide.push_back(completeName);
    }

    if (side == 0 && occupancy->GetBinContent(stripID) > (m_vSideOccupancyFactor * sensorAverage)) {
      int adjacentStrip = 0;
      if (!strips_vSide.empty()) {
        adjacentStrip = stripID - strips_vSide.back();
      }
      strips_vSide.push_back(stripID);
      if (!highOccChips_vSide.empty() && completeName == highOccChips_vSide.back() && adjacentStrip == 1) {
        if (clusterStrips_vSide.empty() || clusterStrips_vSide.back() != stripNum) {
          clusterStrips_vSide.push_back(highOccChipsStripNum_vSide.back());
        }
        clusterStrips_vSide.push_back(stripNum);
      }
      highOccChipsStripNum_vSide.push_back(stripNum);
      highOccChips_vSide.push_back(completeName);
    }

  }

  std::sort(clusterChips_uSide.begin(), clusterChips_uSide.end());
  clusterChips_uSide.erase(unique(clusterChips_uSide.begin(), clusterChips_uSide.end()), clusterChips_uSide.end());
  int numberOfClusterChips = std::distance(clusterChips_uSide.begin(), std::unique(clusterChips_uSide.begin(),
                                           clusterChips_uSide.end()));

//   Crosstalk events flagged using u-side
  if (numberOfClusterChips > m_nAPVFactor) {
    for (auto& svdRecoDigit : m_svdRecoDigits) {
      std::string sensorID = svdRecoDigit.getSensorID();
      std::string digitID = sensorID + "." + std::to_string(svdRecoDigit.isUStrip());
      std::string stripID = digitID + "." + std::to_string(svdRecoDigit.getCellID());
      if (std::find(clusterStrips_uSide.begin(), clusterStrips_uSide.end(), stripID) != clusterStrips_uSide.end()) {
        svdRecoDigit.setCrossTalkEventFlag(true);
      }

      if (std::find(clusterStrips_vSide.begin(), clusterStrips_vSide.end(), stripID) != clusterStrips_vSide.end()) {
        svdRecoDigit.setCrossTalkEventFlag(true);
      }

    }//reco digit loop
  }

}

void SVDCrossTalkFinderModule::terminate()
{
  B2INFO("SVDCrossTalkFinderModule::terminate");

  m_calibrationFile->Delete();
}


void SVDCrossTalkFinderModule::calculateAverage(TH1F* occupancyHist, double& mean)
{
  double nBins = occupancyHist->GetXaxis()->GetNbins();
  double count = 0;
  for (int i = 0; i < nBins; i++) {
    count += occupancyHist->GetBinContent(i);
  }
  mean /= nBins;
}
