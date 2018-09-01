#include <pxd/modules/pxdBackground/PXDBgTupleProducerModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <framework/core/InputController.h>
#include <pxd/reconstruction/PXDGainCalibrator.h>

#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <cmath>
#include <fstream>
#include <set>
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
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("integrationTime", m_integrationTime, "PXD integration time", m_integrationTime);
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
  for (int i = 0; i < nPXDSensors; i++) {
    VxdID sensorID = gTools->getSensorIDFromPXDIndex(i);
    m_sensorData[sensorID] = SensorData();
  }
}

void PXDBgTupleProducerModule::beginRun() {}

void PXDBgTupleProducerModule::event()
{
  //Register collections
  const StoreArray<PXDCluster> storeClusters(m_storeClustersName);
  const StoreArray<PXDDigit> storeDigits(m_storeDigitsName);

  //Get the event meta data
  StoreObjPtr<EventMetaData> eventMetaDataPtr;

  // Compute the curent one second timestamp
  unsigned long long int ts = eventMetaDataPtr->getTime() / 1000000000;

  auto iter = m_buffer.find(ts);
  if (iter == m_buffer.end()) {
    m_buffer[ts] = m_sensorData;
  }

  for (auto&   pair : m_buffer[ts]) {
    auto& bgdata = pair.second;
    bgdata.m_run = eventMetaDataPtr->getRun();
    bgdata.m_nEvents += 1;
  }

  // We accumulate data in one second blocks
  VxdID currentSensorID(0);
  int currentSensorPixels(0);

  for (const PXDDigit& storeDigit : storeDigits) {
    VxdID sensorID = storeDigit.getSensorID();
    if (sensorID != currentSensorID) {
      currentSensorID = sensorID;
      auto info = getInfo(sensorID);
      currentSensorPixels = info.getUCells() * info.getVCells();
    }
    double ADUToEnergy =  PXDGainCalibrator::getInstance().getADUToEnergy(sensorID, storeDigit.getUCellID(), storeDigit.getVCellID());
    double hitEnergy = storeDigit.getCharge() * ADUToEnergy;

    m_buffer[ts][currentSensorID].m_occupancy += 1.0 / currentSensorPixels;
    m_buffer[ts][currentSensorID].m_dose += (hitEnergy / Unit::J);
    m_buffer[ts][currentSensorID].m_expo += hitEnergy;
  }

  for (const PXDCluster& cluster : storeClusters) {
    // Update if we have a new sensor
    VxdID sensorID = cluster.getSensorID();
    auto info = getInfo(sensorID);
    if (sensorID != currentSensorID) {
      currentSensorID = sensorID;
    }

    auto cluster_uID = info.getUCellID(cluster.getU());
    auto cluster_vID = info.getVCellID(cluster.getV());
    double ADUToEnergy =  PXDGainCalibrator::getInstance().getADUToEnergy(sensorID, cluster_uID, cluster_vID);
    double clusterEnergy = cluster.getCharge() * ADUToEnergy;

    if (cluster.getSize() == 1 && clusterEnergy < 10000 * Unit::eV && clusterEnergy > 6000 * Unit::eV) {
      m_buffer[ts][currentSensorID].m_softPhotonFlux += 1.0;
    } else if (cluster.getSize() == 1 && clusterEnergy > 10000 * Unit::eV) {
      m_buffer[ts][currentSensorID].m_hardPhotonFlux += 1.0;
    } else if (cluster.getSize() > 1 && clusterEnergy > 10000 * Unit::eV) {
      m_buffer[ts][currentSensorID].m_chargedParticleFlux += 1.0;
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
    treeBEAST->Branch(str(format("sensor_%1%_run") % sensorDescr).c_str(), &(bgdata.m_run));
    treeBEAST->Branch(str(format("sensor_%1%_nEvents") % sensorDescr).c_str(), &(bgdata.m_nEvents));
    treeBEAST->Branch(str(format("sensor_%1%_occupancy") % sensorDescr).c_str(), &(bgdata.m_occupancy));
    treeBEAST->Branch(str(format("sensor_%1%_exposition") % sensorDescr).c_str(), &(bgdata.m_expo));
    treeBEAST->Branch(str(format("sensor_%1%_dose") % sensorDescr).c_str(), &(bgdata.m_dose));
    treeBEAST->Branch(str(format("sensor_%1%_softPhotonFlux") % sensorDescr).c_str(), &(bgdata.m_softPhotonFlux));
    treeBEAST->Branch(str(format("sensor_%1%_hardPhotonFlux") % sensorDescr).c_str(), &(bgdata.m_hardPhotonFlux));
    treeBEAST->Branch(str(format("sensor_%1%_chargedParticleFlux") % sensorDescr).c_str(),
                      &(bgdata.m_chargedParticleFlux));
  }

  // Compute the first second of the run
  //unsigned long long int ts_run_start = m_buffer.begin()->first;

  // Write timestamp and background rates into TTree
  for (auto const& pair1 : m_buffer) {
    auto const& timestamp = pair1.first;
    auto const& sensors = pair1.second;

    // Set variables for dumping into tree
    ts = timestamp; // - ts_run_start;
    for (auto const& pair2 : sensors) {
      auto const& sensorID = pair2.first;
      auto const& bgdata = pair2.second;
      double currentComponentTime = bgdata.m_nEvents * m_integrationTime;
      double currentSensorMass = getSensorMass(sensorID);
      double currentSensorArea = getSensorArea(sensorID);
      m_sensorData[sensorID] = bgdata;
      // Some bg rates are still in wrong units. We have to fix this now.
      m_sensorData[sensorID].m_occupancy = bgdata.m_occupancy / bgdata.m_nEvents;
      m_sensorData[sensorID].m_dose *= (1.0 / currentComponentTime) * (1000 / currentSensorMass);
      m_sensorData[sensorID].m_expo *= (1.0 / currentSensorArea) * (1.0  / (currentComponentTime / Unit::s));
      m_sensorData[sensorID].m_softPhotonFlux *= (1.0 / currentSensorArea) * (1.0 / (currentComponentTime / Unit::s));
      m_sensorData[sensorID].m_hardPhotonFlux *= (1.0 / currentSensorArea) * (1.0 / (currentComponentTime / Unit::s));
      m_sensorData[sensorID].m_chargedParticleFlux *= (1.0 / currentSensorArea) * (1.0 / (currentComponentTime / Unit::s));
    }

    // Dump variables into tree
    treeBEAST->Fill();
  }

  // Write output tuple
  rfile->cd();
  treeBEAST->Write();
  rfile->Close();
}
