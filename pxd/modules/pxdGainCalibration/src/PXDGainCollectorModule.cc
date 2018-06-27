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
#include <pxd/unpacking/PXDMappingLookup.h>

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
}

void PXDGainCollectorModule::prepare() // Do your initialise() stuff here
{
  m_pxdCluster.isRequired();

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  int nDCD = gTools->getNumberOfPXDUSideChips();
  int nSWB = gTools->getNumberOfPXDVSideChips();
  int nPXDSensors = gTools->getNumberOfPXDSensors();

  //-------------------------------------------------------------------------------------
  // PXDDataCounter: Count the number of PXDClusters for each DCD/SWB pair in data sample
  //-------------------------------------------------------------------------------------

  auto hPXDDataCounter = new TH1I("hPXDDataCounter", "Number of clusters found in data sample", nDCD * nSWB * nPXDSensors, 0,
                                  nDCD * nSWB * nPXDSensors);
  hPXDDataCounter->GetXaxis()->SetTitle("Chip pair");
  hPXDDataCounter->GetYaxis()->SetTitle("Number of clusters");
  for (int iSensor = 0; iSensor < nPXDSensors; iSensor++) {
    for (int iDCD = 0; iDCD < nDCD; iDCD++) {
      for (int iSWB = 0; iSWB < nSWB; iSWB++) {
        VxdID id = gTools->getSensorIDFromPXDIndex(iSensor);
        string sensorDescr = id;
        hPXDDataCounter->GetXaxis()->SetBinLabel(iSensor * nDCD * nSWB + iDCD * nSWB + iSWB + 1,
                                                 str(format("%1%_%2%_%3%") % sensorDescr % iDCD % iSWB).c_str());
      }
    }
  }
  registerObject<TH1I>("PXDDataCounter", hPXDDataCounter);

  //---------------------------------------------------------------------------------
  // PXDMCCounter: Count the number of PXDClusters for each DCD/SWB pair in MC sample
  //---------------------------------------------------------------------------------

  auto hPXDMCCounter = new TH1I("hPXDMCCounter", "Number of clusters found in mc sample", nDCD * nSWB * nPXDSensors, 0,
                                nDCD * nSWB * nPXDSensors);
  hPXDMCCounter->GetXaxis()->SetTitle("Chip pair");
  hPXDMCCounter->GetYaxis()->SetTitle("Number of clusters");
  for (int iSensor = 0; iSensor < nPXDSensors; iSensor++) {
    for (int iDCD = 0; iDCD < nDCD; iDCD++) {
      for (int iSWB = 0; iSWB < nSWB; iSWB++) {
        VxdID id = gTools->getSensorIDFromPXDIndex(iSensor);
        string sensorDescr = id;
        hPXDMCCounter->GetXaxis()->SetBinLabel(iSensor * nDCD * nSWB + iDCD * nSWB + iSWB + 1,
                                               str(format("%1%_%2%_%3%") % sensorDescr % iDCD % iSWB).c_str());
      }
    }
  }
  registerObject<TH1I>("PXDMCCounter", hPXDMCCounter);

  //----------------------------------------------------------------------
  // PXDTrees: One tree to store the calibration data for each chip region
  //----------------------------------------------------------------------

  for (int iSensor = 0; iSensor < nPXDSensors; iSensor++) {
    for (int iDCD = 0; iDCD < nDCD; iDCD++) {
      for (int iSWB = 0; iSWB < nSWB; iSWB++) {
        VxdID id = gTools->getSensorIDFromPXDIndex(iSensor);
        auto layerNumber = id.getLayerNumber();
        auto ladderNumber = id.getLadderNumber();
        auto sensorNumber = id.getSensorNumber();
        string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % iDCD % iSWB);
        auto tree = new TTree(treename.c_str(), treename.c_str());
        tree->Branch<int>("sensorID", &m_sensorID);
        tree->Branch<int>("uCellID", &m_uCellID);
        tree->Branch<int>("vCellID", &m_vCellID);
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
  int nDCD = gTools->getNumberOfPXDUSideChips();
  int nSWB = gTools->getNumberOfPXDVSideChips();

  auto mc_counter = getObjectPtr<TH1I>("PXDMCCounter");
  auto data_counter = getObjectPtr<TH1I>("PXDDataCounter");

  for (auto& cluster :  m_pxdCluster) {

    // Apply cluster selection cuts
    if (cluster.getCharge() >= m_minClusterCharge && cluster.getSize() >= m_minClusterSize && cluster.getSize() <= m_maxClusterSize) {

      VxdID sensorID = cluster.getSensorID();
      const PXD::SensorInfo& Info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));

      // Compute variables from cluster needed for gain estimation
      m_isMC = m_simulatedDataFlag;
      m_sensorID = int(sensorID);
      m_uCellID = Info.getUCellID(cluster.getU());
      m_vCellID = Info.getVCellID(cluster.getV());
      m_signal = cluster.getCharge();

      // Fill variabels into the right tree
      auto iSensor = gTools->getPXDSensorIndex(sensorID);
      auto layerNumber = sensorID.getLayerNumber();
      auto ladderNumber = sensorID.getLadderNumber();
      auto sensorNumber = sensorID.getSensorNumber();
      auto iDCD = PXD::PXDMappingLookup::getDCDID(m_uCellID, m_vCellID, sensorID) - 1;
      auto iSWB = PXD::PXDMappingLookup::getSWBID(m_vCellID) - 1;
      string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % iDCD % iSWB);
      getObjectPtr<TTree>(treename)->Fill();

      // Increment the counters
      if (m_simulatedDataFlag) {
        mc_counter->Fill(iSensor * nDCD * nSWB + iDCD * nSWB + iSWB);
      } else {
        data_counter->Fill(iSensor * nDCD * nSWB + iDCD * nSWB + iSWB);
      }
    }
  }
}
