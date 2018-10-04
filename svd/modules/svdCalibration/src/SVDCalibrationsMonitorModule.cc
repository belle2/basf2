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
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

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
  m_histoList_gainInElectrons = new TList;
  m_histoList_peakTime = new TList;
  m_histoList_pulseWidth = new TList;
  m_histoList_timeshift = new TList;
  m_histoList_triggerbin = new TList;
  //}

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  //tree initialization
  m_tree = new TTree("calib", "RECREATE");
  b_run = m_tree->Branch("run", &m_run, "run/i");
  b_layer = m_tree->Branch("layer", &m_layer, "layer/i");
  b_ladder = m_tree->Branch("ladder", &m_ladder, "ladder/i");
  b_sensor = m_tree->Branch("sensor", &m_sensor, "sensor/i");
  b_side = m_tree->Branch("side", &m_side, "side/i");
  b_noiseAVE = m_tree->Branch("noiseAVE", &m_noiseAVE, "noiseAVE/F");
  b_noiseRMS = m_tree->Branch("noiseRMS", &m_noiseRMS, "noiseRMS/F");
  b_gainAVE = m_tree->Branch("gainAVE", &m_gainAVE, "gainAVE/F");
  b_gainRMS = m_tree->Branch("gainRMS", &m_gainRMS, "gainRMS/F");
  b_peakTimeAVE = m_tree->Branch("peakTimeAVE", &m_peakTimeAVE, "peakTimeAVE/F");
  b_peakTimeRMS = m_tree->Branch("peakTimeRMS", &m_peakTimeRMS, "peakTimeRMS/F");
  b_pulseWidthAVE = m_tree->Branch("pulseWidthAVE", &m_pulseWidthAVE, "pulseWidthAVE/F");
  b_pulseWidthRMS = m_tree->Branch("pulseWidthRMS", &m_pulseWidthRMS, "pulseWidthRMS/F");



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

          NameOfHisto = "noiseADC_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "strip noise (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" + nameSensor + "," + nameSide + " side)";
          h_noise[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 80, -0.5, 9.5, "strip noise (ADC)",
                                                                   m_histoList_noise);

          NameOfHisto = "noiseELEC_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "strip noise (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" + nameSensor + "," + nameSide + " side)";
          h_noiseInElectrons[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 600, 199.5, 1499.5,
                                                            "strip noise (e-)", m_histoList_noiseInElectrons);


          // GAIN
          NameOfHisto = "gainInElectrons_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "Gain (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" + nameSensor + "," + nameSide + " side)";
          h_gainInElectrons[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 600, 0., 600,
                                                           "Gain (electron charge)",
                                                           m_histoList_gainInElectrons);

          //PEAK TIME
          NameOfHisto = "peakTime_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "Peak time (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" + nameSensor + "," + nameSide + " side)";
          h_peakTime[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 255, -0.5, 254.5, "Peak Time (ns)",
                                                                      m_histoList_peakTime);


          //PULSE WIDTH
          NameOfHisto = "pulseWidth_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "Pulse width (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" + nameSensor + "," + nameSide + " side)";
          h_pulseWidth[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 255, -0.5, 254.5, "Pulse width (ns)",
                                                      m_histoList_pulseWidth);

          //CoG TIME SHIFT
          NameOfHisto = "CoG_ShiftMeanToZero" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "CoG_ShiftMeanToZero (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" + nameSensor + "," + nameSide +
                         " side)";
          h_timeshift[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 255, -0.5, 254.5,
                                                     "CoG_ShiftMeanToZero (ns)",
                                                     m_histoList_timeshift);
          //CoG TRIGGER BIN CORRECTION
          NameOfHisto = "CoG_ShiftMeanToZeroTBDep" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "CoG_ShiftMeanToZeroTBDep (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" + nameSensor + "," + nameSide +
                         " side)";
          h_triggerbin[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 255, -0.5, 254.5,
                                                      "CoG_ShiftMeanToZeroTBDep (ns)",
                                                      m_histoList_triggerbin);

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
  if (!m_NoiseCal.isValid())
    B2WARNING("No valid SVDNoiseCalibration for the requested IoV");
  if (! m_PulseShapeCal.isValid())
    B2WARNING("No valid SVDPulseShapeCalibrations for the requested IoV");
  /*  if(!m_PedCal.isValid())
    B2WARNING("No valid SVDPedestalCalibrations for the requested IoV");
  if(!m_OccCal.isValid())
    B2WARNING("No valid SVDOccupancyCalibrations for the requested IoV");
  if(!m_HotStripsCal.isValid())
    B2WARNING("No valid SVDHotStripsCalibrations for the requested IoV");
  */

}

void SVDCalibrationsMonitorModule::event()
{

  StoreObjPtr<EventMetaData> meta;
  m_run = meta->getRun();

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



          float ADCnoise = m_NoiseCal.getNoise(theVxdID, 1, Ustrip);
          double noiseInElectrons = m_NoiseCal.getNoiseInElectrons(theVxdID, 1, Ustrip);

          float ELECgain = m_PulseShapeCal.getChargeFromADC(theVxdID, 1, Ustrip, 1);

          float time = m_PulseShapeCal.getPeakTime(theVxdID, 1, Ustrip);
          float width =  m_PulseShapeCal.getWidth(theVxdID, 1, Ustrip);
          float time_shift = m_PulseShapeCal.getTimeShiftCorrection(theVxdID, 1, Ustrip);

          float triggerbin_shift = m_PulseShapeCal.getTriggerBinDependentCorrection(theVxdID, 1, Ustrip,
                                   0); /*reading by default the trigger bin #0*/

          h_noise[layer][ladder][sensor][1]->Fill(ADCnoise);
          h_noiseInElectrons[layer][ladder][sensor][1]->Fill(noiseInElectrons);
          h_gainInElectrons[layer][ladder][sensor][1]->Fill(ELECgain);
          h_peakTime[layer][ladder][sensor][1]->Fill(time);
          h_pulseWidth[layer][ladder][sensor][1]->Fill(width);
          h_timeshift[layer][ladder][sensor][1]->Fill(time_shift);
          h_triggerbin[layer][ladder][sensor][1]->Fill(triggerbin_shift);

        } //histogram filled for U side

        for (int Vstrip = 0; Vstrip < currentSensorInfo->getVCells(); Vstrip++) {
          //fill your histogram for V side



          float ADCnoise = m_NoiseCal.getNoise(theVxdID, 0, Vstrip);

          double noiseInElectrons = m_NoiseCal.getNoiseInElectrons(theVxdID, 0, Vstrip);

          float ELECgain = m_PulseShapeCal.getChargeFromADC(theVxdID, 0, Vstrip, 1);

          float time = m_PulseShapeCal.getPeakTime(theVxdID, 0, Vstrip);
          float width =  m_PulseShapeCal.getWidth(theVxdID, 0, Vstrip);
          float time_shift = m_PulseShapeCal.getTimeShiftCorrection(theVxdID, 0, Vstrip);
          float triggerbin_shift = m_PulseShapeCal.getTriggerBinDependentCorrection(theVxdID, 0, Vstrip,
                                   0); /*reading by default the trigger bin #0*/


          h_noise[layer][ladder][sensor][0]->Fill(ADCnoise);
          h_noiseInElectrons[layer][ladder][sensor][0]->Fill(noiseInElectrons);
          h_gainInElectrons[layer][ladder][sensor][0]->Fill(ELECgain);
          h_peakTime[layer][ladder][sensor][0]->Fill(time);
          h_pulseWidth[layer][ladder][sensor][0]->Fill(width);
          h_timeshift[layer][ladder][sensor][0]->Fill(time_shift);
          h_triggerbin[layer][ladder][sensor][0]->Fill(triggerbin_shift);

        } //histogram filled for V side

        m_layer = layer;
        m_ladder = ladder;
        m_sensor = sensor;
        for (int s = 0; s < 2; s++) {
          m_side = s;
          m_noiseAVE = h_noise[layer][ladder][sensor][s]->GetMean();
          m_noiseRMS = h_noise[layer][ladder][sensor][s]->GetRMS();
          m_gainAVE = h_gainInElectrons[layer][ladder][sensor][s]->GetMean();
          m_gainRMS = h_gainInElectrons[layer][ladder][sensor][s]->GetRMS();
          m_peakTimeAVE = h_peakTime[layer][ladder][sensor][s]->GetMean();
          m_peakTimeRMS = h_peakTime[layer][ladder][sensor][s]->GetRMS();
          m_pulseWidthAVE = h_pulseWidth[layer][ladder][sensor][s]->GetMean();
          m_pulseWidthRMS = h_pulseWidth[layer][ladder][sensor][s]->GetRMS();


          m_tree->Fill();
        }
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }

}

void SVDCalibrationsMonitorModule::endRun()
{
  B2RESULT("******************************************");
  B2RESULT("** UNIQUE IDs of calibration DB objects **");
  B2RESULT("");
  B2RESULT("   - SVDNoiseCalibrations:" << m_NoiseCal.getUniqueID());
  B2RESULT("   - SVDPulseShapeCalibrations:" << m_PulseShapeCal.getUniqueID());
}

void SVDCalibrationsMonitorModule::terminate()
{
  TObject* obj;
  if (m_rootFilePtr != NULL) {

    //write the tree
    m_tree->Write();

    //writing the histogram list for the noises in ADC units
    m_rootFilePtr->mkdir("noise_ADCunits");
    m_rootFilePtr->cd("noise_ADCunits");

    TIter nextH_noise(m_histoList_noise);
    while ((obj = nextH_noise()))
      obj->Write();


    //writing the histogram list for the noises in electron charge
    m_rootFilePtr->mkdir("noise_electronsCharge");
    m_rootFilePtr->cd("noise_electronsCharge");
    TIter nextH_noiseInElectrons(m_histoList_noiseInElectrons);
    while ((obj = nextH_noiseInElectrons()))
      obj->Write();



    //writing the histogram list for the gains in electron charge
    m_rootFilePtr->mkdir("gain_electronsCharge");
    m_rootFilePtr->cd("gain_electronsCharge");
    TIter nextH_gainInElectrons(m_histoList_gainInElectrons);
    while ((obj = nextH_gainInElectrons()))
      obj->Write();


    //writing the histogram list for the peak times in ns
    m_rootFilePtr->mkdir("peakTime");
    m_rootFilePtr->cd("peakTime");

    TIter nextH_peakTime(m_histoList_peakTime);
    while ((obj = nextH_peakTime()))
      obj->Write();

    //writing the histogram list for the pulse widths in ns
    m_rootFilePtr->mkdir("pulseWidth");
    m_rootFilePtr->cd("pulseWidth");

    TIter nextH_width(m_histoList_pulseWidth);
    while ((obj = nextH_width()))
      obj->Write();

    //writing the histogram list for the time shift correction in ns
    m_rootFilePtr->mkdir("CoG_ShiftMeanToZero");
    m_rootFilePtr->cd("CoG_ShiftMeanToZero");

    TIter nextH_timeshift(m_histoList_timeshift);
    while ((obj = nextH_timeshift()))
      obj->Write();

    //writing the histogram list for the trigger bin correction in ns
    m_rootFilePtr->mkdir("CoG_ShiftMeanToZeroTBDep");
    m_rootFilePtr->cd("CoG_ShiftMeanToZeroTBDep");

    TIter nextH_triggerbin(m_histoList_triggerbin);
    while ((obj = nextH_triggerbin()))
      obj->Write();



    m_rootFilePtr->Close();
    B2RESULT("The rootfile containing the list of histograms has been filled and closed.");


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
