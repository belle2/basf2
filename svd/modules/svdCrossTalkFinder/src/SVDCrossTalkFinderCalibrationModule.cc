/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: James Webb                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdCrossTalkFinder/SVDCrossTalkFinderCalibrationModule.h>
#include <svd/modules/svdCrossTalkFinder/SVDCrossTalkFinderHelperFunctions.h>


using namespace std;
using namespace Belle2;

REG_MODULE(SVDCrossTalkFinderCalibration)

SVDCrossTalkFinderCalibrationModule::SVDCrossTalkFinderCalibrationModule() : Module()

{
  setDescription("Generate occupancy map required for crossTalkFinder module");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDRecoDigits", m_svdRecoDigitsName,
           "SVDRecoDigit collection name", string(""));

  addParam("occupancyOutputFile", m_occupancyOutputFile,
           "Name of the occupancy root file", std::string("SVDOccupancyMap.root"));


}

void SVDCrossTalkFinderCalibrationModule::initialize()
{

  m_svdRecoDigits.isRequired(m_svdRecoDigitsName);

  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (auto& layers : geo.getLayers(VXD::SensorInfoBase::SVD)) {
    for (auto& ladders : geo.getLadders(layers)) {
      for (auto& sensors : geo.getSensors(ladders)) {
        for (int side = 0; side <= 1; side++) {
          std::string sensorName;
          occupancyPDFName(sensors, side, sensorName);
          int layerInt = sensors.getLayerNumber();
          if (histMap.count(sensorName) == 0) {
            if (layerInt != 3 && side == 0) {
              TH1F* sigHist  = new TH1F(sensorName.c_str(), "", 512, 0, 511);
              histMap[sensorName] = sigHist;
            } else {
              TH1F* sigHist  = new TH1F(sensorName.c_str(), "", 768, 0, 767);
              histMap[sensorName] = sigHist;

            }
          }

        }
      }
    }

  }
  nEvents = 0;

}
void SVDCrossTalkFinderCalibrationModule::event()
{

  nEvents++;

  for (auto& recoDigit : m_svdRecoDigits) {
    const VxdID& sensor = recoDigit.getSensorID();
    int side = recoDigit.isUStrip();
    int strip = recoDigit.getCellID();
    std::string sensorName;
    occupancyPDFName(sensor, side, sensorName);

    auto hist = histMap.at(sensorName);
    hist->Fill(strip);
  }


}
void SVDCrossTalkFinderCalibrationModule::terminate()
{
  //Open rootfile
  TFile* f = new TFile(m_occupancyOutputFile.c_str(), "RECREATE");
  f->cd();

  std::vector<std::string> usedSensors; //Store names to avoid double counting

  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (auto& layers : geo.getLayers(VXD::SensorInfoBase::SVD)) {
    for (auto& ladders : geo.getLadders(layers)) {
      for (auto& sensors : geo.getSensors(ladders)) {
        for (int side = 0; side <= 1; side++) {
          std::string sensorName;
          occupancyPDFName(sensors, side, sensorName);

          if (std::find(usedSensors.begin(), usedSensors.end(), sensorName.c_str()) == usedSensors.end()) {
            usedSensors.push_back(sensorName);
            auto hist = histMap.at(sensorName);
            hist->Scale(1. / nEvents);
            hist->Write();
          }
        }
      }
    }
  }
  f->Close();
}
