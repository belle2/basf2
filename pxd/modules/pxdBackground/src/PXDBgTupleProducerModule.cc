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
  setDescription("PXD background module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("integrationTime", m_integrationTime, "PXD integration time", m_integrationTime);
  addParam("outputDirectory", m_outputDirectoryName, "Name of output directory", m_outputDirectoryName);
  addParam("outputFileName", m_outputFileName, "Output file name", m_outputFileName);

  // initialize other private data members
  m_file = nullptr;
  m_treeBEAST = nullptr;
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

PXDBgTupleProducerModule::~PXDBgTupleProducerModule()
{
}

void PXDBgTupleProducerModule::initialize()
{
  //Register collections
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);
  StoreArray<PXDDigit> storeDigits(m_storeDigitsName);

  //Make sure the EventMetaData already exists.
  StoreObjPtr<EventMetaData>().isRequired();

  //Store names to speed up creation later
  m_storeDigitsName = storeDigits.getName();

  //Pointer to GeoTools instance
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfPXDLayers() == 0) {
    B2WARNING("Missing geometry for PXD, PXD-masking is skiped.");
  }

  m_integrationTime *= Unit::us;

  m_ts = 0;



  m_file = new TFile(m_outputFileName.c_str(), "RECREATE");
  m_treeBEAST = new TTree("tout", "BEAST data tree");
  m_treeBEAST->Branch("ts", &(m_ts));
  m_treeBEAST->Branch("run", &(m_run));
  m_treeBEAST->Branch("subrun", &(m_subrun));

  int nPXDSensors = gTools->getNumberOfPXDSensors();
  for (int i = 0; i < nPXDSensors; i++) {
    VxdID sensorID = gTools->getSensorIDFromPXDIndex(i);
    m_sensorData[sensorID] = SensorData();
    string sensorDescr = sensorID;
    m_treeBEAST->Branch(str(format("sensor_%1%_occupancy") % sensorDescr).c_str(), &(m_sensorData[sensorID].m_occupancy));
    m_treeBEAST->Branch(str(format("sensor_%1%_exposition") % sensorDescr).c_str(), &(m_sensorData[sensorID].m_expo));
    m_treeBEAST->Branch(str(format("sensor_%1%_dose") % sensorDescr).c_str(), &(m_sensorData[sensorID].m_dose));
    m_treeBEAST->Branch(str(format("sensor_%1%_softPhotonFlux") % sensorDescr).c_str(), &(m_sensorData[sensorID].m_softPhotonFlux));
    m_treeBEAST->Branch(str(format("sensor_%1%_hardPhotonFlux") % sensorDescr).c_str(), &(m_sensorData[sensorID].m_hardPhotonFlux));
    m_treeBEAST->Branch(str(format("sensor_%1%_chargedParticleFlux") % sensorDescr).c_str(),
                        &(m_sensorData[sensorID].m_chargedParticleFlux));
  }
}

void PXDBgTupleProducerModule::beginRun()
{


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

  // Initialize m_ts with current timestamp
  if (m_ts == 0) {
    m_ts = ts;
  }

  if (ts > m_ts) {
    // Write timestamp and background rates into TTree
    m_ts = ts;
    m_run =  eventMetaDataPtr->getRun();
    m_subrun = eventMetaDataPtr->getSubrun();

    m_treeBEAST->Fill();
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

    // Occupancy in fraction of fired pixels
    m_sensorData[currentSensorID].m_occupancy += 1.0 / currentSensorPixels;

    // Dose in Gy/smy, normalize by sensor mass
    m_sensorData[currentSensorID].m_dose +=
      (hitEnergy / Unit::J) / (currentSensorMass / 1000) * (1.0 / currentComponentTime);

    // Exposition in GeV/cm2/s
    m_sensorData[currentSensorID].m_expo += hitEnergy / currentSensorArea / (currentComponentTime / Unit::s);
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
      m_sensorData[currentSensorID].m_softPhotonFlux += 1.0 / currentSensorArea / (currentComponentTime / Unit::s);
    } else if (cluster.getSize() == 1 && clusterEnergy > 10000) {
      // Hard photon flux per cm and second
      m_sensorData[currentSensorID].m_hardPhotonFlux += 1.0 / currentSensorArea / (currentComponentTime / Unit::s);
    } else if (cluster.getSize() > 1 && clusterEnergy > 10000) {
      // Charged particle flux per cm and second
      m_sensorData[currentSensorID].m_chargedParticleFlux += 1.0 / currentSensorArea / (currentComponentTime / Unit::s);
    }
  }
}

void PXDBgTupleProducerModule::endRun()
{
}


void PXDBgTupleProducerModule::terminate()
{
  // Write output tuple
  m_file->cd();
  m_treeBEAST->Write();
  m_file->Close();
}
