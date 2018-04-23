#include <svd/modules/svdBackground/SVDBackgroundModule.h>

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
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDEnergyDepositionEvent.h>
#include <svd/dataobjects/SVDNeutronFluxEvent.h>
#include <svd/dataobjects/SVDOccupancyEvent.h>
#include <cmath>
#include <fstream>
#include <set>
#include <algorithm>
#include <numeric>
#include <boost/format.hpp>

#include "TH1F.h"
#include "TH2F.h"
#include "TVector3.h"
#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;
using namespace Belle2::SVD;

//-----------------------------------------------------------------
// A small helper function to convert between electons and ADU
// ----------------------------------------------------------------
double eToADU(double charge)
{
  double minADC = -96000;
  double maxADC = 288000;
  double unitADC = (maxADC - minADC) / 1024.0;
  return round(std::min(maxADC, std::max(minADC, charge)) / unitADC);
}

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDBackground)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDBackgroundModule::SVDBackgroundModule() :
  Module(), m_outputDirectoryName(""),
  m_doseReportingLevel(c_reportNTuple),
  m_nfluxReportingLevel(c_reportNTuple),
  m_occupancyReportingLevel(c_reportNTuple),
  m_componentName(""), m_componentTime(0),
  m_triggerWidth(5), m_acceptanceWidth(2.5), // keeps 99%
  m_nielNeutrons(new TNiel(c_niel_neutronFile)),
  m_nielProtons(new TNiel(c_niel_protonFile)),
  m_nielPions(new TNiel(c_niel_pionFile)),
  m_nielElectrons(new TNiel(c_niel_electronFile))
{
  //Set module properties
  setDescription("SVD background module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  // FIXME: This information can in principle be extracted from bg files, though not trivially.
  addParam("componentName", m_componentName, "Background component name to process", m_componentName);
  addParam("componentTime", m_componentTime, "Background component time", m_componentTime);
  addParam("triggerWidth", m_triggerWidth, "RMS of trigger time estimate in ns", m_triggerWidth);
  addParam("acceptanceWidth", m_acceptanceWidth,
           "A hit is accepted if arrived within +/- accpetanceWidth * RMS(hit time - trigger time) of trigger; in ns", m_acceptanceWidth);
  addParam("doseReportingLevel", m_doseReportingLevel, "0 - no data, 1 - summary only, 2 - summary + ntuple", m_doseReportingLevel);
  addParam("nfluxReportingLevel", m_nfluxReportingLevel, "0 - no data, 1 - summary only, 2 - summary + ntuple",
           m_nfluxReportingLevel);
  addParam("occupancyReportingLevel", m_occupancyReportingLevel, "0 - no data, 1 - summary only, 2 - summary + ntuple",
           m_occupancyReportingLevel);
  addParam("outputDirectory", m_outputDirectoryName, "Name of output directory", m_outputDirectoryName);
}

const TVector3& SVDBackgroundModule::pointToGlobal(VxdID sensorID, const TVector3& local)
{
  static TVector3 result(0, 0, 0);

  const SVD::SensorInfo& info = getInfo(sensorID);
  result = info.pointToGlobal(local);
  return result;
}

const TVector3& SVDBackgroundModule::vectorToGlobal(VxdID sensorID, const TVector3& local)
{
  static TVector3 result(0, 0, 0);

  const SVD::SensorInfo& info = getInfo(sensorID);
  result = info.vectorToGlobal(local);
  return result;
}

SVDBackgroundModule::~SVDBackgroundModule()
{
}

void SVDBackgroundModule::initialize()
{
  //Register collections
  StoreObjPtr<FileMetaData> storeFileMetaData(m_storeFileMetaDataName, DataStore::c_Persistent);
  StoreObjPtr<BackgroundMetaData> storeBgMetaData(m_storeBgMetaDataName, DataStore::c_Persistent);
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<SVDSimHit> storeSimHits(m_storeSimHitsName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  StoreArray<SVDCluster> storeClusters(m_storeClustersName);

  RelationArray relDigitsMCParticles(storeDigits, storeMCParticles);
  RelationArray relDigitsTrueHits(storeDigits, storeTrueHits);
  RelationArray relMCParticlesTrueHits(storeMCParticles, storeTrueHits);
  RelationArray relTrueHitsSimHits(storeTrueHits, storeSimHits);

  // Add two new StoreArrays
  StoreArray<SVDEnergyDepositionEvent> storeEnergyDeposits(m_storeEnergyDepositsName);
  storeEnergyDeposits.registerInDataStore();
  StoreArray<SVDNeutronFluxEvent> storeNeutronFluxes(m_storeNeutronFluxesName);
  storeNeutronFluxes.registerInDataStore();
  StoreArray<SVDOccupancyEvent> storeOccupancyEvents(m_storeOccupancyEventsName);
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
  m_acceptanceWidth *= Unit::ns;
  m_triggerWidth *= Unit::ns;
}

void SVDBackgroundModule::beginRun()
{
}

void SVDBackgroundModule::event()
{
  //Register collections
  const StoreObjPtr<FileMetaData> storeFileMetaData(m_storeFileMetaDataName, DataStore::c_Persistent);
  const StoreObjPtr<BackgroundMetaData> storeBgMetaData(m_storeBgMetaDataName, DataStore::c_Persistent);
  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<SVDSimHit> storeSimHits(m_storeSimHitsName);
  const StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  const StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  const StoreArray<SVDCluster> storeClsuters(m_storeClustersName);

  // Add two new StoreArrays
  StoreArray<SVDEnergyDepositionEvent> storeEnergyDeposits(m_storeEnergyDepositsName);
  StoreArray<SVDNeutronFluxEvent> storeNeutronFluxes(m_storeNeutronFluxesName);
  StoreArray<SVDOccupancyEvent> storeOccupancyEvents(m_storeOccupancyEventsName);

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
    for (const SVDSimHit& hit : storeSimHits) {
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
    for (const SVDTrueHit& hit : storeTrueHits) {
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
      const SVDSimHit* simhit = hit.getRelatedTo<SVDSimHit>();
      if (!simhit) { //either something is very wrong, or we just don't have the relation. Try to find an appropriate SimHit manually.
        double minDistance = 1.0e10;
        for (const SVDSimHit& related : storeSimHits) {
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

      // Store data in a SVDNeutronFluxEvent object
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

  // Fired strips
  if (m_occupancyReportingLevel > c_reportNone) {
    B2DEBUG(100, "Fired strips");
    currentSensorID.setID(0);
    double currentSensorUCut = 0;
    double currentSensorVCut = 0;
    // Store fired strips: count number of digits over threshold
    std::map<VxdID, std::multiset<unsigned short> > firedStrips;
    for (const SVDDigit& digit : storeDigits) {
      // Filter out digits with signals below zero-suppression threshold
      // ARE THRE SUCH DIGITS?
      VxdID sensorID = digit.getSensorID();
      if (sensorID != currentSensorID) {
        currentSensorID = sensorID;
        auto info = getInfo(sensorID);
        currentSensorUCut = eToADU(3.0 * info.getElectronicNoiseU());
        currentSensorVCut = eToADU(3.0 * info.getElectronicNoiseV());
      }
      B2DEBUG(30, "Digit charge: " << digit.getCharge() << " threshold: " << (digit.isUStrip() ? currentSensorUCut : currentSensorVCut));
      if (digit.getCharge() < (digit.isUStrip() ? currentSensorUCut : currentSensorVCut)) continue;
      B2DEBUG(30, "Passed.");
      // Economize writing u- and v- strips by re-using the Segment field of VxdID
      VxdID writeID(sensorID);
      if (digit.isUStrip())
        writeID.setSegmentNumber(0);
      else
        writeID.setSegmentNumber(1);
      firedStrips[writeID].insert(digit.getCellID());
    }
    // Process the map
    for (auto idAndSet : firedStrips) {
      bool isUStrip = (idAndSet.first.getSegmentNumber() == 0);
      VxdID sensorID = idAndSet.first;
      sensorID.setSegmentNumber(0);
      double sensorArea = getSensorArea(sensorID);
      int nFired_APV = idAndSet.second.size();
      int nFired = 0; // count unique keys
      for (auto it = idAndSet.second.begin();
           it != idAndSet.second.end();
           it = idAndSet.second.upper_bound(*it)) nFired++;
      double fired = nFired / (currentComponentTime / Unit::s) / sensorArea;
      double fired_t = nFired_APV * c_APVCycleTime / (currentComponentTime / Unit::s) / sensorArea;
      if (isUStrip) {
        m_sensorData[sensorID].m_firedU += fired;
        m_sensorData[sensorID].m_firedU_t += fired_t;
      } else {
        m_sensorData[sensorID].m_firedV += fired;
        m_sensorData[sensorID].m_firedV_t += fired_t;
      }
    }

    // Occupancy
    //
    // We assume a S/N dependent acceptance window of size
    //   W = 2 * acceptanceWidth * RMS(hit_time - trigger_time)
    // that is used to keep most of signal hits.
    // occupancy for a cluster with S/N = sn and size sz on sensor id =
    //  cluster_rate(sn,sz,id) * W * sz / #strips(id)
    // Cluster rate is number of clusters / sample time, and as we expect
    // clusters to be justly represented in the sample as to S/N, size, and
    // sensor they appear on, we calculate occupancy on sensor id as
    //
    // occupancy(id) = Sum_over_clusters_in_id (
    //  W(sn) / t_simulation * sz / #strips(id)
    // )
    //
    B2DEBUG(100, "Occupancy");
    currentSensorID.setID(0);
    double currentNoiseU = 0;
    double currentNoiseV = 0;
    int nStripsU = 0;
    int nStripsV = 0;
    for (auto cluster : storeClsuters) {
      VxdID sensorID = cluster.getSensorID();
      if (currentSensorID != sensorID) {
        currentSensorID = sensorID;
        auto info = getInfo(sensorID);
        currentNoiseU = eToADU(info.getElectronicNoiseU());
        currentNoiseV = eToADU(info.getElectronicNoiseV());
        nStripsU = info.getUCells();
        nStripsV = info.getVCells();
      }
      bool isU = cluster.isUCluster();
      double snr = (isU) ? cluster.getCharge() / currentNoiseU : cluster.getCharge() / currentNoiseV;
      int nStrips = (isU) ? nStripsU : nStripsV;
      double tau_error = 45 / snr * Unit::ns;
      tau_error = sqrt(m_triggerWidth * m_triggerWidth + tau_error * tau_error);
      double tau_acceptance = 2 * m_acceptanceWidth * tau_error;
      double w_acceptance =  tau_acceptance / currentComponentTime;
      double w_acceptance_APV = c_APVCycleTime / currentComponentTime;
      double occupancy = 1.0 / nStrips * cluster.getSize();
      if (isU) {
        m_sensorData[sensorID].m_occupancyU += w_acceptance * occupancy;
        m_sensorData[sensorID].m_occupancyU_APV += w_acceptance_APV * occupancy;
      } else {
        m_sensorData[sensorID].m_occupancyV += w_acceptance * occupancy;
        m_sensorData[sensorID].m_occupancyV_APV += w_acceptance_APV * occupancy;
      }
      if (m_occupancyReportingLevel == c_reportNTuple) {
        storeOccupancyEvents.appendNew(
          sensorID.getLayerNumber(), sensorID.getLadderNumber(),
          sensorID.getSensorNumber(), cluster.getClsTime(),
          cluster.isUCluster(), cluster.getPosition(), cluster.getSize(),
          cluster.getCharge(), snr, w_acceptance, w_acceptance * occupancy,
          w_acceptance_APV * occupancy
        );
      }
    }
  }
}

void SVDBackgroundModule::endRun()
{
}


void SVDBackgroundModule::terminate()
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
          << "fired_u\t"
          << "fired_v\t"
          << "fired_u_t\t"
          << "fired_v_t\t"
          << "occupancy_u\t"
          << "occupancy_v\t"
          << "occupancy_u_APV\t"
          << "occupancy_v_APV"
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
            << vxdSensor.second.m_firedU << "\t"
            << vxdSensor.second.m_firedV << "\t"
            << vxdSensor.second.m_firedU_t << "\t"
            << vxdSensor.second.m_firedV_t << "\t"
            << vxdSensor.second.m_occupancyU << "\t"
            << vxdSensor.second.m_occupancyV << "\t"
            << vxdSensor.second.m_occupancyU_APV << "\t"
            << vxdSensor.second.m_occupancyV_APV
            << endl;
  }
  outfile << endl;
}
