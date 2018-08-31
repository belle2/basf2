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
  Module(), m_outputDirectoryName(""),  m_outputFileName("beast_tuple.root"),
  m_integrationTime(20)
{
  //Set module properties
  setDescription("PXD background tuple producer module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("integrationTime", m_integrationTime, "PXD integration time", m_integrationTime);
  addParam("outputDirectory", m_outputDirectoryName, "Name of output directory", m_outputDirectoryName);
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
  double currentComponentTime = 1 * Unit::s;
  VxdID currentSensorID(0);
  double currentSensorMass(0);
  double currentSensorArea(0);
  int currentSensorPixels(0);
  double currentSensorADCUnit(0);
  double currentSensorGq(0);

  for (const PXDDigit& storeDigit : storeDigits) {
    VxdID sensorID = storeDigit.getSensorID();
    if (sensorID != currentSensorID) {
      currentSensorID = sensorID;
      currentSensorMass = getSensorMass(currentSensorID);
      currentSensorArea = getSensorArea(currentSensorID);
      auto info = getInfo(sensorID);
      currentSensorPixels = info.getUCells() * info.getVCells();
      currentSensorADCUnit = 130.0;
      currentSensorGq = 0.6;
    }

    double gain = PXDGainCalibrator::getInstance().getGainCorrection(sensorID, storeDigit.getUCellID(), storeDigit.getVCellID());
    double hitEnergy = (currentSensorADCUnit / currentSensorGq / gain) * storeDigit.getCharge() * Const::ehEnergy;

    m_buffer[ts][currentSensorID].m_occupancy += 1.0 / currentSensorPixels;

    m_buffer[ts][currentSensorID].m_dose +=
      (hitEnergy / Unit::J) / (currentSensorMass / 1000) * (1.0 / currentComponentTime);

    m_buffer[ts][currentSensorID].m_expo += hitEnergy / currentSensorArea / (currentComponentTime / Unit::s);
  }

  for (const PXDCluster& cluster : storeClusters) {
    // Update if we have a new sensor
    VxdID sensorID = cluster.getSensorID();
    auto info = getInfo(sensorID);
    if (sensorID != currentSensorID) {
      currentSensorID = sensorID;
      currentSensorArea = getSensorArea(currentSensorID);
      currentSensorADCUnit = 130.0;
      currentSensorGq = 0.6;
    }

    auto cluster_uID = info.getUCellID(cluster.getU());
    auto cluster_vID = info.getVCellID(cluster.getV());
    double gain = PXDGainCalibrator::getInstance().getGainCorrection(sensorID, cluster_uID, cluster_vID);
    double clusterEnergy = (currentSensorADCUnit / currentSensorGq / gain) * cluster.getCharge() * Const::ehEnergy;

    if (cluster.getSize() == 1 && clusterEnergy < 10000 && clusterEnergy > 6000) {
      // Soft photon flux per cm and second
      m_buffer[ts][currentSensorID].m_softPhotonFlux += 1.0 / currentSensorArea / (currentComponentTime / Unit::s);
    } else if (cluster.getSize() == 1 && clusterEnergy > 10000) {
      // Hard photon flux per cm and second
      m_buffer[ts][currentSensorID].m_hardPhotonFlux += 1.0 / currentSensorArea / (currentComponentTime / Unit::s);
    } else if (cluster.getSize() > 1 && clusterEnergy > 10000) {
      // Charged particle flux per cm and second
      m_buffer[ts][currentSensorID].m_chargedParticleFlux += 1.0 / currentSensorArea / (currentComponentTime / Unit::s);
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
  unsigned long long int ts_run_start = m_buffer.begin()->first;

  // Write timestamp and background rates into TTree
  for (auto const& pair1 : m_buffer) {
    auto const& timestamp = pair1.first;
    auto const& sensors = pair1.second;

    B2INFO("Write time block :" << timestamp);

    for (auto const& pair2 : sensors) {
      auto const& sensorID = pair2.first;
      auto const& bgdata = pair2.second;

      B2INFO("sensor: " << sensorID);
      B2INFO("occupanvy: " << bgdata.m_occupancy);
      B2INFO("expo: " << bgdata.m_expo);
      B2INFO("dose: " << bgdata.m_dose);
      B2INFO("photons: " << bgdata.m_softPhotonFlux);
      B2INFO("mixed: " << bgdata.m_hardPhotonFlux);
      B2INFO("charged: " << bgdata.m_chargedParticleFlux);
    }

    // Set variables for dumping into tree
    ts = timestamp - ts_run_start;
    for (auto const& pair2 : sensors) {
      auto const& sensorID = pair2.first;
      auto const& bgdata = pair2.second;
      m_sensorData[sensorID] = bgdata;
      m_sensorData[sensorID].m_occupancy = bgdata.m_occupancy / bgdata.m_nEvents;
    }

    // Dump variables into tree
    treeBEAST->Fill();
  }

  // Write output tuple
  rfile->cd();
  treeBEAST->Write();
  rfile->Close();
}
