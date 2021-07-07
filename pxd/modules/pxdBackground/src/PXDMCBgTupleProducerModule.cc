/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/



#include <pxd/modules/pxdBackground/PXDMCBgTupleProducerModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/BackgroundMetaData.h>
#include <pxd/reconstruction/PXDGainCalibrator.h>
#include <pxd/reconstruction/PXDPixelMasker.h>

#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <boost/format.hpp>

#include <TFile.h>
#include <TTree.h>

using namespace std;
using boost::format;
using namespace Belle2;
using namespace Belle2::PXD;



//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDMCBgTupleProducer)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDMCBgTupleProducerModule::PXDMCBgTupleProducerModule() : Module()
  , m_hasPXDData(false), m_componentTime(0)
{
  //Set module properties
  setDescription("PXD background tuple producer module");
  addParam("integrationTime", m_integrationTime, "PXD integration time in micro seconds", double(20));
  addParam("outputFileName", m_outputFileName, "Output file name", string("beast_tuple.root"));
  addParam("maskDeadPixels", m_maskDeadPixels, "Correct bg rates by known dead pixels", bool(true));
  addParam("nBinsU", m_nBinsU, "Number of regions per sensor along u side", int(1));
  addParam("nBinsV", m_nBinsV, "Number of regions per sensor along v side", int(6));
  addParam("overrideComponentTime", m_overrideComponentTime, "User specified component time in micro seconds", double(0.0));
}



void PXDMCBgTupleProducerModule::initialize()
{
  //Register collections
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);
  StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  StoreObjPtr<BackgroundMetaData> storeBgMetaData(m_storeBgMetaDataName, DataStore::c_Persistent);

  //Store names to speed up creation later
  m_storeDigitsName = storeDigits.getName();
  m_storeBgMetaDataName = storeBgMetaData.getName();

  // PXD integration time
  m_integrationTime *= Unit::us;

  m_overrideComponentTime *= Unit::us;

  // So far, we did not see PXD data
  m_hasPXDData = false;

  //Pointer to GeoTools instance
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfPXDLayers() == 0) {
    B2WARNING("Missing geometry for PXD, PXD-masking is skiped.");
  }
  int nPXDSensors = gTools->getNumberOfPXDSensors();

  // Initialize m_sensorData with empty sensorData for all sensors
  for (int i = 0; i < nPXDSensors; i++) {
    VxdID sensorID = gTools->getSensorIDFromPXDIndex(i);
    m_sensorData[sensorID] = SensorData();
    // Initialize counters for subdivisions per sensor
    m_sensorData[sensorID].m_regionExpoMap = vector<double>(m_nBinsU * m_nBinsV, 0);
    m_sensorData[sensorID].m_regionDoseMap = vector<double>(m_nBinsU * m_nBinsV, 0);
    m_sensorData[sensorID].m_regionSoftPhotonFluxMap = vector<double>(m_nBinsU * m_nBinsV, 0);
    m_sensorData[sensorID].m_regionChargedParticleFluxMap = vector<double>(m_nBinsU * m_nBinsV, 0);
    m_sensorData[sensorID].m_regionHardPhotonFluxMap = vector<double>(m_nBinsU * m_nBinsV, 0);
  }
}

void PXDMCBgTupleProducerModule::beginRun()
{
  // Compute the sensitive area for all PXD sensors
  for (auto const& pair2 : m_sensorData) {
    auto const& sensorID = pair2.first;
    auto info = getInfo(sensorID);

    // Compute nominal number of pixel per sensor
    m_sensitivePixelMap[sensorID] = info.getUCells() * info.getVCells();
    // Compute nominal area per sensor
    m_sensitiveAreaMap[sensorID] = getSensorArea(sensorID);

    for (int uBin = 0; uBin < m_nBinsU; ++uBin) {
      for (int vBin = 0; vBin < m_nBinsV; ++vBin)  {
        std::pair<VxdID, int> key(sensorID, getRegionID(uBin, vBin));
        // Compute nominal number of pixel per sensor subregion
        m_regionSensitivePixelMap[key] = info.getUCells() * info.getVCells() / m_nBinsU / m_nBinsV;
        // Compute nominal area per sensor subregion
        m_regionSensitiveAreaMap[key] = getRegionArea(sensorID, vBin);
      }
    }

    if (m_maskDeadPixels) {
      for (int ui = 0; ui < info.getUCells(); ++ui) {
        for (int vi = 0; vi < info.getVCells(); ++vi) {
          if (PXD::PXDPixelMasker::getInstance().pixelDead(sensorID, ui, vi)
              || !PXD::PXDPixelMasker::getInstance().pixelOK(sensorID, ui, vi)) {
            m_sensitivePixelMap[sensorID] -= 1;
            m_sensitiveAreaMap[sensorID] -= info.getVPitch(info.getVCellPosition(vi)) * info.getUPitch();
            int uBin = PXDGainCalibrator::getInstance().getBinU(sensorID, ui, vi, m_nBinsU);
            int vBin = PXDGainCalibrator::getInstance().getBinV(sensorID, vi, m_nBinsV);
            std::pair<VxdID, int> key(sensorID, getRegionID(uBin, vBin));
            m_regionSensitivePixelMap[key] -= 1;
            m_regionSensitiveAreaMap[key] -= info.getVPitch(info.getVCellPosition(vi)) * info.getUPitch();
          }
        }
      }
    }

    if (m_sensitivePixelMap[sensorID] == 0) {
      B2WARNING("All pixels from Sensor=" << sensorID << " are masked.");
    }

    for (int uBin = 0; uBin < m_nBinsU; ++uBin) {
      for (int vBin = 0; vBin < m_nBinsV; ++vBin)  {
        std::pair<VxdID, int> key(sensorID, getRegionID(uBin, vBin));
        if (m_regionSensitivePixelMap[key] == 0) {
          B2WARNING("All pixels from subregion uBin=" << uBin << " vBin=" << vBin << " on Sensor=" << sensorID << " are masked.");
        }
      }
    }

  }
}

void PXDMCBgTupleProducerModule::event()
{
  //Register collections
  const StoreArray<PXDCluster> storeClusters(m_storeClustersName);
  const StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  const StoreObjPtr<BackgroundMetaData> storeBgMetaData(m_storeBgMetaDataName, DataStore::c_Persistent);

  // Set the real time
  m_componentTime = storeBgMetaData->getRealTime();

  // Empty map for computing event wise occupancy
  std::map<VxdID, double> occupancyMap;

  // Check if there is PXD data
  if (storeDigits.getEntries() > 0) {
    m_hasPXDData = true;
  }

  for (const PXDDigit& storeDigit : storeDigits) {
    VxdID sensorID = storeDigit.getSensorID();
    double ADUToEnergy =  PXDGainCalibrator::getInstance().getADUToEnergy(sensorID, storeDigit.getUCellID(), storeDigit.getVCellID());
    double hitEnergy = storeDigit.getCharge() * ADUToEnergy;

    if (m_sensitivePixelMap[sensorID] != 0) {
      occupancyMap[sensorID] += 1.0 / m_sensitivePixelMap[sensorID];
    }
    m_sensorData[sensorID].m_dose += (hitEnergy / Unit::J);
    m_sensorData[sensorID].m_expo += hitEnergy;

    int uBin = PXDGainCalibrator::getInstance().getBinU(sensorID, storeDigit.getUCellID(), storeDigit.getVCellID(), m_nBinsU);
    int vBin = PXDGainCalibrator::getInstance().getBinV(sensorID, storeDigit.getVCellID(), m_nBinsV);
    int regionID = getRegionID(uBin, vBin);
    m_sensorData[sensorID].m_regionDoseMap[regionID] += (hitEnergy / Unit::J);
    m_sensorData[sensorID].m_regionExpoMap[regionID] += hitEnergy;
  }

  for (auto& pair : m_sensorData) {
    auto& sensorID = pair.first;
    auto& bgdata = pair.second;

    // Check if there is actually data for this sensor
    if (occupancyMap.find(sensorID) != occupancyMap.end()) {
      bgdata.m_meanOccupancy += occupancyMap[sensorID];
    }
  }

  for (const PXDCluster& cluster : storeClusters) {
    // Update if we have a new sensor
    VxdID sensorID = cluster.getSensorID();
    auto info = getInfo(sensorID);

    auto cluster_uID = info.getUCellID(cluster.getU());
    auto cluster_vID = info.getVCellID(cluster.getV());
    int uBin = PXDGainCalibrator::getInstance().getBinU(sensorID, cluster_uID, cluster_vID, m_nBinsU);
    int vBin = PXDGainCalibrator::getInstance().getBinV(sensorID, cluster_vID, m_nBinsV);
    int regionID = getRegionID(uBin, vBin);
    double ADUToEnergy =  PXDGainCalibrator::getInstance().getADUToEnergy(sensorID, cluster_uID, cluster_vID);
    double clusterEnergy = cluster.getCharge() * ADUToEnergy;

    if (cluster.getSize() == 1 && clusterEnergy < 10000 * Unit::eV && clusterEnergy > 6000 * Unit::eV) {
      m_sensorData[sensorID].m_softPhotonFlux += 1.0;
      m_sensorData[sensorID].m_regionSoftPhotonFluxMap[regionID] += 1.0;
    } else if (cluster.getSize() == 1 && clusterEnergy > 10000 * Unit::eV) {
      m_sensorData[sensorID].m_hardPhotonFlux += 1.0;
      m_sensorData[sensorID].m_regionHardPhotonFluxMap[regionID] += 1.0;
    } else if (cluster.getSize() > 1 && clusterEnergy > 10000 * Unit::eV) {
      m_sensorData[sensorID].m_chargedParticleFlux += 1.0;
      m_sensorData[sensorID].m_regionChargedParticleFluxMap[regionID] += 1.0;
    }
  }
}

void PXDMCBgTupleProducerModule::terminate()
{
  // Create beast tuple
  if (m_hasPXDData) {
    TFile* rfile = new TFile(m_outputFileName.c_str(), "RECREATE");
    TTree* treeBEAST = new TTree("tout", "BEAST data tree");

    double currentComponentTime = m_componentTime;
    if (m_overrideComponentTime > 0.0) currentComponentTime = m_overrideComponentTime;

    B2RESULT("Total real time is " << currentComponentTime / Unit::us << " microseconds.");
    B2RESULT("This is equivalent to  " << currentComponentTime / m_integrationTime << " random triggered events.");

    for (auto&   pair : m_sensorData) {
      auto& sensorID = pair.first;
      auto& bgdata = pair.second;
      const PXD::SensorInfo& info = getInfo(sensorID);
      double currentSensorMass = m_sensitiveAreaMap[sensorID] * info.getThickness() * c_densitySi;
      double currentSensorArea = m_sensitiveAreaMap[sensorID];

      // Finalize computation of rates
      m_sensorData[sensorID].m_meanOccupancy = bgdata.m_meanOccupancy * (m_integrationTime / currentComponentTime);

      if (currentSensorArea > 0) {
        m_sensorData[sensorID].m_dose *= (1.0 / (currentComponentTime / Unit::s)) * (1000 / currentSensorMass);
        m_sensorData[sensorID].m_expo *= (1.0 / currentSensorArea) * (1.0  / (currentComponentTime / Unit::s));
        m_sensorData[sensorID].m_softPhotonFlux *= (1.0 / currentSensorArea) * (1.0 / (currentComponentTime / Unit::s));
        m_sensorData[sensorID].m_hardPhotonFlux *= (1.0 / currentSensorArea) * (1.0 / (currentComponentTime / Unit::s));
        m_sensorData[sensorID].m_chargedParticleFlux *= (1.0 / currentSensorArea) * (1.0 / (currentComponentTime / Unit::s));

        for (int regionID = 0; regionID < m_nBinsU * m_nBinsV; ++regionID) {
          std::pair<VxdID, int> key(sensorID, regionID);
          double currentRegionMass = m_regionSensitiveAreaMap[key] * info.getThickness() * c_densitySi;
          double currentRegionArea = m_regionSensitiveAreaMap[key];
          if (currentRegionArea > 0) {
            m_sensorData[sensorID].m_regionDoseMap[regionID] *= (1.0 / currentComponentTime) * (1000 / currentRegionMass);
            m_sensorData[sensorID].m_regionExpoMap[regionID] *= (1.0 / currentRegionArea) * (1.0  / (currentComponentTime / Unit::s));
            m_sensorData[sensorID].m_regionSoftPhotonFluxMap[regionID] *= (1.0 / currentRegionArea) * (1.0 / (currentComponentTime / Unit::s));
            m_sensorData[sensorID].m_regionHardPhotonFluxMap[regionID] *= (1.0 / currentRegionArea) * (1.0 / (currentComponentTime / Unit::s));
            m_sensorData[sensorID].m_regionChargedParticleFluxMap[regionID] *= (1.0 / currentRegionArea) * (1.0 /
                (currentComponentTime / Unit::s));
          }
        }
      }

      // Prepare output tree
      string sensorDescr = sensorID;
      treeBEAST->Branch(str(format("pxd_%1%_meanOccupancy") % sensorDescr).c_str(), &(bgdata.m_meanOccupancy));
      treeBEAST->Branch(str(format("pxd_%1%_exposition") % sensorDescr).c_str(), &(bgdata.m_expo));
      treeBEAST->Branch(str(format("pxd_%1%_dose") % sensorDescr).c_str(), &(bgdata.m_dose));
      treeBEAST->Branch(str(format("pxd_%1%_softPhotonFlux") % sensorDescr).c_str(), &(bgdata.m_softPhotonFlux));
      treeBEAST->Branch(str(format("pxd_%1%_hardPhotonFlux") % sensorDescr).c_str(), &(bgdata.m_hardPhotonFlux));
      treeBEAST->Branch(str(format("pxd_%1%_chargedParticleFlux") % sensorDescr).c_str(),
                        &(bgdata.m_chargedParticleFlux));

      for (int uBin = 0; uBin < m_nBinsU; ++uBin) {
        for (int vBin = 0; vBin < m_nBinsV; ++vBin)  {
          int regionID = getRegionID(uBin, vBin);
          treeBEAST->Branch(str(format("pxd_%1%_region_%2%_%3%_exposition") % sensorDescr % uBin % vBin).c_str(),
                            &(bgdata.m_regionExpoMap[regionID]));
          treeBEAST->Branch(str(format("pxd_%1%_region_%2%_%3%_dose") % sensorDescr % uBin % vBin).c_str(),
                            &(bgdata.m_regionDoseMap[regionID]));
          treeBEAST->Branch(str(format("pxd_%1%_region_%2%_%3%_softPhotonFlux") % sensorDescr % uBin % vBin).c_str(),
                            &(bgdata.m_regionSoftPhotonFluxMap[regionID]));
          treeBEAST->Branch(str(format("pxd_%1%_region_%2%_%3%_hardPhotonFlux") % sensorDescr % uBin % vBin).c_str(),
                            &(bgdata.m_regionHardPhotonFluxMap[regionID]));
          treeBEAST->Branch(str(format("pxd_%1%_region_%2%_%3%_chargedParticleFlux") % sensorDescr % uBin % vBin).c_str(),
                            &(bgdata.m_regionChargedParticleFluxMap[regionID]));
        }
      }
    }

    // Dump variables into tree
    treeBEAST->Fill();
    // Write output tuple
    rfile->cd();
    treeBEAST->Write();
    rfile->Close();
  }
}
