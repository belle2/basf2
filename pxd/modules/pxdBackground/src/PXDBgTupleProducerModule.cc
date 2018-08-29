#include <pxd/modules/pxdBackground/PXDBgTupleProducerModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <framework/core/InputController.h>

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

  m_integrationTime *= Unit::us;

  m_ts = 0;


  m_file = new TFile(m_outputFileName.c_str(), "RECREATE");
  m_treeBEAST = new TTree("tout", "BEAST data tree");
  m_treeBEAST->Branch("ts", &(m_ts));
  m_treeBEAST->Branch("run", &(m_run));
  m_treeBEAST->Branch("subrun", &(m_subrun));

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

  // unsigned long numberOfEvents = storeFileMetaData->getNEvents();
  double currentComponentTime = 0;

  VxdID currentSensorID(0);
  double currentSensorThickness(0);
  double currentSensorMass(0);
  double currentSensorArea(0);

  // Exposition and dose

  B2DEBUG(100, "Expo and dose");
  currentSensorID.setID(0);
  for (const PXDCluster& hit : storeClusters) {
    // Update if we have a new sensor
    VxdID sensorID = hit.getSensorID();
    if (sensorID != currentSensorID) {
      currentSensorID = sensorID;
      currentSensorThickness = getSensorThickness(currentSensorID);
      currentSensorMass = getSensorMass(currentSensorID);
      currentSensorArea = getSensorArea(currentSensorID);
    }
    double hitEnergy = hit.getCharge() * Const::ehEnergy;  // hit.getElectrons() * Const::ehEnergy;

    /*
    gain = GainCalibrator.getGainCorrection(vxd_id, self.data.cluster_uID, self.data.cluster_vID)
            self.data.cluster_energy = (3.65 * ADCUnit / Gq / gain ) * ( cluster.getCharge() + ROOT.gRandom.Gaus(0.0,0.5) )
    */

    // Dose in Gy/smy, normalize by sensor mass
    m_sensorData[currentSensorID].m_dose +=
      (hitEnergy / Unit::J) / (currentSensorMass / 1000) * (1.0 / currentComponentTime);
    // Exposition in GeV/cm2/s
    m_sensorData[currentSensorID].m_expo += hitEnergy / currentSensorArea / (currentComponentTime / Unit::s);


    const TVector3 localPos(hit.getU(), hit.getV(), 0.0);
    const TVector3 globalPos = pointToGlobal(currentSensorID, localPos);
    float globalPosXYZ[3];
    globalPos.GetXYZ(globalPosXYZ);
    /*
    storeEnergyDeposits.appendNew(
      sensorID.getLayerNumber(), sensorID.getLadderNumber(), sensorID.getSensorNumber(),
          hit.getPDGcode(), hit.getGlobalTime(),
      localPos.X(), localPos.Y(), globalPosXYZ, hitEnergy,
      (hitEnergy / Unit::J) / (currentSensorMass / 1000) / (currentComponentTime / Unit::s),
      (hitEnergy / Unit::J) / currentSensorArea / (currentComponentTime / Unit::s)
    );
    */
  }




  // Occupancy

  B2DEBUG(100, "Fired pixels");
  currentSensorID.setID(0);
  double currentSensorCut = 0;
  // Store fired pixels: count number of digits over threshold
  std::map<VxdID, std::vector<float> > firedPixels;
  for (const PXDDigit& storeDigit : storeDigits) {
    // Filter out digits with signals below zero-suppression threshold
    // ARE THRE SUCH DIGITS?
    VxdID sensorID = storeDigit.getSensorID();
    if (sensorID != currentSensorID) {
      currentSensorID = sensorID;
      auto info = getInfo(sensorID);
      currentSensorCut = info.getChargeThreshold();
    }
    B2DEBUG(30, "Digit charge: " << storeDigit.getCharge() << " threshold: " << currentSensorCut);
    if (storeDigit.getCharge() <  currentSensorCut) continue;
    B2DEBUG(30, "Passed.");
    firedPixels[sensorID].push_back(storeDigit.getCharge());
  }
  // Process the map
  for (auto idAndSet : firedPixels) {
    VxdID sensorID = idAndSet.first;
    double sensorArea = getSensorArea(sensorID);
    int nFired = idAndSet.second.size();
    double fired = nFired / (currentComponentTime / Unit::s) / sensorArea;
    //m_sensorData[sensorID].m_fired += fired;
  }

  B2DEBUG(100, "Occupancy");
  currentSensorID.setID(0);
  int nPixels = 0;
  for (auto cluster : storeClusters) {
    VxdID sensorID = cluster.getSensorID();
    if (currentSensorID != sensorID) {
      currentSensorID = sensorID;
      auto info = getInfo(sensorID);
      nPixels = info.getUCells() * info.getVCells();
    }

    double w_acceptance =  m_integrationTime / currentComponentTime;
    double occupancy = 1.0 / nPixels * cluster.getSize();
    m_sensorData[sensorID].m_occupancy +=  w_acceptance * occupancy;

    /*
    storeOccupancyEvents.appendNew(
      sensorID.getLayerNumber(), sensorID.getLadderNumber(),
      sensorID.getSensorNumber(),
      cluster.getU(), cluster.getV(), cluster.getSize(),
      cluster.getCharge(), occupancy
    );
    */
  }
}

void PXDBgTupleProducerModule::endRun()
{
}


void PXDBgTupleProducerModule::terminate()
{
  // Write out m_data
  ofstream outfile;
  string outfileName(m_outputDirectoryName + "_summary.txt");
  outfile.open(outfileName.c_str(), ios::out | ios::trunc);
  outfile << "component_name\t"
          << "component_time\t"
          << "layer\t"
          << "ladder\t"
          << "sensor\t"
          << "dose\t"
          << "expo\t"
          << "fired\t"
          << "occupancy"
          << endl;
  for (auto vxdSensor : m_sensorData) {
    outfile << vxdSensor.first.getLayerNumber() << "\t"
            << vxdSensor.first.getLadderNumber() << "\t"
            << vxdSensor.first.getSensorNumber() << "\t"
            << vxdSensor.second.m_dose << "\t"
            << vxdSensor.second.m_expo << "\t"
            //<< vxdSensor.second.m_fired << "\t"
            << vxdSensor.second.m_occupancy
            << endl;
  }
  outfile << endl;
}
