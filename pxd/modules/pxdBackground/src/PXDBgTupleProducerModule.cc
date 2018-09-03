#include <pxd/modules/pxdBackground/PXDBgTupleProducerModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <pxd/reconstruction/PXDGainCalibrator.h>
#include <pxd/reconstruction/PXDPixelMasker.h>

#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <boost/format.hpp>

#include "TVector3.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TChain.h"

using namespace std;
using boost::format;
using namespace Belle2;
using namespace Belle2::PXD;



//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDBgTupleProducer)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDBgTupleProducerModule::PXDBgTupleProducerModule() :
  Module(), m_outputFileName("beast_tuple.root"),
  m_integrationTime(20)
{
  //Set module properties
  setDescription("PXD background tuple producer module");
  addParam("integrationTime", m_integrationTime, "PXD integration time in micro seconds", m_integrationTime);
  addParam("outputFileName", m_outputFileName, "Output file name", m_outputFileName);
}

const TVector3& PXDBgTupleProducerModule::pointToGlobal(VxdID sensorID, const TVector3& local)
{
  static TVector3 result(0, 0, 0);

  const PXD::SensorInfo& info = getInfo(sensorID);
  result = info.pointToGlobal(local);
  return result;
}

const TVector3& PXDBgTupleProducerModule::vectorToGlobal(VxdID sensorID, const TVector3& local)
{
  static TVector3 result(0, 0, 0);

  const PXD::SensorInfo& info = getInfo(sensorID);
  result = info.vectorToGlobal(local);
  return result;
}

PXDBgTupleProducerModule::~PXDBgTupleProducerModule() {}

void PXDBgTupleProducerModule::initialize()
{
  //Register collections
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);
  StoreArray<PXDDigit> storeDigits(m_storeDigitsName);

  //Make sure the EventMetaData already exists.
  StoreObjPtr<EventMetaData>().isRequired();

  //Store names to speed up creation later
  m_storeDigitsName = storeDigits.getName();

  // PXD integration time
  m_integrationTime *= Unit::us;

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
    // Start value for minOccupancy should be one not zero
    m_sensorData[sensorID].m_minOccupancy = 1.0;
  }
}

void PXDBgTupleProducerModule::beginRun()
{
  // Compute the sensitive area for all PXD sensors
  for (auto const& pair2 : m_sensorData) {
    auto const& sensorID = pair2.first;
    auto info = getInfo(sensorID);

    m_sensitivePixelMap[sensorID] = info.getUCells() * info.getVCells();
    m_sensitiveAreaMap[sensorID] = getSensorArea(sensorID);

    for (int ui = 0; ui < info.getUCells(); ++ui) {
      for (int vi = 0; vi < info.getVCells(); ++vi) {
        if (PXD::PXDPixelMasker::getInstance().pixelDead(sensorID, ui, vi)
            || !PXD::PXDPixelMasker::getInstance().pixelOK(sensorID, ui, vi)) {
          m_sensitivePixelMap[sensorID] -= 1;
          m_sensitiveAreaMap[sensorID] -= info.getVPitch(info.getVCellPosition(vi)) * info.getUPitch();
        }
      }
    }

    if (m_sensitivePixelMap[sensorID] == 0) {
      B2WARNING("All pixels from Sensor=" << sensorID << " are masked.");
    }
  }
}

void PXDBgTupleProducerModule::event()
{
  //Register collections
  const StoreArray<PXDCluster> storeClusters(m_storeClustersName);
  const StoreArray<PXDDigit> storeDigits(m_storeDigitsName);

  //Get the event meta data
  StoreObjPtr<EventMetaData> eventMetaDataPtr;

  // Compute the curent one second timestamp
  unsigned long long int ts = eventMetaDataPtr->getTime() / 1000000000;

  // If needed, add a new one second block to buffer
  auto iter = m_buffer.find(ts);
  if (iter == m_buffer.end()) {
    m_buffer[ts] = m_sensorData;
  }

  // Empty map for computing event wise occupancy
  std::map<VxdID, double> occupancyMap;

  for (const PXDDigit& storeDigit : storeDigits) {
    VxdID sensorID = storeDigit.getSensorID();
    double ADUToEnergy =  PXDGainCalibrator::getInstance().getADUToEnergy(sensorID, storeDigit.getUCellID(), storeDigit.getVCellID());
    double hitEnergy = storeDigit.getCharge() * ADUToEnergy;

    if (m_sensitivePixelMap[sensorID] != 0) {
      occupancyMap[sensorID] += 1.0 / m_sensitivePixelMap[sensorID];
    }
    m_buffer[ts][sensorID].m_dose += (hitEnergy / Unit::J);
    m_buffer[ts][sensorID].m_expo += hitEnergy;
  }

  for (auto& pair : m_buffer[ts]) {
    auto& sensorID = pair.first;
    auto& bgdata = pair.second;
    bgdata.m_run = eventMetaDataPtr->getRun();
    // Check if there is actually data for this sensor
    if (occupancyMap.find(sensorID) != occupancyMap.end()) {
      bgdata.m_nEvents += 1;
      bgdata.m_occupancy += occupancyMap[sensorID];
      if (occupancyMap[sensorID] > bgdata.m_maxOccupancy) {
        bgdata.m_maxOccupancy = occupancyMap[sensorID];
      }
      if (occupancyMap[sensorID] < bgdata.m_minOccupancy) {
        bgdata.m_minOccupancy = occupancyMap[sensorID];
      }
    }
  }

  for (const PXDCluster& cluster : storeClusters) {
    // Update if we have a new sensor
    VxdID sensorID = cluster.getSensorID();
    auto info = getInfo(sensorID);

    auto cluster_uID = info.getUCellID(cluster.getU());
    auto cluster_vID = info.getVCellID(cluster.getV());
    double ADUToEnergy =  PXDGainCalibrator::getInstance().getADUToEnergy(sensorID, cluster_uID, cluster_vID);
    double clusterEnergy = cluster.getCharge() * ADUToEnergy;

    if (cluster.getSize() == 1 && clusterEnergy < 10000 * Unit::eV && clusterEnergy > 6000 * Unit::eV) {
      m_buffer[ts][sensorID].m_softPhotonFlux += 1.0;
    } else if (cluster.getSize() == 1 && clusterEnergy > 10000 * Unit::eV) {
      m_buffer[ts][sensorID].m_hardPhotonFlux += 1.0;
    } else if (cluster.getSize() > 1 && clusterEnergy > 10000 * Unit::eV) {
      m_buffer[ts][sensorID].m_chargedParticleFlux += 1.0;
    }
  }
}

void PXDBgTupleProducerModule::endRun() {}


void PXDBgTupleProducerModule::terminate()
{
  TFile* rfile = new TFile(m_outputFileName.c_str(), "RECREATE");
  TTree* treeBEAST = new TTree("tout", "BEAST data tree");

  unsigned long long int ts = 0;
  treeBEAST->Branch("ts", &(ts));

  for (auto&   pair : m_sensorData) {
    auto& sensorID = pair.first;
    auto& bgdata = pair.second;
    string sensorDescr = sensorID;
    treeBEAST->Branch(str(format("pxd_%1%_run") % sensorDescr).c_str(), &(bgdata.m_run));
    treeBEAST->Branch(str(format("pxd_%1%_nEvents") % sensorDescr).c_str(), &(bgdata.m_nEvents));
    treeBEAST->Branch(str(format("pxd_%1%_minOccupancy") % sensorDescr).c_str(), &(bgdata.m_minOccupancy));
    treeBEAST->Branch(str(format("pxd_%1%_maxOccupancy") % sensorDescr).c_str(), &(bgdata.m_maxOccupancy));
    treeBEAST->Branch(str(format("pxd_%1%_occupancy") % sensorDescr).c_str(), &(bgdata.m_occupancy));
    treeBEAST->Branch(str(format("pxd_%1%_exposition") % sensorDescr).c_str(), &(bgdata.m_expo));
    treeBEAST->Branch(str(format("pxd_%1%_dose") % sensorDescr).c_str(), &(bgdata.m_dose));
    treeBEAST->Branch(str(format("pxd_%1%_softPhotonFlux") % sensorDescr).c_str(), &(bgdata.m_softPhotonFlux));
    treeBEAST->Branch(str(format("pxd_%1%_hardPhotonFlux") % sensorDescr).c_str(), &(bgdata.m_hardPhotonFlux));
    treeBEAST->Branch(str(format("pxd_%1%_chargedParticleFlux") % sensorDescr).c_str(),
                      &(bgdata.m_chargedParticleFlux));
  }

  // Write timestamp and background rates into TTree
  for (auto const& pair1 : m_buffer) {
    auto const& timestamp = pair1.first;
    auto const& sensors = pair1.second;

    // Set variables for dumping into tree
    ts = timestamp;
    for (auto const& pair2 : sensors) {
      auto const& sensorID = pair2.first;
      auto const& bgdata = pair2.second;
      double currentComponentTime = bgdata.m_nEvents * m_integrationTime;
      const PXD::SensorInfo& info = getInfo(sensorID);
      double currentSensorMass = m_sensitiveAreaMap[sensorID] * info.getThickness() * c_densitySi;
      double currentSensorArea = m_sensitiveAreaMap[sensorID];
      m_sensorData[sensorID] = bgdata;
      // Some bg rates are still in wrong units. We have to fix this now.
      m_sensorData[sensorID].m_occupancy = bgdata.m_occupancy / bgdata.m_nEvents;

      if (currentSensorArea > 0) {
        m_sensorData[sensorID].m_dose *= (1.0 / currentComponentTime) * (1000 / currentSensorMass);
        m_sensorData[sensorID].m_expo *= (1.0 / currentSensorArea) * (1.0  / (currentComponentTime / Unit::s));
        m_sensorData[sensorID].m_softPhotonFlux *= (1.0 / currentSensorArea) * (1.0 / (currentComponentTime / Unit::s));
        m_sensorData[sensorID].m_hardPhotonFlux *= (1.0 / currentSensorArea) * (1.0 / (currentComponentTime / Unit::s));
        m_sensorData[sensorID].m_chargedParticleFlux *= (1.0 / currentSensorArea) * (1.0 / (currentComponentTime / Unit::s));
      }
    }

    // Dump variables into tree
    treeBEAST->Fill();
  }

  // Write output tuple
  rfile->cd();
  treeBEAST->Write();
  rfile->Close();
}
