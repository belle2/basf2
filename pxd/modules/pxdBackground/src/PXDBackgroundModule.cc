#include <pxd/modules/pxdBackground/PXDBackgroundModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <simulation/background/BkgNeutronWeight.h>

#include <framework/core/InputController.h>

#include <framework/dataobjects/FileMetaData.h>
#include <background/dataobjects/BackgroundMetaData.h>
#include <mdst/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDEnergyDepositionEvent.h>
#include <pxd/dataobjects/PXDNeutronFluxEvent.h>
#include <pxd/dataobjects/PXDOccupancyEvent.h>
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
REG_MODULE(PXDBackground)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDBackgroundModule::PXDBackgroundModule() :
  Module(), m_outputDirectoryName(""),
  m_doseReportingLevel(c_reportNTuple),
  m_nfluxReportingLevel(c_reportNTuple),
  m_occupancyReportingLevel(c_reportNTuple),
  m_componentName(""), m_componentTime(0), m_integrationTime(20),
  m_nielNeutrons(new TNiel(c_niel_neutronFile)),
  m_nielProtons(new TNiel(c_niel_protonFile)),
  m_nielPions(new TNiel(c_niel_pionFile)),
  m_nielElectrons(new TNiel(c_niel_electronFile))
{
  //Set module properties
  setDescription("PXD background module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  // FIXME: This information can in principle be extracted from bg files, though not trivially.
  addParam("componentName", m_componentName, "Background component name to process", m_componentName);
  addParam("componentTime", m_componentTime, "Background component time", m_componentTime);
  addParam("integrationTime", m_integrationTime, "PXD integration time", m_integrationTime);
  addParam("doseReportingLevel", m_doseReportingLevel, "0 - no data, 1 - summary only, 2 - summary + ntuple", m_doseReportingLevel);
  addParam("nfluxReportingLevel", m_nfluxReportingLevel, "0 - no data, 1 - summary only, 2 - summary + ntuple",
           m_nfluxReportingLevel);
  addParam("occupancyReportingLevel", m_occupancyReportingLevel, "0 - no data, 1 - summary only, 2 - summary + ntuple",
           m_occupancyReportingLevel);
  addParam("outputDirectory", m_outputDirectoryName, "Name of output directory", m_outputDirectoryName);
}

const TVector3& PXDBackgroundModule::pointToGlobal(VxdID sensorID, const TVector3& local)
{
  static TVector3 result(0, 0, 0);

  const PXD::SensorInfo& info = getInfo(sensorID);
  result = info.pointToGlobal(local, true);
  return result;
}

const TVector3& PXDBackgroundModule::vectorToGlobal(VxdID sensorID, const TVector3& local)
{
  static TVector3 result(0, 0, 0);

  const PXD::SensorInfo& info = getInfo(sensorID);
  result = info.vectorToGlobal(local, true);
  return result;
}

PXDBackgroundModule::~PXDBackgroundModule()
{
}

void PXDBackgroundModule::initialize()
{
  //Register collections
  StoreObjPtr<FileMetaData> storeFileMetaData(m_storeFileMetaDataName, DataStore::c_Persistent);
  StoreObjPtr<BackgroundMetaData> storeBgMetaData(m_storeBgMetaDataName, DataStore::c_Persistent);
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<PXDSimHit> storeSimHits(m_storeSimHitsName);
  StoreArray<PXDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);

  RelationArray relDigitsMCParticles(storeDigits, storeMCParticles);
  RelationArray relDigitsTrueHits(storeDigits, storeTrueHits);
  RelationArray relMCParticlesTrueHits(storeMCParticles, storeTrueHits);
  RelationArray relTrueHitsSimHits(storeTrueHits, storeSimHits);

  // Add two new StoreArrays
  StoreArray<PXDEnergyDepositionEvent> storeEnergyDeposits(m_storeEnergyDepositsName);
  storeEnergyDeposits.registerInDataStore();
  StoreArray<PXDNeutronFluxEvent> storeNeutronFluxes(m_storeNeutronFluxesName);
  storeNeutronFluxes.registerInDataStore();
  StoreArray<PXDOccupancyEvent> storeOccupancyEvents(m_storeOccupancyEventsName);
  storeOccupancyEvents.registerInDataStore();

  //Store names to speed up creation later
  m_storeFileMetaDataName = storeFileMetaData.getName();
  m_storeBgMetaDataName = storeBgMetaData.getName();
  m_storeMCParticlesName = storeMCParticles.getName();
  m_storeSimHitsName = storeSimHits.getName();
  m_storeTrueHitsName = storeTrueHits.getName();
  m_storeDigitsName = storeDigits.getName();
  m_relDigitsMCParticlesName = relDigitsMCParticles.getName();
  m_relDigitsTrueHitsName = relDigitsTrueHits.getName();
  m_relParticlesTrueHitsName = relMCParticlesTrueHits.getName();
  m_relTrueHitsSimHitsName = relTrueHitsSimHits.getName();
  m_storeEnergyDepositsName = storeEnergyDeposits.getName();
  m_storeNeutronFluxesName = storeNeutronFluxes.getName();

  m_componentTime *= Unit::us;
  m_integrationTime *= Unit::us;
}

void PXDBackgroundModule::beginRun()
{
}

void PXDBackgroundModule::event()
{
  //Register collections
  const StoreObjPtr<FileMetaData> storeFileMetaData(m_storeFileMetaDataName, DataStore::c_Persistent);
  const StoreObjPtr<BackgroundMetaData> storeBgMetaData(m_storeBgMetaDataName, DataStore::c_Persistent);
  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<PXDSimHit> storeSimHits(m_storeSimHitsName);
  const StoreArray<PXDTrueHit> storeTrueHits(m_storeTrueHitsName);
  const StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  const StoreArray<PXDCluster> storeClsuters(m_storeClustersName);

  // Add two new StoreArrays
  StoreArray<PXDEnergyDepositionEvent> storeEnergyDeposits(m_storeEnergyDepositsName);
  StoreArray<PXDNeutronFluxEvent> storeNeutronFluxes(m_storeNeutronFluxesName);
  StoreArray<PXDOccupancyEvent> storeOccupancyEvents(m_storeOccupancyEventsName);

  // Relations
  RelationArray relDigitsMCParticles(storeDigits, storeMCParticles, m_relDigitsMCParticlesName);
  RelationArray relDigitsTrueHits(storeDigits, storeTrueHits, m_relDigitsTrueHitsName);
  RelationArray relTrueHitsSimHits(storeTrueHits, storeSimHits, m_relTrueHitsSimHitsName);
  RelationArray relTrueHitsMCParticles(storeMCParticles, storeTrueHits, m_relParticlesTrueHitsName);

  // unsigned long numberOfEvents = storeFileMetaData->getNEvents();
  double currentComponentTime = storeBgMetaData->getRealTime();
  if (currentComponentTime != m_componentTime)
    B2FATAL("Mismatch in component times:\n"
            << "Steering file:   " << m_componentTime << "\n"
            << "Background file: " << currentComponentTime);

  VxdID currentSensorID(0);
  double currentSensorThickness(0);
  double currentSensorMass(0);
  double currentSensorArea(0);

  // Exposition and dose
  if (m_doseReportingLevel > c_reportNone) {
    B2DEBUG(100, "Expo and dose");
    currentSensorID.setID(0);
    for (const PXDSimHit& hit : storeSimHits) {
      // Update if we have a new sensor
      VxdID sensorID = hit.getSensorID();
      if (sensorID != currentSensorID) {
        currentSensorID = sensorID;
        currentSensorThickness = getSensorThickness(currentSensorID);
        currentSensorMass = getSensorMass(currentSensorID);
        currentSensorArea = getSensorArea(currentSensorID);
      }
      double hitEnergy = hit.getElectrons() * Const::ehEnergy;
      // Dose in Gy/smy, normalize by sensor mass
      m_sensorData[currentSensorID].m_dose +=
        (hitEnergy / Unit::J) / (currentSensorMass / 1000) * (c_smy / currentComponentTime);
      // Exposition in GeV/cm2/s
      m_sensorData[currentSensorID].m_expo += hitEnergy / currentSensorArea / (currentComponentTime / Unit::s);
      if (m_doseReportingLevel == c_reportNTuple) {
        const TVector3 localPos = hit.getPosIn();
        const TVector3 globalPos = pointToGlobal(currentSensorID, localPos);
        float globalPosXYZ[3];
        globalPos.GetXYZ(globalPosXYZ);
        storeEnergyDeposits.appendNew(
          sensorID.getLayerNumber(), sensorID.getLadderNumber(), sensorID.getSensorNumber(),
          hit.getPDGcode(), hit.getGlobalTime(),
          localPos.X(), localPos.Y(), globalPosXYZ, hitEnergy,
          (hitEnergy / Unit::J) / (currentSensorMass / 1000) / (currentComponentTime / Unit::s),
          (hitEnergy / Unit::J) / currentSensorArea / (currentComponentTime / Unit::s)
        );
      }
    }
  }

  // Neutron flux
  if (m_nfluxReportingLevel > c_reportNone) {
    B2DEBUG(100, "Neutron flux");
    currentSensorID.setID(0);
    for (const PXDTrueHit& hit : storeTrueHits) {
      VxdID sensorID = hit.getSensorID();
      // Update if we are on a new sensor
      if (sensorID != currentSensorID) {
        currentSensorID = sensorID;
        currentSensorThickness = getSensorThickness(currentSensorID);
        currentSensorMass = getSensorMass(currentSensorID);
        currentSensorArea = getSensorArea(currentSensorID);
      }
      // J(TrueHit) = abs(step)/thickness * correctionFactor;
      TVector3 entryPos(hit.getEntryU(), hit.getEntryV(), hit.getEntryW());
      TVector3 exitPos(hit.getExitU(), hit.getExitV(), hit.getExitW());
      double stepLength = (exitPos - entryPos).Mag();
      // Identify what particle we've got. We need type and kinetic energy.
      // TODO: TrueHit must carry pdg or SimHit must carry energy.
      // NOTE: MCParticles may get remapped, then SimHits still carry correct pdg.
      const PXDSimHit* simhit = hit.getRelatedTo<PXDSimHit>();
      if (!simhit) { //either something is very wrong, or we just don't have the relation. Try to find an appropriate SimHit manually.
        double minDistance = 1.0e10;
        for (const PXDSimHit& related : storeSimHits) {
          double distance = (entryPos - related.getPosIn()).Mag();
          if (distance < minDistance) {
            minDistance = distance;
            simhit = &related;
          }
        }
      }
      // FIXME: Is there a difference between positrons and electrons wrt. NIEL?
      // We fill neutronFluxBars with summary NIEL deposit for all kinds of particles by layer and component.
      // Fluency plots are by component and are deposition histograms for a particular type of particle and compoonent.
      // Special treatment of corrupt p's in TrueHits:
      TVector3 hitMomentum(hit.getMomentum());
      hitMomentum.SetX(std::isfinite(hitMomentum.X()) ? hitMomentum.X() : 0.0);
      hitMomentum.SetY(std::isfinite(hitMomentum.Y()) ? hitMomentum.Y() : 0.0);
      hitMomentum.SetZ(std::isfinite(hitMomentum.Z()) ? hitMomentum.Z() : 0.0);
      int pdg = abs(simhit->getPDGcode());
      double kineticEnergy(0.0);
      double nielWeight(0.0);
      if (pdg == 2112) {
        double m0 = 0.940;
        kineticEnergy = sqrt(hitMomentum.Mag2() + m0 * m0) - m0;
        nielWeight = m_nielNeutrons->getNielFactor(kineticEnergy / Unit::MeV);
      }
      if (pdg == 2212) {
        double m0 = 0.938;
        kineticEnergy = sqrt(hitMomentum.Mag2() + m0 * m0) - m0;
        nielWeight = m_nielProtons->getNielFactor(kineticEnergy / Unit::MeV);
      }
      if (pdg == 111 || pdg == 211) {
        double m0 = 0.135;
        kineticEnergy = sqrt(hitMomentum.Mag2() + m0 * m0) - m0;
        nielWeight = m_nielPions->getNielFactor(kineticEnergy / Unit::MeV);
      }
      if (pdg == 11) {
        double m0 = 0.511e-3;
        kineticEnergy = sqrt(hitMomentum.Mag2() + m0 * m0) - m0;
        nielWeight = m_nielElectrons->getNielFactor(kineticEnergy / Unit::MeV);
      }
      if (pdg == 22) {
        double m0 = 0.0;
        kineticEnergy = sqrt(hitMomentum.Mag2() + m0 * m0) - m0;
      }

      // Only set weight for supported particles
      nielWeight = std::isfinite(nielWeight) ? nielWeight : 0.0;
      m_sensorData[currentSensorID].m_neutronFlux += nielWeight * stepLength / currentSensorThickness / currentSensorArea /
                                                     currentComponentTime * c_smy;

      // Store data in a PXDNeutronFluxEvent object
      if (m_nfluxReportingLevel == c_reportNTuple) {
        TVector3 localPos(hit.getU(), hit.getV(), hit.getW());
        const TVector3 globalPos = pointToGlobal(currentSensorID, localPos);
        float globalPosXYZ[3];
        globalPos.GetXYZ(globalPosXYZ);
        TVector3 localMom = hit.getMomentum();
        const TVector3 globalMom = vectorToGlobal(currentSensorID, localMom);
        float globalMomXYZ[3];
        globalMom.GetXYZ(globalMomXYZ);
        storeNeutronFluxes.appendNew(
          sensorID.getLayerNumber(), sensorID.getLadderNumber(), sensorID.getSensorNumber(),
          simhit->getPDGcode(), simhit->getGlobalTime(),
          hit.getU(), hit.getV(), globalPosXYZ, globalMomXYZ, kineticEnergy,
          stepLength, nielWeight,
          stepLength / currentSensorThickness / currentSensorArea / (currentComponentTime / Unit::s),
          nielWeight * stepLength / currentSensorThickness / currentSensorArea / (currentComponentTime / Unit::s)
        );
      }
    }
  }

  // Occupancy
  if (m_occupancyReportingLevel > c_reportNone) {
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
      m_sensorData[sensorID].m_fired += fired;
    }

    B2DEBUG(100, "Occupancy");
    currentSensorID.setID(0);
    int nPixels = 0;
    for (auto cluster : storeClsuters) {
      VxdID sensorID = cluster.getSensorID();
      if (currentSensorID != sensorID) {
        currentSensorID = sensorID;
        auto info = getInfo(sensorID);
        nPixels = info.getUCells() * info.getVCells();
      }

      double w_acceptance =  m_integrationTime / currentComponentTime;
      double occupancy = 1.0 / nPixels * cluster.getSize();
      m_sensorData[sensorID].m_occupancy +=  w_acceptance * occupancy;

      if (m_occupancyReportingLevel == c_reportNTuple) {
        storeOccupancyEvents.appendNew(
          sensorID.getLayerNumber(), sensorID.getLadderNumber(),
          sensorID.getSensorNumber(),
          cluster.getU(), cluster.getV(), cluster.getSize(),
          cluster.getCharge(), occupancy
        );
      }
    }
  }
}

void PXDBackgroundModule::endRun()
{
}


void PXDBackgroundModule::terminate()
{
  // Write out m_data
  ofstream outfile;
  string outfileName(m_outputDirectoryName + m_componentName + "_summary.txt");
  outfile.open(outfileName.c_str(), ios::out | ios::trunc);
  outfile << "component_name\t"
          << "component_time\t"
          << "layer\t"
          << "ladder\t"
          << "sensor\t"
          << "dose\t"
          << "expo\t"
          << "neutronFlux\t"
          << "fired\t"
          << "occupancy"
          << endl;
  double componentTime = m_componentTime / Unit::us;
  for (auto vxdSensor : m_sensorData) {
    outfile << m_componentName.c_str() << "\t"
            << componentTime << "\t"
            << vxdSensor.first.getLayerNumber() << "\t"
            << vxdSensor.first.getLadderNumber() << "\t"
            << vxdSensor.first.getSensorNumber() << "\t"
            << vxdSensor.second.m_dose << "\t"
            << vxdSensor.second.m_expo << "\t"
            << vxdSensor.second.m_neutronFlux << "\t"
            << vxdSensor.second.m_fired << "\t"
            << vxdSensor.second.m_occupancy
            << endl;
  }
  outfile << endl;
}
