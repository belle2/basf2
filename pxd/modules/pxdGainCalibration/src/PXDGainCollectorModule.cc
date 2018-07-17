/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdGainCalibration/PXDGainCollectorModule.h>
#include <vxd/geometry/GeoCache.h>
#include <pxd/geometry/SensorInfo.h>
#include <pxd/reconstruction/PXDGainCalibrator.h>

#include <TTree.h>
#include <TH1I.h>

#include <boost/format.hpp>
#include <cmath>

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDGainCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDGainCollectorModule::PXDGainCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("Calibration collector module for PXD gain calibration");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("clustersName", m_storeClustersName, "Name of the collection to use for PXDClusters", string(""));
  addParam("minClusterCharge", m_minClusterCharge, "Minimum cluster charge cut", int(0));
  addParam("minClusterSize", m_minClusterSize, "Minimum cluster size cut ", int(2));
  addParam("maxClusterSize", m_maxClusterSize, "Maximum cluster size cut ", int(6));
  addParam("collectSimulatedData", m_simulatedDataFlag, "If true, collector runs over simulation data ", bool(false));
  addParam("nBinsU", m_nBinsU, "Number of gain corrections per sensor along u side", int(4));
  addParam("nBinsV", m_nBinsV, "Number of gain corrections per sensor along v side", int(6));
}

void PXDGainCollectorModule::prepare() // Do your initialise() stuff here
{
  m_pxdCluster.isRequired();

  if (m_nBinsU == 0) {
    B2WARNING("Number of bins along u side incremented from 0->1");
    m_nBinsU = 1;
  }

  if (m_nBinsV == 0) {
    B2WARNING("Number of bins along v side incremented from 0->1");
    m_nBinsV = 1;
  }

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  int nPXDSensors = gTools->getNumberOfPXDSensors();

  //-------------------------------------------------------------------------------------
  // PXDDataCounter: Count the number of PXDClusters for each uBin/vBin pair in data sample
  //-------------------------------------------------------------------------------------

  auto hPXDDataCounter = new TH1I("hPXDDataCounter", "Number of clusters found in data sample", m_nBinsU * m_nBinsV * nPXDSensors, 0,
                                  m_nBinsU * m_nBinsV * nPXDSensors);
  hPXDDataCounter->GetXaxis()->SetTitle("Gain id");
  hPXDDataCounter->GetYaxis()->SetTitle("Number of clusters");
  for (int iSensor = 0; iSensor < nPXDSensors; iSensor++) {
    for (int uBin = 0; uBin < m_nBinsU; uBin++) {
      for (int vBin = 0; vBin < m_nBinsV; vBin++) {
        VxdID id = gTools->getSensorIDFromPXDIndex(iSensor);
        string sensorDescr = id;
        hPXDDataCounter->GetXaxis()->SetBinLabel(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin + 1,
                                                 str(format("%1%_%2%_%3%") % sensorDescr % uBin % vBin).c_str());
      }
    }
  }
  registerObject<TH1I>("PXDDataCounter", hPXDDataCounter);

  //---------------------------------------------------------------------------------
  // PXDMCCounter: Count the number of PXDClusters for each uBin/vBin pair in MC sample
  //---------------------------------------------------------------------------------

  auto hPXDMCCounter = new TH1I("hPXDMCCounter", "Number of clusters found in mc sample", m_nBinsU * m_nBinsV * nPXDSensors, 0,
                                m_nBinsU * m_nBinsV * nPXDSensors);
  hPXDMCCounter->GetXaxis()->SetTitle("Gain id");
  hPXDMCCounter->GetYaxis()->SetTitle("Number of clusters");
  for (int iSensor = 0; iSensor < nPXDSensors; iSensor++) {
    for (int uBin = 0; uBin < m_nBinsU; uBin++) {
      for (int vBin = 0; vBin < m_nBinsV; vBin++) {
        VxdID id = gTools->getSensorIDFromPXDIndex(iSensor);
        string sensorDescr = id;
        hPXDMCCounter->GetXaxis()->SetBinLabel(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin + 1,
                                               str(format("%1%_%2%_%3%") % sensorDescr % uBin % vBin).c_str());
      }
    }
  }
  registerObject<TH1I>("PXDMCCounter", hPXDMCCounter);

  //----------------------------------------------------------------------
  // PXDTrees: One tree to store the calibration data for each gain region
  //----------------------------------------------------------------------

  for (int iSensor = 0; iSensor < nPXDSensors; iSensor++) {
    for (int uBin = 0; uBin < m_nBinsU; uBin++) {
      for (int vBin = 0; vBin < m_nBinsV; vBin++) {
        VxdID id = gTools->getSensorIDFromPXDIndex(iSensor);
        auto layerNumber = id.getLayerNumber();
        auto ladderNumber = id.getLadderNumber();
        auto sensorNumber = id.getSensorNumber();
        string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % uBin % vBin);
        auto tree = new TTree(treename.c_str(), treename.c_str());
        tree->Branch<float>("gain", &m_gain);
        tree->Branch<int>("signal", &m_signal);
        tree->Branch<bool>("isMC", &m_isMC);
        registerObject<TTree>(treename, tree);
      }
    }
  }

}

void PXDGainCollectorModule::collect() // Do your event() stuff here
{
  // If no input, nothing to do
  if (!m_pxdCluster) return;

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();

  auto mc_counter = getObjectPtr<TH1I>("PXDMCCounter");
  auto data_counter = getObjectPtr<TH1I>("PXDDataCounter");

  for (auto& cluster :  m_pxdCluster) {

    // Apply cluster selection cuts
    if (cluster.getCharge() >= m_minClusterCharge && cluster.getSize() >= m_minClusterSize && cluster.getSize() <= m_maxClusterSize) {

      VxdID sensorID = cluster.getSensorID();
      const PXD::SensorInfo& Info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
      auto uID = Info.getUCellID(cluster.getU());
      auto vID = Info.getVCellID(cluster.getV());
      auto iSensor = gTools->getPXDSensorIndex(sensorID);
      auto layerNumber = sensorID.getLayerNumber();
      auto ladderNumber = sensorID.getLadderNumber();
      auto sensorNumber = sensorID.getSensorNumber();
      auto uBin = PXD::PXDGainCalibrator::getInstance().getBinU(sensorID, uID, vID, m_nBinsU);
      auto vBin = PXD::PXDGainCalibrator::getInstance().getBinV(sensorID, vID, m_nBinsV);
      string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % uBin % vBin);

      // Compute variables from cluster needed for gain estimation
      m_isMC = m_simulatedDataFlag;
      m_signal = cluster.getCharge();
      if (m_simulatedDataFlag) {
        // For MC, the cluster charge is gain corrected
        m_gain = PXD::PXDGainCalibrator::getInstance().getGainCorrection(sensorID, uID, vID);
      } else {
        // For data, the cluster charge is not gain corrected
        m_gain = 1.0;
      }
      // Fill variabels into tree
      getObjectPtr<TTree>(treename)->Fill();

      // Increment the counters
      if (m_simulatedDataFlag) {
        mc_counter->Fill(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin);
      } else {
        data_counter->Fill(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin);
      }
    }
  }
}
