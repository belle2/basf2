/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdCalibration/SVDCalibrationsMonitorModule.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDCalibrationsMonitor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDCalibrationsMonitorModule::SVDCalibrationsMonitorModule() : Module()
{
  // Set module properties
  setDescription("Module to produce a list of histograms showing the uploaded calibration constants");

  // Parameter definitions
  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDCalibrationMonitor_output.root"));
}

void SVDCalibrationsMonitorModule::initialize()
{


  //  for (int i = 0; i < m_maxLayers; i++) {
  m_histoList_noise = new TList;
  m_histoList_noiseInElectrons = new TList;
  //}

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  TString NameOfHisto = "";
  TString TitleOfHisto = "";

  //call for a geometry instance
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  std::set<Belle2::VxdID> svdLayers = aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();

  while ((itSvdLayers != svdLayers.end()) && (itSvdLayers->getLayerNumber() != 7)) { //loop on Layers

    std::set<Belle2::VxdID> svdLadders = aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) { //loop on Ladders

      std::set<Belle2::VxdID> svdSensors = aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();
      B2DEBUG(1, "    svd sensor info " << * (svdSensors.begin()));

      while (itSvdSensors != svdSensors.end()) { //loop on sensors
        B2DEBUG(1, "    svd sensor info " << *itSvdSensors);

        for (int side = 0; side < 2; side++) {
          //create your histogram
          int layer = itSvdSensors->getLayerNumber();
          int ladder =  itSvdSensors->getLadderNumber();
          int sensor = itSvdSensors->getSensorNumber();

          TString nameLayer = "";
          nameLayer += layer;

          TString nameLadder = "";
          nameLadder += ladder;

          TString nameSensor = "";
          nameSensor += sensor;

          TString nameSide = "";
          if (side == 1)
            nameSide = "U";
          else if (side == 0)
            nameSide = "V";

          ///NOISES

          NameOfHisto = "noise_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;

          TitleOfHisto = "strip noise (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" + nameSensor + "," + nameSide + " side)";

          h_noise[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 40, -0.5, 9.5, "strip noise(ADC)",
                                                                   m_histoList_noise);

          h_noiseInElectrons[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 400, 199.5, 1199.5,
                                                            "strip noise(electron charge)", m_histoList_noiseInElectrons);
        }
        //histogram created
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }



}

void SVDCalibrationsMonitorModule::beginRun()
{
}

void SVDCalibrationsMonitorModule::event()
{

  //call for a geometry instance
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  std::set<Belle2::VxdID> svdLayers = aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();

  while ((itSvdLayers != svdLayers.end()) && (itSvdLayers->getLayerNumber() != 7)) { //loop on Layers

    std::set<Belle2::VxdID> svdLadders = aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) { //loop on Ladders

      std::set<Belle2::VxdID> svdSensors = aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();
      B2DEBUG(1, "    svd sensor info " << * (svdSensors.begin()));

      while (itSvdSensors != svdSensors.end()) { //loop on sensors
        B2DEBUG(1, "    svd sensor info " << *itSvdSensors);

        int layer = itSvdSensors->getLayerNumber();
        int ladder =  itSvdSensors->getLadderNumber();
        int sensor = itSvdSensors->getSensorNumber();
        Belle2::VxdID theVxdID(layer, ladder, sensor);
        const SVD::SensorInfo* currentSensorInfo = dynamic_cast<const SVD::SensorInfo*>(&VXD::GeoCache::get(theVxdID));
        for (int Ustrip = 0; Ustrip < currentSensorInfo->getUCells(); Ustrip++) {
          //fill your histogram for U side



          float ADCNoise = m_NoiseCal.getNoise(theVxdID, 1, Ustrip);

          double noiseInElectrons = m_NoiseCal.getNoiseInElectrons(theVxdID, 1, Ustrip);

          h_noise[layer][ladder][sensor][1]->Fill(ADCNoise);
          h_noiseInElectrons[layer][ladder][sensor][1]->Fill(noiseInElectrons);

        } //histogram filled for U side

        for (int Vstrip = 0; Vstrip < currentSensorInfo->getVCells(); Vstrip++) {
          //fill your histogram for V side



          float ADCNoise = m_NoiseCal.getNoise(theVxdID, 0, Vstrip);

          double noiseInElectrons = m_NoiseCal.getNoiseInElectrons(theVxdID, 0, Vstrip);

          h_noise[layer][ladder][sensor][0]->Fill(ADCNoise);
          h_noiseInElectrons[layer][ladder][sensor][0]->Fill(noiseInElectrons);

        } //histogram filled for V side

        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }

}

void SVDCalibrationsMonitorModule::endRun()
{
}

void SVDCalibrationsMonitorModule::terminate()
{
  TObject* obj;
  if (m_rootFilePtr != NULL) {

    //writing the histrogram list for the noises in ADC units
    m_rootFilePtr->mkdir("noise_ADCunits");
    m_rootFilePtr->cd("noise_ADCunits");

    TIter nextH_noise(m_histoList_noise);
    while ((obj = nextH_noise())) {
      obj->Print();
      obj->Write();
    }

    //writing the histrogram list for the noises in electron charge
    m_rootFilePtr->mkdir("noise_electronsCharge");
    m_rootFilePtr->cd("noise_electronsCharge");
    TIter nextH_noiseInElectrons(m_histoList_noiseInElectrons);
    while ((obj = nextH_noiseInElectrons())) {
      obj->Print();
      obj->Write();
    }

    m_rootFilePtr->Close();
    B2INFO("The rootfile containing the list of histrograms has been filled and closed.");
  }
}


TH1F*  SVDCalibrationsMonitorModule::createHistogram1D(const char* name, const char* title,
                                                       Int_t nbins, Double_t min, Double_t max,
                                                       const char* xtitle, TList* histoList)
{

  TH1F* h = new TH1F(name, title, nbins, min, max);

  h->GetXaxis()->SetTitle(xtitle);

  if (histoList)
    histoList->Add(h);

  return h;
}
