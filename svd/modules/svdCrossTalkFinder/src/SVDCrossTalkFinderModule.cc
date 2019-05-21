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
#include <svd/modules/svdCrossTalkFinder/SVDCrossTalkFinderHelperFunctions.h>

using namespace std;
using namespace Belle2;

REG_MODULE(SVDCrossTalkFinder)

SVDCrossTalkFinderModule::SVDCrossTalkFinderModule() : Module()

{
  setDescription("Detect SVDRecoDigits created from cross-talk present in the origami sensors");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDRecoDigits", m_svdRecoDigitsName,
           "SVDRecoDigit collection name", string(""));

  addParam("readFromDB", m_readFromDB,
           "Read occupancy file from the databse", bool(true));

  addParam("inputFilePath", m_inputFilePath,
           "Path containing occupancy root file", std::string("/data/svd/SVDOccupancyMap.root"));

  addParam("occupancyInputFile", m_occupancyInputFile,
           "Name of the occupancy root file", std::string("SVDOccupancyMap.root"));

  addParam("uSideOccupancyFactor", m_uSideOccupancyFactor,
           "Multiple of the average occupancy for high occupancy strip classification", 2);

  addParam("vSideOccupancyFactor", m_vSideOccupancyFactor,
           "Multiple of the average occupancy for high occupancy strip classification", 2);

  addParam("nAPVFactor", m_nAPVFactor,
           "Number of APV chips with at least one high occupancy strips in event required for cross talk flag", 30);

}


void SVDCrossTalkFinderModule::initialize()
{

  std::string rootFile;
  if (m_readFromDB) {
    B2DEBUG(1, "SVDCrossTalkFinderModule: Retrieving occupancy file from DB. Filename: " << m_occupancyInputFile.c_str());
    m_ptrDBObjPtr = new DBObjPtr<PayloadFile>(m_occupancyInputFile.c_str());
    if (m_ptrDBObjPtr == nullptr) B2FATAL("SectorMapBootstrapModule: the DBObjPtr is not initialized");

    if (!(*m_ptrDBObjPtr).isValid()) B2FATAL("SVDCrossTalkFinderModule the DB object is not valid!");

    rootFile = (*m_ptrDBObjPtr)->getFileName();
    m_calibrationFile = new TFile(rootFile.c_str(), "READ");

    if (!m_calibrationFile->IsOpen())
      B2FATAL("Couldn't open occupancy file:" << m_inputFilePath);

  }

  else {
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
  }
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
    //Remove L3 and +fw sensors, not affected by cross-talk
    if (svdRecoDigit.getSensorID().getLayerNumber() == 3 or svdRecoDigit.getSensorID().getSensorNumber() == 1) {
      continue;
    }

    int side = svdRecoDigit.isUStrip();

    std::string sensorName;
    occupancyPDFName(svdRecoDigit.getSensorID(), side, sensorName);


    TH1F* occupancy = nullptr;
    m_calibrationFile->GetObject(sensorName.c_str(), occupancy);
    double sensorAverage = 0;
    calculateAverage(occupancy, sensorAverage);
    int stripID = svdRecoDigit.getCellID();
    std::string sensorStripNum = sensorName + "." + std::to_string(stripID);
    //Clustering only works assuming digits are ordered.//

    if (side == 1 && occupancy->GetBinContent(stripID) > (m_uSideOccupancyFactor * sensorAverage)) {

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

    if (side == 0 && occupancy->GetBinContent(stripID) > (m_vSideOccupancyFactor * sensorAverage)) {
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
