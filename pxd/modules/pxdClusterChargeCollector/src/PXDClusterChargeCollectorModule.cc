/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdClusterChargeCollector/PXDClusterChargeCollectorModule.h>
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
REG_MODULE(PXDClusterChargeCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDClusterChargeCollectorModule::PXDClusterChargeCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("Calibration collector module for cluster charge related calibrations.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("clustersName", m_storeClustersName, "Name of the collection to use for PXDClusters", string(""));
  addParam("minClusterCharge", m_minClusterCharge, "Minimum cluster charge cut", int(0));
  addParam("minClusterSize", m_minClusterSize, "Minimum cluster size cut ", int(2));
  addParam("maxClusterSize", m_maxClusterSize, "Maximum cluster size cut ", int(6));
  addParam("nBinsU", m_nBinsU, "Number of gain corrections per sensor along u side", int(4));
  addParam("nBinsV", m_nBinsV, "Number of gain corrections per sensor along v side", int(6));
}

void PXDClusterChargeCollectorModule::prepare() // Do your initialise() stuff here
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
  // PXDClusterCounter: Count the number of PXDClusters for each uBin/vBin pair
  //-------------------------------------------------------------------------------------

  auto hPXDClusterCounter = new TH1I("hPXDClusterCounter", "Number of clusters found in data sample",
                                     m_nBinsU * m_nBinsV * nPXDSensors, 0,
                                     m_nBinsU * m_nBinsV * nPXDSensors);
  hPXDClusterCounter->GetXaxis()->SetTitle("bin id");
  hPXDClusterCounter->GetYaxis()->SetTitle("Number of clusters");
  for (int iSensor = 0; iSensor < nPXDSensors; iSensor++) {
    for (int uBin = 0; uBin < m_nBinsU; uBin++) {
      for (int vBin = 0; vBin < m_nBinsV; vBin++) {
        VxdID id = gTools->getSensorIDFromPXDIndex(iSensor);
        string sensorDescr = id;
        hPXDClusterCounter->GetXaxis()->SetBinLabel(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin + 1,
                                                    str(format("%1%_%2%_%3%") % sensorDescr % uBin % vBin).c_str());
      }
    }
  }
  registerObject<TH1I>("PXDClusterCounter", hPXDClusterCounter);


  //----------------------------------------------------------------------
  // PXDTrees: One tree to store the calibration data for each grid bin
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
        tree->Branch<int>("signal", &m_signal);
        registerObject<TTree>(treename, tree);
      }
    }
  }

  auto dbtree = new TTree("dbtree", "dbtree");
  dbtree->Branch<int>("run", &m_run);
  dbtree->Branch<int>("exp", &m_exp);
  dbtree->Branch<PXDClusterChargeMapPar>("chargeMap", &m_chargeMap);
  dbtree->Branch<PXDGainMapPar>("gainMap", &m_gainMap);
  registerObject<TTree>("dbtree", dbtree);
}

void PXDClusterChargeCollectorModule::startRun() // Do your beginRun() stuff here
{
  m_run = m_evtMetaData->getRun();
  m_exp = m_evtMetaData->getExperiment();
  m_chargeMap = *m_DBChargeMapPar;
  m_gainMap = *m_DBGainMapPar;
  getObjectPtr<TTree>("dbtree")->Fill();
}


void PXDClusterChargeCollectorModule::collect() // Do your event() stuff here
{
  // If no input, nothing to do
  if (!m_pxdCluster) return;

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();

  auto cluster_counter = getObjectPtr<TH1I>("PXDClusterCounter");

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
      m_signal = cluster.getCharge();
      // Fill variabels into tree
      getObjectPtr<TTree>(treename)->Fill();
      // Increment the counter
      cluster_counter->Fill(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin);
    }
  }
}
