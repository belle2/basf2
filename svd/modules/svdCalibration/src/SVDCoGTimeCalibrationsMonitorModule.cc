/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani, Giulia Casarosa                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdCalibration/SVDCoGTimeCalibrationsMonitorModule.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDCoGTimeCalibrationsMonitor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDCoGTimeCalibrationsMonitorModule::SVDCoGTimeCalibrationsMonitorModule() : Module()
{
  // Set module properties
  setDescription("Module to produce a list of histograms showing the uploaded calibration constants");

  // Parameter definitions
  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDCoGTimeCalibrationMonitor_output.root"));
}

void SVDCoGTimeCalibrationsMonitorModule::beginRun()
{

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  //tree initialization
  m_tree = new TTree("calibCoGTime", "RECREATE");
  b_run = m_tree->Branch("run", &m_run, "run/i");
  b_layer = m_tree->Branch("layer", &m_layer, "layer/i");
  b_ladder = m_tree->Branch("ladder", &m_ladder, "ladder/i");
  b_sensor = m_tree->Branch("sensor", &m_sensor, "sensor/i");
  b_side = m_tree->Branch("side", &m_side, "side/i");
  b_triggerBin = m_tree->Branch("triggerBin", &m_triggerBin, "triggerBin/i");
  b_alpha = m_tree->Branch("alpha", &m_alpha, "alpha/F");
  b_beta = m_tree->Branch("beta", &m_beta, "beta/F");

  if (! m_CoGTimeCal.isValid())
    B2WARNING("No valid SVDCoGTimeCalibrations for the requested IoV");


  /// histogram definition will go here


}

void SVDCoGTimeCalibrationsMonitorModule::event()
{

  StoreObjPtr<EventMetaData> meta;
  m_run = meta->getRun();

  if (!m_CoGTimeCal.isValid())
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

            m_beta = m_CoGTimeCal.getCorrectedTime(theVxdID, m_side, 0 /*strip*/, 0 /*raw time*/, m_triggerBin);
            m_alpha = m_CoGTimeCal.getCorrectedTime(theVxdID, m_side, 0 /*strip*/, 1 /*raw time*/, m_triggerBin) - m_beta;

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

void SVDCoGTimeCalibrationsMonitorModule::endRun()
{
  B2RESULT("******************************************");
  B2RESULT("** UNIQUE IDs of calibration DB objects **");
  B2RESULT("");
  if (m_CoGTimeCal.isValid())
    B2RESULT("   - SVDCoGTimeCalibrations:" << m_CoGTimeCal.getUniqueID());
  else
    B2WARNING("No valid SVDCoGTimeCalibrations for the requested IoV");

  if (m_rootFilePtr != NULL) {

    m_rootFilePtr->cd();

    //write the tree
    m_tree->Write();

    m_rootFilePtr->Close();
    B2RESULT("The rootfile containing the list of histograms has been filled and closed [CoGTime].");


  }
}

