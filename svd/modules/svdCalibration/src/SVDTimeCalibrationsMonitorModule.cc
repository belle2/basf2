/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani, Giulia Casarosa                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdCalibration/SVDTimeCalibrationsMonitorModule.h>
#include <vxd/geometry/GeoCache.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDTimeCalibrationsMonitor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDTimeCalibrationsMonitorModule::SVDTimeCalibrationsMonitorModule() : Module()
{
  // Set module properties
  setDescription("Module to produce a list of histograms showing the uploaded calibration constants");

  // Parameter definitions
  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDTimeCalibrationMonitor_output.root"));

  addParam("timeAlgo", m_timeAlgo, "Name of the time algorithm: CoG6, CoG3, ELS3.", std::string("CoG3"));
}

void SVDTimeCalibrationsMonitorModule::beginRun()
{

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  //tree initialization
  m_tree = new TTree("calibTime", "RECREATE");
  b_exp = m_tree->Branch("exp", &m_exp, "exp/i");
  b_run = m_tree->Branch("run", &m_run, "run/i");
  b_layer = m_tree->Branch("layer", &m_layer, "layer/i");
  b_ladder = m_tree->Branch("ladder", &m_ladder, "ladder/i");
  b_sensor = m_tree->Branch("sensor", &m_sensor, "sensor/i");
  b_side = m_tree->Branch("side", &m_side, "side/i");
  b_triggerBin = m_tree->Branch("triggerBin", &m_triggerBin, "triggerBin/i");
  b_c0 = m_tree->Branch("c0", &m_c0, "c0/F");
  b_c1 = m_tree->Branch("c1", &m_c1, "c1/F");
  b_c2 = m_tree->Branch("c2", &m_c2, "c2/F");
  b_c3 = m_tree->Branch("c3", &m_c3, "c3/F");


  if (TString(m_timeAlgo).Contains("CoG6") && ! m_CoG6TimeCal.isValid())
    B2WARNING("No valid SVDCoGTimeCalibrations for the requested IoV");

  if (TString(m_timeAlgo).Contains("CoG3") && ! m_CoG3TimeCal.isValid())
    B2WARNING("No valid SVD3SampleCoGTimeCalibrations for the requested IoV");


  if (TString(m_timeAlgo).Contains("ELS3") && ! m_ELS3TimeCal.isValid())
    B2WARNING("No valid SVD3SampleELSTimeCalibrations for the requested IoV");


}

void SVDTimeCalibrationsMonitorModule::event()
{

  StoreObjPtr<EventMetaData> meta;
  m_run = meta->getRun();
  m_exp = meta->getExperiment();

  if (TString(m_timeAlgo).Contains("CoG6"))
    if (!m_CoG6TimeCal.isValid())
      return;

  if (TString(m_timeAlgo).Contains("CoG3"))
    if (!m_CoG3TimeCal.isValid())
      return;

  if (TString(m_timeAlgo).Contains("ELS3"))
    if (!m_ELS3TimeCal.isValid())
      return;


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

        m_layer = itSvdSensors->getLayerNumber();
        m_ladder =  itSvdSensors->getLadderNumber();
        m_sensor = itSvdSensors->getSensorNumber();
        Belle2::VxdID theVxdID(m_layer, m_ladder, m_sensor);

        for (m_side = 0; m_side < 2; m_side++) {

          for (m_triggerBin = 0; m_triggerBin < 4; m_triggerBin++) {

            float f0 = -99;
            float f1 = -99;
            float f2 = -99;
            float f4 = -99;

            if (TString(m_timeAlgo).Contains("CoG6")) {
              f0 = m_CoG6TimeCal.getCorrectedTime(theVxdID, m_side, 0 /*strip*/, 0 /*raw time*/, m_triggerBin);
              f1 = m_CoG6TimeCal.getCorrectedTime(theVxdID, m_side, 0 /*strip*/, 1 /*raw time*/, m_triggerBin);
              f2 = m_CoG6TimeCal.getCorrectedTime(theVxdID, m_side, 0 /*strip*/, 2 /*raw time*/, m_triggerBin);
              f4 = m_CoG6TimeCal.getCorrectedTime(theVxdID, m_side, 0 /*strip*/, 4 /*raw time*/, m_triggerBin);
            }

            if (TString(m_timeAlgo).Contains("CoG3")) {
              f0 = m_CoG3TimeCal.getCorrectedTime(theVxdID, m_side, 0 /*strip*/, 0 /*raw time*/, m_triggerBin);
              f1 = m_CoG3TimeCal.getCorrectedTime(theVxdID, m_side, 0 /*strip*/, 1 /*raw time*/, m_triggerBin);
              f2 = m_CoG3TimeCal.getCorrectedTime(theVxdID, m_side, 0 /*strip*/, 2 /*raw time*/, m_triggerBin);
              f4 = m_CoG3TimeCal.getCorrectedTime(theVxdID, m_side, 0 /*strip*/, 4 /*raw time*/, m_triggerBin);

              m_c0 = f0;
              m_c1 = 1. / 12 * (-21 * f0 + 32 * f1 - 12 * f2 + f4);
              m_c2 = 1. / 8  * (7 * f0 - 16 * f1 + 10 * f2 - f4);
              m_c3 = 1. / 24 * (-3 * f0 +  8 * f1 -  6 * f2 + f4);

            }

            if (TString(m_timeAlgo).Contains("ELS3")) {
              f0 = m_ELS3TimeCal.getCorrectedTime(theVxdID, m_side, 0 /*strip*/, 0 /*raw time*/, m_triggerBin);
              f1 = m_ELS3TimeCal.getCorrectedTime(theVxdID, m_side, 0 /*strip*/, 1 /*raw time*/, m_triggerBin);
              f2 = m_ELS3TimeCal.getCorrectedTime(theVxdID, m_side, 0 /*strip*/, 2 /*raw time*/, m_triggerBin);
              f4 = m_ELS3TimeCal.getCorrectedTime(theVxdID, m_side, 0 /*strip*/, 4 /*raw time*/, m_triggerBin);
            }

            m_tree->Fill();
          }

        }
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }

}

void SVDTimeCalibrationsMonitorModule::endRun()
{
  B2RESULT("******************************************");
  B2RESULT("** UNIQUE IDs of calibration DB objects **");
  B2RESULT("");
  if (TString(m_timeAlgo).Contains("CoG6")) {
    if (m_CoG6TimeCal.isValid())
      B2RESULT("   - SVDCoGTimeCalibrations:" << m_CoG6TimeCal.getUniqueID());
    else
      B2WARNING("No valid SVDCoGTimeCalibrations for the requested IoV");
  }

  if (TString(m_timeAlgo).Contains("CoG3")) {
    if (m_CoG3TimeCal.isValid())
      B2RESULT("   - SVD3SampleCoGTimeCalibrations:" << m_CoG3TimeCal.getUniqueID());
    else
      B2WARNING("No valid SVD3SampleCoGTimeCalibrations for the requested IoV");
  }

  if (TString(m_timeAlgo).Contains("ELS3")) {
    if (m_ELS3TimeCal.isValid())
      B2RESULT("   - SVD3SampleELSTimeCalibrations:" << m_ELS3TimeCal.getUniqueID());
    else
      B2WARNING("No valid SVD3SampleELSTimeCalibrations for the requested IoV");
  }

  if (m_rootFilePtr != nullptr) {

    m_rootFilePtr->cd();

    //write the tree
    m_tree->Write();

    m_rootFilePtr->Close();
    B2RESULT("The rootfile containing the list of histograms has been filled and closed [Time].");


  }
}

