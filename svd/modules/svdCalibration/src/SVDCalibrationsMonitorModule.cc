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


  m_histoList_timeshift = new TList;
  m_histoList_triggerbin = new TList;
  m_histoList_cluster = new TList;

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

  m_treeDetailed = new TTree("calibDetailed", "RECREATE");
  b_run = m_treeDetailed->Branch("run", &m_run, "run/i");
  b_layer = m_treeDetailed->Branch("layer", &m_layer, "layer/i");
  b_ladder = m_treeDetailed->Branch("ladder", &m_ladder, "ladder/i");
  b_sensor = m_treeDetailed->Branch("sensor", &m_sensor, "sensor/i");
  b_side = m_treeDetailed->Branch("side", &m_side, "side/i");
  b_strip = m_treeDetailed->Branch("strip", &m_strip, "strip/i");
  b_mask = m_treeDetailed->Branch("mask", &m_mask, "mask/F");
  b_noise = m_treeDetailed->Branch("noise", &m_noise, "noise/F");
  b_noiseEl = m_treeDetailed->Branch("noiseEl", &m_noiseEl, "noiseEl/F");
  b_gain = m_treeDetailed->Branch("gain", &m_gain, "gain/F");
  b_pedestal = m_treeDetailed->Branch("pedestal", &m_pedestal, "pedestal/F");
  b_peakTime = m_treeDetailed->Branch("peakTime", &m_peakTime, "peakTime/F");
  b_pulseWidth = m_treeDetailed->Branch("pulseWidth", &m_pulseWidth, "pulseWidth/F");

  TString NameOfHisto = "";
  TString TitleOfHisto = "";
  TString NameOfProf = "";
  TString TitleOfProf = "";

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



          //------ OFFLINE CALIBRATION CONSTANTS ------

          //CoG TIME SHIFT
          NameOfHisto = "CoG_ShiftMeanToZero_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "CoG_ShiftMeanToZero (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" + nameSensor + "," + nameSide +
                         " side)";
          h_timeshift[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 255, -0.5, 254.5,
                                                     "CoG_ShiftMeanToZero (ns)",
                                                     m_histoList_timeshift);
          //CoG TRIGGER BIN CORRECTION
          NameOfHisto = "CoG_ShiftMeanToZeroTBDep_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "CoG_ShiftMeanToZeroTBDep (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" + nameSensor + "," + nameSide +
                         " side)";
          h_triggerbin[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 255, -0.5, 254.5,
                                                      "CoG_ShiftMeanToZeroTBDep (ns)",
                                                      m_histoList_triggerbin);

          //CLUSTER SNR
          NameOfHisto = "cls_ClusterSNR_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "Cluster Minimum SNR (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" + nameSensor + "," + nameSide +
                         " side)";
          h_clsSNR[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 100, -0.5, 99.5,
                                                                    "cls min SNR",
                                                                    m_histoList_cluster);

          //CLUSTER Seed SNR
          NameOfHisto = "cls_SeedSNR_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "Cluster Seed SNR (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" + nameSensor + "," + nameSide +
                         " side)";
          h_clsSeedSNR[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 100, -0.5, 99.5,
                                                      "cls seed SNR",
                                                      m_histoList_cluster);
          //CLUSTER Adj SNR
          NameOfHisto = "cls_AdjSNR_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "Cluster Adj SNR (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" + nameSensor + "," + nameSide +
                         " side)";
          h_clsAdjSNR[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 100, -0.5, 99.5,
                                                     "cls seed SNR",
                                                     m_histoList_cluster);

          //CLUSTER Scale Error size 1
          NameOfHisto = "cls_ScaleErr1_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "Cluster Position Error Scale Factor for Size 1 (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" +
                         nameSensor + "," + nameSide +
                         " side)";
          h_clsScaleErr1[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 100, 0, 10,
                                                        "scale factor",
                                                        m_histoList_cluster);
          //CLUSTER Scale Error size 2
          NameOfHisto = "cls_ScaleErr2_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "Cluster Position Error Scale Factor for Size 2 (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" +
                         nameSensor + "," + nameSide +
                         " side)";
          h_clsScaleErr2[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 100, 0, 10,
                                                        "scale factor",
                                                        m_histoList_cluster);
          //CLUSTER Scale Error size > 2
          NameOfHisto = "cls_ScaleErr3_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "Cluster Position Error Scale Factor for Size > 2 (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" +
                         nameSensor + "," + nameSide +
                         " side)";
          h_clsScaleErr3[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 100, 0, 10,
                                                        "scale factor",
                                                        m_histoList_cluster);



          //CLUSTER Time Selection Function
          NameOfHisto = "cls_ClusterTimeSelFunction_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "Cluster Time Selection Function Version(Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" + nameSensor + ","
                         + nameSide +
                         " side)";
          h_clsTimeFuncVersion[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 5, -0.5, 4.5,
                                                              "cls time Sel Function ID", m_histoList_cluster);

          //CLUSTER Time
          NameOfHisto = "cls_ClusterTime_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "Cluster Minimum Time (Layer" + nameLayer + ", Ladder" + nameLadder + ", sensor" + nameSensor + "," + nameSide +
                         " side)";
          h_clsTimeMin[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 200, -100, 100,
                                                      "cls min Time",
                                                      m_histoList_cluster);


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
  if (!m_MaskedStr.isValid())
    B2WARNING("No valid SVDFADCMaskedStrip for the requested IoV");
  if (!m_NoiseCal.isValid())
    B2WARNING("No valid SVDNoiseCalibration for the requested IoV");
  if (!m_PedestalCal.isValid())
    B2WARNING("No valid SVDPedestalCalibration for the requested IoV");
  if (! m_PulseShapeCal.isValid())
    B2WARNING("No valid SVDPulseShapeCalibrations for the requested IoV");
  if (! m_ClusterCal.isValid())
    B2WARNING("No valid SVDClusterCalibrations for the requested IoV");
  /*
  if(!m_OccCal.isValid())
    B2WARNING("No valid SVDOccupancyCalibrations for the requested IoV");
  if(!m_HotStripsCal.isValid())
    B2WARNING("No valid SVDHotStripsCalibrations for the requested IoV");
  */


  ///MASKS
  TH1F hMask("masked_L@layerL@ladderS@sensor@view",
             "masked strip in @layer.@ladder.@sensor @view/@side",
             2, -0.5, 1.5);
  hMask.GetXaxis()->SetTitle("isMasked");
  m_hMask = new SVDHistograms<TH1F>(hMask);

  TH2F h2Mask_512("masked2D_512_L@layerL@ladderS@sensor@view",
                  "masked strip in @layer.@ladder.@sensor @view/@side VS strip number",
                  128 * 4, -0.5, 128 * 4 - 0.5, 2, -0.5, 1.5);
  h2Mask_512.GetYaxis()->SetTitle("isMasked");
  h2Mask_512.GetXaxis()->SetTitle("cellID");

  TH2F h2Mask_768("masked2D_768_L@layerL@ladderS@sensor@view",
                  "masked strip in @layer.@ladder.@sensor @view/@side VS strip number",
                  128 * 6, -0.5, 128 * 6 - 0.5, 2, -0.5, 1.5);
  h2Mask_768.GetYaxis()->SetTitle("isMasked");
  h2Mask_768.GetXaxis()->SetTitle("cellID");

  m_h2Mask = new SVDHistograms<TH2F>(h2Mask_768, h2Mask_768, h2Mask_768, h2Mask_512);

  ///NOISE ADC
  TH1F hNoise("noiseADC_L@layerL@ladderS@sensor@view",
              "noise in ADC in @layer.@ladder.@sensor @view/@side",
              80, -0.5, 9.5);
  hNoise.GetXaxis()->SetTitle("strip noise (ADC)");
  m_hNoise = new SVDHistograms<TH1F>(hNoise);

  TH2F h2Noise_512("noise2D_512_L@layerL@ladderS@sensor@view",
                   "noise in ADC in @layer.@ladder.@sensor @view/@side VS strip number",
                   128 * 4, -0.5, 128 * 4 - 0.5, 80, -0.5, 9.5);
  h2Noise_512.GetYaxis()->SetTitle("strip noise (ADC)");
  h2Noise_512.GetXaxis()->SetTitle("cellID");

  TH2F h2Noise_768("noise2D_768_L@layerL@ladderS@sensor@view",
                   "noise in ADC in @layer.@ladder.@sensor @view/@side VS strip number",
                   128 * 6, -0.5, 128 * 6 - 0.5, 80, -0.5, 9.5);
  h2Noise_768.GetYaxis()->SetTitle("strip noise (ADC)");
  h2Noise_768.GetXaxis()->SetTitle("cellID");

  m_h2Noise = new SVDHistograms<TH2F>(h2Noise_768, h2Noise_768, h2Noise_768, h2Noise_512);


  ///NOISE e-
  TH1F hNoiseEl("noiseEl_L@layerL@ladderS@sensor@view",
                "noise in e- in @layer.@ladder.@sensor @view/@side",
                600, -199.5, 1499.5);
  hNoiseEl.GetXaxis()->SetTitle("strip noise (e-)");
  m_hNoiseEl = new SVDHistograms<TH1F>(hNoiseEl);

  TH2F h2NoiseEl_512("noiseEl2D_512_L@layerL@ladderS@sensor@view",
                     "noise in e- in @layer.@ladder.@sensor @view/@side VS strip number",
                     128 * 4, -0.5, 128 * 4 - 0.5, 600, -199.5, 1499.5);
  h2NoiseEl_512.GetYaxis()->SetTitle("strip noise (e-)");
  h2NoiseEl_512.GetXaxis()->SetTitle("cellID");

  TH2F h2NoiseEl_768("noiseEl2D_768_L@layerL@ladderS@sensor@view",
                     "noise in e- in @layer.@ladder.@sensor @view/@side VS strip number",
                     128 * 6, -0.5, 128 * 6 - 0.5, 600, -199.5, 1499.5);
  h2NoiseEl_768.GetYaxis()->SetTitle("strip noise (e-)");
  h2NoiseEl_768.GetXaxis()->SetTitle("cellID");

  m_h2NoiseEl = new SVDHistograms<TH2F>(h2NoiseEl_768, h2NoiseEl_768, h2NoiseEl_768, h2NoiseEl_512);


  ///PEDESTAL ADC
  TH1F hPedestal("pedestalADC_L@layerL@ladderS@sensor@view",
                 "pedestal in ADC in @layer.@ladder.@sensor @view/@side",
                 200, -199.5, 599.5);
  hPedestal.GetXaxis()->SetTitle("strip pedestal (ADC)");
  m_hPedestal = new SVDHistograms<TH1F>(hPedestal);

  TH2F h2Pedestal_512("pedestal2D_512_L@layerL@ladderS@sensor@view",
                      "pedestal in ADC in @layer.@ladder.@sensor @view/@side VS strip number",
                      128 * 4, -0.5, 128 * 4 - 0.5, 200, -199.5, 599.5);
  h2Pedestal_512.GetYaxis()->SetTitle("strip pedestal (ADC)");
  h2Pedestal_512.GetXaxis()->SetTitle("cellID");

  TH2F h2Pedestal_768("pedestal2D_768_L@layerL@ladderS@sensor@view",
                      "pedestal in ADC in @layer.@ladder.@sensor @view/@side VS strip number",
                      128 * 6, -0.5, 128 * 6 - 0.5, 200, -199.5, 599.5);
  h2Pedestal_768.GetYaxis()->SetTitle("strip pedestal (ADC)");
  h2Pedestal_768.GetXaxis()->SetTitle("cellID");

  m_h2Pedestal = new SVDHistograms<TH2F>(h2Pedestal_768, h2Pedestal_768, h2Pedestal_768, h2Pedestal_512);

  /// 1/GAIN (e-/ADC)
  TH1F hGain("gainADC_L@layerL@ladderS@sensor@view",
             "1/gain in @layer.@ladder.@sensor @view/@side",
             300, -0.5, 499.5);
  hGain.GetXaxis()->SetTitle("strip 1/gain (e-/ADC)");
  m_hGain = new SVDHistograms<TH1F>(hGain);

  TH2F h2Gain_512("gain2D_512_L@layerL@ladderS@sensor@view",
                  "1/gain in @layer.@ladder.@sensor @view/@side VS strip number",
                  128 * 4, -0.5, 128 * 4 - 0.5, 300, -0.5, 499.5);
  h2Gain_512.GetYaxis()->SetTitle("strip 1/gain (e-/ADC)");
  h2Gain_512.GetXaxis()->SetTitle("cellID");

  TH2F h2Gain_768("gain2D_768_L@layerL@ladderS@sensor@view",
                  "1/gain in @layer.@ladder.@sensor @view/@side VS strip number",
                  128 * 6, -0.5, 128 * 6 - 0.5, 300, -0.5, 499.5);
  h2Gain_768.GetYaxis()->SetTitle("strip 1/gain (e-/ADC)");
  h2Gain_768.GetXaxis()->SetTitle("cellID");

  m_h2Gain = new SVDHistograms<TH2F>(h2Gain_768, h2Gain_768, h2Gain_768, h2Gain_512);

  // PEAKTIME (ns)
  TH1F hPeakTime("peakTime_L@layerL@ladderS@sensor@view",
                 "peakTime in @layer.@ladder.@sensor @view/@side",
                 255, -0.5, 254.5);
  hPeakTime.GetXaxis()->SetTitle("strip peakTime (ns)");
  m_hPeakTime = new SVDHistograms<TH1F>(hPeakTime);

  TH2F h2PeakTime_512("peakTime2D_512_L@layerL@ladderS@sensor@view",
                      "peakTime in @layer.@ladder.@sensor @view/@side VS strip number",
                      128 * 4, -0.5, 128 * 4 - 0.5, 255, -0.5, 254.5);
  h2PeakTime_512.GetYaxis()->SetTitle("strip peakTime (ns)");
  h2PeakTime_512.GetXaxis()->SetTitle("cellID");

  TH2F h2PeakTime_768("peakTime2D_768_L@layerL@ladderS@sensor@view",
                      "peakTime in @layer.@ladder.@sensor @view/@side VS strip number",
                      128 * 6, -0.5, 128 * 6 - 0.5, 255, -0.5, 254.5);
  h2PeakTime_768.GetYaxis()->SetTitle("strip peakTime (ns)");
  h2PeakTime_768.GetXaxis()->SetTitle("cellID");

  m_h2PeakTime = new SVDHistograms<TH2F>(h2PeakTime_768, h2PeakTime_768, h2PeakTime_768, h2PeakTime_512);

  // PULSE WIDTH (ns)
  TH1F hPulseWidth("pulseWidth_L@layerL@ladderS@sensor@view",
                   "pulseWidth in @layer.@ladder.@sensor @view/@side",
                   255, -0.5, 254.5);
  hPulseWidth.GetXaxis()->SetTitle("strip pulseWidth (ns)");
  m_hPulseWidth = new SVDHistograms<TH1F>(hPulseWidth);

  TH2F h2PulseWidth_512("pulseWidth2D_512_L@layerL@ladderS@sensor@view",
                        "pulseWidth in @layer.@ladder.@sensor @view/@side VS strip number",
                        128 * 4, -0.5, 128 * 4 - 0.5, 255, -0.5, 254.5);
  h2PulseWidth_512.GetYaxis()->SetTitle("strip pulseWidth (ns)");
  h2PulseWidth_512.GetXaxis()->SetTitle("cellID");

  TH2F h2PulseWidth_768("pulseWidth2D_768_L@layerL@ladderS@sensor@view",
                        "pulseWidth in @layer.@ladder.@sensor @view/@side VS strip number",
                        128 * 6, -0.5, 128 * 6 - 0.5, 255, -0.5, 254.5);
  h2PulseWidth_768.GetYaxis()->SetTitle("strip pulseWidth (ns)");
  h2PulseWidth_768.GetXaxis()->SetTitle("cellID");

  m_h2PulseWidth = new SVDHistograms<TH2F>(h2PulseWidth_768, h2PulseWidth_768, h2PulseWidth_768, h2PulseWidth_512);

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

        m_layer = layer;
        m_ladder = ladder;
        m_sensor = sensor;

        for (m_side = 0; m_side < 2; m_side++) {

          int Ncells = currentSensorInfo->getUCells();
          if (m_side == 0)
            Ncells = currentSensorInfo->getVCells();

          for (m_strip = 0; m_strip < Ncells; m_strip++) {

            m_mask = -1;
            if (m_MaskedStr.isValid())
              m_mask = m_MaskedStr.isMasked(theVxdID, m_side, m_strip);
            m_hMask->fill(theVxdID, m_side, m_mask);
            m_h2Mask->fill(theVxdID, m_side, m_strip, m_mask);

            m_noise = -1;
            m_noiseEl = -1;
            if (m_NoiseCal.isValid()) {
              m_noise = m_NoiseCal.getNoise(theVxdID, m_side, m_strip);
              m_noiseEl = m_NoiseCal.getNoiseInElectrons(theVxdID, m_side, m_strip);
            }
            m_hNoise->fill(theVxdID, m_side, m_noise);
            m_h2Noise->fill(theVxdID, m_side, m_strip, m_noise);
            m_hNoiseEl->fill(theVxdID, m_side, m_noiseEl);
            m_h2NoiseEl->fill(theVxdID, m_side, m_strip, m_noiseEl);

            m_pedestal = -1;
            if (m_PedestalCal.isValid())
              m_pedestal = m_PedestalCal.getPedestal(theVxdID, m_side, m_strip);
            m_hPedestal->fill(theVxdID, m_side, m_pedestal);
            m_h2Pedestal->fill(theVxdID, m_side, m_strip, m_pedestal);

            m_gain = -1;
            if (m_PulseShapeCal.isValid()) {
              m_gain = m_PulseShapeCal.getChargeFromADC(theVxdID, m_side, m_strip, 1/*ADC*/);
              m_peakTime = m_PulseShapeCal.getPeakTime(theVxdID, m_side, m_strip);
              m_pulseWidth = m_PulseShapeCal.getWidth(theVxdID, m_side, m_strip);
            }
            m_hGain->fill(theVxdID, m_side, m_gain);
            m_h2Gain->fill(theVxdID, m_side, m_strip, m_gain);
            m_hPeakTime->fill(theVxdID, m_side, m_peakTime);
            m_h2PeakTime->fill(theVxdID, m_side, m_strip, m_peakTime);
            m_hPulseWidth->fill(theVxdID, m_side, m_pulseWidth);
            m_h2PulseWidth->fill(theVxdID, m_side, m_strip, m_pulseWidth);


            float time_shift = m_PulseShapeCal.getTimeShiftCorrection(theVxdID, m_side, m_strip);
            h_timeshift[layer][ladder][sensor][m_side]->Fill(time_shift);
            float triggerbin_shift = m_PulseShapeCal.getTriggerBinDependentCorrection(theVxdID, m_side, m_strip,
                                     0); /*reading by default the trigger bin #0*/
            h_triggerbin[layer][ladder][sensor][m_side]->Fill(triggerbin_shift);

            float clsSNR = m_ClusterCal.getMinClusterSNR(theVxdID, m_side);
            h_clsSNR[layer][ladder][sensor][m_side]->Fill(clsSNR);
            float clsSeedSNR = m_ClusterCal.getMinSeedSNR(theVxdID, m_side);
            h_clsSeedSNR[layer][ladder][sensor][m_side]->Fill(clsSeedSNR);
            float clsAdjSNR = m_ClusterCal.getMinAdjSNR(theVxdID, m_side);
            h_clsAdjSNR[layer][ladder][sensor][m_side]->Fill(clsAdjSNR);
            float clsScaleErr1 = m_ClusterCal.getCorrectedClusterPositionError(theVxdID, m_side, 1, 1);
            h_clsScaleErr1[layer][ladder][sensor][m_side]->Fill(clsScaleErr1);
            float clsScaleErr2 = m_ClusterCal.getCorrectedClusterPositionError(theVxdID, m_side, 2, 1);
            h_clsScaleErr2[layer][ladder][sensor][m_side]->Fill(clsScaleErr2);
            float clsScaleErr3 = m_ClusterCal.getCorrectedClusterPositionError(theVxdID, 1, 3, 1);
            h_clsScaleErr3[layer][ladder][sensor][m_side]->Fill(clsScaleErr3);

            float clsTimeMin = m_ClusterCal.getMinClusterTime(theVxdID, m_side);
            h_clsTimeMin[layer][ladder][sensor][m_side]->Fill(clsTimeMin);

            float clsTimeFunc = m_ClusterCal.getTimeSelectionFunction(theVxdID, m_side);
            h_clsTimeFuncVersion[layer][ladder][sensor][m_side]->Fill(clsTimeFunc);

            m_treeDetailed->Fill();

          }

        }

        //compute averages adn RMS
        for (m_side = 0; m_side < 2; m_side++) {
          m_noiseAVE = -1; //h_noise[layer][ladder][sensor][m_side]->GetMean();
          m_noiseRMS = -1; //h_noise[layer][ladder][sensor][m_side]->GetRMS();
          m_gainAVE = -1;//h_gainInElectrons[layer][ladder][sensor][m_side]->GetMean();
          m_gainRMS = -1;//h_gainInElectrons[layer][ladder][sensor][m_side]->GetRMS();
          m_peakTimeAVE = -1; //h_peakTime[layer][ladder][sensor][m_side]->GetMean();
          m_peakTimeRMS = -1; //h_peakTime[layer][ladder][sensor][m_side]->GetRMS();
          m_pulseWidthAVE = -1;//h_pulseWidth[layer][ladder][sensor][m_side]->GetMean();
          m_pulseWidthRMS = -1;//h_pulseWidth[layer][ladder][sensor][m_side]->GetRMS();
          m_tree->Fill();

          /*
                //fill TProfiles
                char profName[128];
                char selection[128];
                TString nameSide = "";
                if (m_side == 1)
                  nameSide = "U";
                else if (m_side == 0)
                  nameSide = "V";

                //mask
                sprintf(profName, "prof_masked_%d.%d.%d.%s", layer, ladder, sensor, nameSide.Data());
                sprintf(selection, "layer==%d&&ladder==%d&&sensor==%d&&side==%d", layer, ladder, sensor, m_side);
          //          m_treeDetailed->Project(profName, "mask:strip", selection);

                //noise
                sprintf(profName, "prof_noiseADC_%d.%d.%d.%s", layer, ladder, sensor, nameSide.Data());
                sprintf(selection, "layer==%d&&ladder==%d&&sensor==%d&&side==%d", layer, ladder, sensor, m_side);
          m_treeDetailed->Project(profName, "noise:strip", selection);


                //gain
                sprintf(profName, "prof_gain_%d.%d.%d.%s", layer, ladder, sensor, nameSide.Data());
                sprintf(selection, "layer==%d&&ladder==%d&&sensor==%d&&side==%d", layer, ladder, sensor, m_side);
                m_treeDetailed->Project(profName, "gain:strip", selection);
          */
        }
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }

  //  B2INFO("iscluster in time if t0 = 0? "<< );

}

void SVDCalibrationsMonitorModule::endRun()
{
  B2RESULT("******************************************");
  B2RESULT("** UNIQUE IDs of calibration DB objects **");
  B2RESULT("");
  B2RESULT("   - SVDFADCMaskedStrips:" << m_MaskedStr.getUniqueID());
  B2RESULT("   - SVDNoiseCalibrations:" << m_NoiseCal.getUniqueID());
  B2RESULT("   - SVDPulseShapeCalibrations:" << m_PulseShapeCal.getUniqueID());
  B2RESULT("   - SVDClusterCalibrations:" << m_ClusterCal.getUniqueID());
}

void SVDCalibrationsMonitorModule::terminate()
{
  TObject* obj;
  if (m_rootFilePtr != NULL) {

    //write the tree
    m_treeDetailed->Write();
    m_tree->Write();

    m_rootFilePtr->mkdir("masked_strips");
    m_rootFilePtr->mkdir("pedestal_ADCunits");
    m_rootFilePtr->mkdir("noise_ADCunits");
    m_rootFilePtr->mkdir("noise_electronsCharge");
    m_rootFilePtr->mkdir("gain_electronsCharge");
    m_rootFilePtr->mkdir("peakTime");
    m_rootFilePtr->mkdir("pulseWidth");

    VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

    for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD))
      for (auto ladder : geoCache.getLadders(layer))
        for (Belle2::VxdID sensor :  geoCache.getSensors(ladder))
          for (int view = SVDHistograms<TH1F>::VIndex ; view < SVDHistograms<TH1F>::UIndex + 1; view++) {

            //writing the histogram list for the masks in ADC units
            m_rootFilePtr->cd("masked_strips");
            (m_hMask->getHistogram(sensor, view))->Write();
            (m_h2Mask->getHistogram(sensor, view))->Write();

            //writing the histogram list for the pedestals in ADC units
            m_rootFilePtr->cd("pedestal_ADCunits");
            (m_hPedestal->getHistogram(sensor, view))->Write();
            (m_h2Pedestal->getHistogram(sensor, view))->Write();

            //writing the histogram list for the noises in ADC units
            m_rootFilePtr->cd("noise_ADCunits");
            (m_hNoise->getHistogram(sensor, view))->Write();
            (m_h2Noise->getHistogram(sensor, view))->Write();

            //writing the histogram list for the noises in electron charge
            m_rootFilePtr->cd("noise_electronsCharge");
            (m_hNoiseEl->getHistogram(sensor, view))->Write();
            (m_h2NoiseEl->getHistogram(sensor, view))->Write();

            //writing the histogram list for the gains in electron charge
            m_rootFilePtr->cd("gain_electronsCharge");
            (m_hGain->getHistogram(sensor, view))->Write();
            (m_h2Gain->getHistogram(sensor, view))->Write();

            //writing the histogram list for the peak times in ns
            m_rootFilePtr->cd("peakTime");
            (m_hPeakTime->getHistogram(sensor, view))->Write();
            (m_h2PeakTime->getHistogram(sensor, view))->Write();

            //writing the histogram list for the pulse widths in ns
            m_rootFilePtr->cd("pulseWidth");
            (m_hPulseWidth->getHistogram(sensor, view))->Write();
            (m_h2PulseWidth->getHistogram(sensor, view))->Write();

          }

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


    //writing the histogram list for the clusters
    m_rootFilePtr->mkdir("cluster");
    m_rootFilePtr->cd("cluster");
    TIter nextH_clusters(m_histoList_cluster);
    while ((obj = nextH_clusters()))
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

TProfile*  SVDCalibrationsMonitorModule::createProfile(const char* name, const char* title,
                                                       Int_t nbins, Double_t min, Double_t max,
                                                       const char* xtitle, const char* ytitle, TList* histoList)
{

  TProfile* h = new TProfile(name, title, nbins, min, max);

  h->GetXaxis()->SetTitle(xtitle);
  h->GetYaxis()->SetTitle(ytitle);

  if (histoList)
    histoList->Add(h);

  return h;
}
