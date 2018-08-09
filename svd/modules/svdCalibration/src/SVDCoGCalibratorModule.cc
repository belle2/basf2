/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani, Giulia Casarosa, Michael De Nuccio           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdCalibration/SVDCoGCalibratorModule.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/dataobjects/VxdID.h>
#include <TFile.h>
#include <TH1F.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDCluster.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>

#include <framework/dataobjects/EventMetaData.h>

#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>
//#include <framework/gearbox/Unit.h>
//#include <framework/gearbox/Const.h>
//#include <framework/logging/Logger.h>
//#include <framework/utilities/FileSystem.h>
//#include <svd/calibration/SVDNoiseCalibrations.h>
//#include <svd/calibration/SVDPulseShapeCalibrations.h>
//#include <svd/dbobjects/SVDLocalRunBadStrips.h>
//#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
//#include <vxd/dataobjects/VxdID.h>
//#include <iostream>
//#include <fstream>
//#include <sstream>
//#include <TFile.h>
//#include <TVectorF.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDCoGCalibrator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDCoGCalibratorModule::SVDCoGCalibratorModule() : Module()
{
  // Set module properties
  setDescription("Module to produce a list of histogram showing the uploaded calibration constants");

  // Parameter definitions
  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDCoGCalibration_output.root"));
  addParam("useTracks", m_useTracks, "Use Tracks.", true);
  addParam("fillAreas", m_fillAreas, "Fill DB with area-related values; if false it fills it with sensor-related values.", true);
}

void SVDCoGCalibratorModule::initialize()
{
  m_histoList_CoGTime = new TList;

  m_histoList_CoGTime_all = new TList;
  m_histoList_CoGTime_areas = new TList;
  m_histoList_CoGTime_phi = new TList;

  m_histoList_CoGMean_all = new TList;
  m_histoList_CoGMean_areas = new TList;
  m_histoList_CoGMean_phi = new TList;

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  TString NameOfHisto = "";
  TString TitleOfHisto = "";

  //call for a geometry instance
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  std::set<Belle2::VxdID> svdLayers = aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();
  //loop to create the initial histos
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

          ///HISTOS
          NameOfHisto = "CoGTime_" + nameLayer + "." + nameLadder + "." + nameSensor + "." + nameSide;
          TitleOfHisto = "CoGTime (Layer" + nameLayer + ", Ladder" + nameLadder + ", Sensor" + nameSensor + ", Side" + nameSide + ")";
          h_CoGTime[layer][ladder][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 800, -200, 200, "CoGTime [ns]",
                                                                     m_histoList_CoGTime);
        }
        //histogram created
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }

  //loop to create the all-means histos
  for (int side = 0; side < 2; side++) {
    //create your histogram
    TString nameSide = "";
    if (side == 1)
      nameSide = "U";
    else if (side == 0)
      nameSide = "V";

    ///HISTOS
    NameOfHisto = "CoG_SensorMeanTime_AllSensors." + nameSide;
    TitleOfHisto = "CoG_SensorsMeanTime (All Sensors, Side" + nameSide + ")";
    h_CoGMean_all[side] = createHistogram1D(NameOfHisto, TitleOfHisto, 4000, -200, 200, "CoGMeanTime [ns]", m_histoList_CoGMean_all);

    NameOfHisto = "CoG_SensorTime_AllSensors." + nameSide;
    TitleOfHisto = "CoG_SensorsTime (All Sensors, Side" + nameSide + ")";
    h_CoGTime_all[side] = createHistogram1D(NameOfHisto, TitleOfHisto, 800, -200, 200, "CoGTime [ns]", m_histoList_CoGTime_all);
  }

  //loop to create the areas-means histos
  for (int i = 0; i < m_Nsets; i ++) {
    NameOfHisto = "CoG_SensorMeanTime_" + IntExtFromIndex(i) + "." + FWFromIndex(i) + "." + UVFromIndex(i);
    TitleOfHisto = "CoG_SensorsMeanTime (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", Side" + UVFromIndex(i) + ")";
    h_CoGMean_areas[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 4000, -200, 200, "CoGTime [ns]", m_histoList_CoGMean_areas);

    NameOfHisto = "CoG_SensorTime_" + IntExtFromIndex(i) + "." + FWFromIndex(i) + "." + UVFromIndex(i);
    TitleOfHisto = "CoG_SensorsTime (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", Side" + UVFromIndex(i) + ")";
    h_CoGTime_areas[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 800, -200, 200, "CoGTime [ns]", m_histoList_CoGTime_areas);
  }

  //restart the counter of layers
  itSvdLayers = svdLayers.begin();
  //loop to create the phi-means histos
  while ((itSvdLayers != svdLayers.end()) && (itSvdLayers->getLayerNumber() != 7)) { //loop on Layers

    std::set<Belle2::VxdID> svdLadders = aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    std::set<Belle2::VxdID> svdSensors = aGeometry.getSensors(*itSvdLadders);
    std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();
    B2DEBUG(1, "    svd sensor info " << * (svdSensors.begin()));

    while (itSvdSensors != svdSensors.end()) { //loop on sensors
      B2DEBUG(1, "    svd sensor info " << *itSvdSensors);

      for (int side = 0; side < 2; side++) {
        //create your histogram
        int layer = itSvdSensors->getLayerNumber();
        int sensor = itSvdSensors->getSensorNumber();
        TString nameLayer = "";
        nameLayer += layer;

        TString nameSensor = "";
        nameSensor += sensor;

        TString nameSide = "";
        if (side == 1)
          nameSide = "U";
        else if (side == 0)
          nameSide = "V";

        ///HISTOS
        NameOfHisto = "CoG_SensorMeanTime_" + nameLayer + "." + "all" + "." + nameSensor + "." + nameSide;
        TitleOfHisto = "CoG_SensorsMeanTime (Layer" + nameLayer + ", Sensor" + nameSensor + ", Side" + nameSide + ")";
        h_CoGMean_phi[layer][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 4000, -200, 200, "CoGTime [ns]",
                                                               m_histoList_CoGMean_phi);

        NameOfHisto = "CoG_SensorTime_" + nameLayer + "." + "all" + "." + nameSensor + "." + nameSide;
        TitleOfHisto = "CoG_SensorsTime (Layer" + nameLayer + ", Sensor" + nameSensor + ", Side" + nameSide + ")";
        h_CoGTime_phi[layer][sensor][side] = createHistogram1D(NameOfHisto, TitleOfHisto, 800, -200, 200, "CoGTime [ns]",
                                                               m_histoList_CoGTime_phi);
      }
      //histogram created
      ++itSvdSensors;
    }
    ++itSvdLayers;
  }
}

void SVDCoGCalibratorModule::beginRun()
{
}

void SVDCoGCalibratorModule::event()
{
  StoreArray<SVDRecoDigit> SVDRecoDigits;
  StoreArray<SVDCluster> SVDClusters;
  StoreArray<RecoTrack> RecoTracks;
  StoreArray<Track> Tracks;
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  B2DEBUG(1, "RecoTracks size : " << RecoTracks.getEntries());

  m_firstRun = eventMetaDataPtr->getRun();
  m_lastRun = eventMetaDataPtr->getRun();
  m_firstExperiment = eventMetaDataPtr->getExperiment();
  m_lastExperiment = eventMetaDataPtr->getExperiment();

  //loop over the Reco and fill the histos
  if (m_useTracks) {
    B2DEBUG(1, "SVDRecoDigit size : " << SVDRecoDigits.getEntries());
    for (const SVDRecoDigit& reco : SVDRecoDigits) {

      RelationVector<SVDCluster> relatVectorRecoToClus = DataStore::getRelationsWithObj<SVDCluster>(&reco);
      B2DEBUG(1, "relatVectorRecoToClus size : " << relatVectorRecoToClus.size());
      if (relatVectorRecoToClus.size() > 0) {
        RelationVector<RecoTrack> relatVectorClusToRecoTrack = DataStore::getRelationsWithObj<RecoTrack>(relatVectorRecoToClus[0]);
        B2DEBUG(1, "relatVectorClusToRecoTrack size : " << relatVectorClusToRecoTrack.size());
        if (relatVectorClusToRecoTrack.size() > 0) {
          RelationVector<Track> relatVectorRecoTrackToTrack = DataStore::getRelationsWithObj<Track>(relatVectorClusToRecoTrack[0]);
          B2DEBUG(1, "relatVectorRecoTrackToTrack size : " << relatVectorRecoTrackToTrack.size());
          if (relatVectorRecoTrackToTrack.size() > 0) {
            m_CoGTime = reco.getTime();

            VxdID thisSensorID = reco.getSensorID();
            m_Side = reco.isUStrip();
            m_Layer = thisSensorID.getLayerNumber();
            m_Ladder = thisSensorID.getLadderNumber();
            m_Sensor = thisSensorID.getSensorNumber();

            int indexForHistosAndGraphs = indexFromLayerSensorSide(m_Layer, m_Sensor, m_Side);

            h_CoGTime[m_Layer][m_Ladder][m_Sensor][m_Side]->Fill(m_CoGTime);

            h_CoGTime_all[m_Side]->Fill(m_CoGTime);
            h_CoGTime_areas[indexForHistosAndGraphs]->Fill(m_CoGTime);
            h_CoGTime_phi[m_Layer][m_Sensor][m_Side]->Fill(m_CoGTime);
          }
        }
      }
    }
  } else {
    for (const SVDRecoDigit& reco : SVDRecoDigits) {
      m_CoGTime = reco.getTime();

      VxdID thisSensorID = reco.getSensorID();
      m_Side = reco.isUStrip();
      m_Layer = thisSensorID.getLayerNumber();
      m_Ladder = thisSensorID.getLadderNumber();
      m_Sensor = thisSensorID.getSensorNumber();

      int indexForHistosAndGraphs = indexFromLayerSensorSide(m_Layer, m_Sensor, m_Side);

      h_CoGTime[m_Layer][m_Ladder][m_Sensor][m_Side]->Fill(m_CoGTime);

      h_CoGTime_all[m_Side]->Fill(m_CoGTime);
      h_CoGTime_areas[indexForHistosAndGraphs]->Fill(m_CoGTime);
      h_CoGTime_phi[m_Layer][m_Sensor][m_Side]->Fill(m_CoGTime);
    }
  }
}

void SVDCoGCalibratorModule::endRun()
{
}

void SVDCoGCalibratorModule::terminate()
{
  //fill the histos with means
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  std::set<Belle2::VxdID> svdLayers = aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();
  while ((itSvdLayers != svdLayers.end()) && (itSvdLayers->getLayerNumber() != 7)) { //loop on Layers
    std::set<Belle2::VxdID> svdLadders = aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();
    while (itSvdLadders != svdLadders.end()) { //loop on Ladders
      std::set<Belle2::VxdID> svdSensors = aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();
      while (itSvdSensors != svdSensors.end()) { //loop on sensors
        for (int side = 0; side < 2; side++) {
          int layer = itSvdSensors->getLayerNumber();
          int ladder =  itSvdSensors->getLadderNumber();
          int sensor = itSvdSensors->getSensorNumber();

          int indexForHistosAndGraphs = indexFromLayerSensorSide(layer, sensor, side);

          float meanCoG = h_CoGTime[layer][ladder][sensor][side]->GetMean();

          h_CoGMean_all[side]->Fill(meanCoG);
          h_CoGMean_areas[indexForHistosAndGraphs]->Fill(meanCoG);
          h_CoGMean_phi[layer][sensor][side]->Fill(meanCoG);
        }
        //histogram created
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }

  //save histos in appropriate folders
  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    TObject* obj;
    TDirectory* oldDir = gDirectory;

    TDirectory* dir_CoGSensor = oldDir->mkdir("CoGTimeDistribution_perSensor");
    dir_CoGSensor->cd();
    TIter nextH_CoGTime(m_histoList_CoGTime);
    while ((obj = nextH_CoGTime())) {
      obj->Write();
    }

    TDirectory* dir_timeAll = oldDir->mkdir("CoGTime_all");
    dir_timeAll->cd();
    TIter nextH_TimeAll(m_histoList_CoGTime_all);
    while ((obj = nextH_TimeAll())) {
      obj->Write();
    }

    TDirectory* dir_timeAreas = oldDir->mkdir("CoGTime_areas");
    dir_timeAreas->cd();
    TIter nextH_TimeAreas(m_histoList_CoGTime_areas);
    while ((obj = nextH_TimeAreas())) {
      obj->Write();
    }

    TDirectory* dir_timePhi = oldDir->mkdir("CoGTime_phi");
    dir_timePhi->cd();
    TIter nextH_TimePhi(m_histoList_CoGTime_phi);
    while ((obj = nextH_TimePhi())) {
      obj->Write();
    }

    TDirectory* dir_meanAll = oldDir->mkdir("CoGMeanTime_all");
    dir_meanAll->cd();
    TIter nextH_MeanAll(m_histoList_CoGMean_all);
    while ((obj = nextH_MeanAll())) {
      obj->Write();
    }

    TDirectory* dir_meanAreas = oldDir->mkdir("CoGMeanTime_areas");
    dir_meanAreas->cd();
    TIter nextH_MeanAreas(m_histoList_CoGMean_areas);
    while ((obj = nextH_MeanAreas())) {
      obj->Write();
    }

    TDirectory* dir_meanPhi = oldDir->mkdir("CoGMeanTime_phi");
    dir_meanPhi->cd();
    TIter nextH_MeanPhi(m_histoList_CoGMean_phi);
    while ((obj = nextH_MeanPhi())) {
      obj->Write();
    }

    m_rootFilePtr->Close();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// EXTRA FUNCTIONS ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

TH1F*  SVDCoGCalibratorModule::createHistogram1D(const char* name, const char* title,
                                                 Int_t nbins, Double_t min, Double_t max,
                                                 const char* xtitle, TList* histoList)
{
  TH1F* h = new TH1F(name, title, nbins, min, max);

  h->GetXaxis()->SetTitle(xtitle);

  if (histoList)
    histoList->Add(h);

  return h;
}

int SVDCoGCalibratorModule::indexFromLayerSensorSide(int LayerNumber, int SensorNumber, int UVNumber)
{
  int Index;

  if (LayerNumber == 3) { //L3
    if (UVNumber) //U
      Index = 0;
    else //V
      Index = 1;
  } else { //L456
    if (SensorNumber == 1) { //FW
      if (UVNumber) //U
        Index = 2;
      else //V
        Index = 3;
    } else { //barrel
      if (UVNumber) //U
        Index = 4;
      else //V
        Index = 5;
    }
  }

  return Index;
}

TString SVDCoGCalibratorModule::IntExtFromIndex(int idx)
{
  TString name = "";

  if (idx < 2)
    name = "L3";
  else
    name = "L456";

  return name;
}

TString SVDCoGCalibratorModule::FWFromIndex(int idx)
{
  TString name = "";

  if (idx == 2 || idx == 3)
    name = "FWD";
  else
    name = "Barrel";

  return name;
}

TString SVDCoGCalibratorModule::UVFromIndex(int idx)
{
  TString name = "";

  if (idx % 2 == 0)
    name = "U";
  else
    name = "V";

  return name;
}

















