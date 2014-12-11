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
#include <svd/background/HistogramFactory.h>

#include <fstream>
#include <set>
#include <algorithm>
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
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDBackground)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDBackgroundModule::SVDBackgroundModule() :
  HistoModule(), m_outputDirectoryName(""), m_treeSummary(0),
  m_nielNeutrons(new TNiel(c_niel_neutronFile)),
  m_nielProtons(new TNiel(c_niel_protonFile)),
  m_nielPions(new TNiel(c_niel_pionFile)),
  m_nielElectrons(new TNiel(c_niel_electronFile)),
  m_histogramFactory(new HistogramFactory)
{
  //Set module properties
  setDescription("SVD background module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  // FIXME: This information can in principle be extracted from bg files, though not trivially.
  addParam("componentNames", m_componentNames, "List of background component names to process", m_componentNames);
  addParam("componentTimes", m_componentTimes, "List of background component times", m_componentTimes);
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

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void SVDBackgroundModule::defineHisto()
{
  // Declare the TTree that will hold background data in tabular/database form
  m_treeSummary = new TTree("bSummary", "Summary data for a background study");
  // Declare the componentwise plots
  for (auto compNameAndData : m_data) {
    const string& componentName(compNameAndData.first);
    // Create an underscored version of the component name
    string componentID(componentName);
    replace(componentID.begin(), componentID.end(), ' ', '_');
    BackgroundData& bData = m_data[componentName];
    // Bar plots
    bData.m_doseBars = m_histogramFactory->MakeBarPlot(componentName, "dose");
    bData.m_neutronFluxBars = m_histogramFactory->MakeBarPlot(componentName, "neutronFlux");
    bData.m_uFiredBars = m_histogramFactory->MakeBarPlot(componentName, "firedU");
    bData.m_vFiredBars = m_histogramFactory->MakeBarPlot(componentName, "firedV");
    bData.m_uOccupancyBars = m_histogramFactory->MakeBarPlot(componentName, "occupancyU");
    bData.m_vOccupancyBars = m_histogramFactory->MakeBarPlot(componentName, "occupancyV");
    // Fluencies - by layer
    for (int iLayer = 3; iLayer <= 6; ++iLayer) {
      bData.m_neutronFlux[iLayer] = m_histogramFactory->MakeFluencePlot(componentName, "neutronFlux", iLayer);
      bData.m_neutronFluxNIEL[iLayer] = m_histogramFactory->MakeFluencePlot(componentName, "neutronFluxNIEL", iLayer);
      bData.m_protonFlux[iLayer] = m_histogramFactory->MakeFluencePlot(componentName, "protonFlux", iLayer);
      bData.m_protonFluxNIEL[iLayer] = m_histogramFactory->MakeFluencePlot(componentName, "protonFluxNIEL", iLayer);
      bData.m_pionFlux[iLayer] = m_histogramFactory->MakeFluencePlot(componentName, "pionFlux", iLayer);
      bData.m_pionFluxNIEL[iLayer] = m_histogramFactory->MakeFluencePlot(componentName, "pionFluxNIEL", iLayer);
      bData.m_electronFlux[iLayer] = m_histogramFactory->MakeFluencePlot(componentName, "electronFlux", iLayer);
      bData.m_electronFluxNIEL[iLayer] = m_histogramFactory->MakeFluencePlot(componentName, "electronFluxNIEL", iLayer);
      bData.m_photonFlux[iLayer] = m_histogramFactory->MakeFluencePlot(componentName, "photonFlux", iLayer);
    }
  }
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

  RelationArray relDigitsMCParticles(storeDigits, storeMCParticles);
  RelationArray relDigitsTrueHits(storeDigits, storeTrueHits);
  RelationArray relMCParticlesTrueHits(storeMCParticles, storeTrueHits);
  RelationArray relTrueHitsSimHits(storeTrueHits, storeSimHits);

  // Add two new StoreArrays
  StoreArray<SVDEnergyDepositionEvent> storeEnergyDeposits(m_storeEnergyDepositsName);
  storeEnergyDeposits.registerInDataStore();
  StoreArray<SVDNeutronFluxEvent> storeNeutronFluxes(m_storeNeutronFluxesName);
  storeNeutronFluxes.registerInDataStore();

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

  // Initialize m_data:
  for (unsigned int iComp = 0; iComp < m_componentNames.size(); ++iComp) {
    string componentName = m_componentNames[iComp];
    m_data[componentName].m_componentName = componentName;
    m_data[componentName].m_componentTime = m_componentTimes[iComp] * Unit::us;
  }
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

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

  // Add two new StoreArrays
  StoreArray<SVDEnergyDepositionEvent> storeEnergyDeposits(m_storeEnergyDepositsName);
  StoreArray<SVDNeutronFluxEvent> storeNeutronFluxes(m_storeNeutronFluxesName);

  // Relations
  RelationArray relDigitsMCParticles(storeDigits, storeMCParticles, m_relDigitsMCParticlesName);
  RelationArray relDigitsTrueHits(storeDigits, storeTrueHits, m_relDigitsTrueHitsName);
  RelationArray relTrueHitsSimHits(storeTrueHits, storeSimHits, m_relTrueHitsSimHitsName);
  RelationArray relTrueHitsMCParticles(storeMCParticles, storeTrueHits, m_relParticlesTrueHitsName);

  const string& componentName = storeBgMetaData->getBackgroundType();
  if (componentName != m_currentComponentName) {
    B2INFO("Current component: " << componentName);
    m_currentComponentName = componentName;
  }

  // Get number of events to process
  unsigned long numberOfEvents = storeFileMetaData->getEvents();
  double currentComponentTime = storeBgMetaData->getRealTime() * Unit::us;

  // Exposition and dose
  B2DEBUG(100, "Expo and dose");
  BackgroundData& bData = m_data[m_currentComponentName];
  VxdID currentSensorID(0);
  double currentSensorThickness(0);
  double currentSensorMass(0);
  double currentSensorArea(0);
  int currentLayerNumber(0);
  double currentLayerMass(0);
  double currentLayerArea(0);

  // Exposition and dose
  currentSensorID.setID(0);
  for (const SVDSimHit & hit : storeSimHits) {
    // Update if we have a new sensor
    VxdID sensorID = hit.getSensorID();
    if (sensorID != currentSensorID) {
      currentSensorID = sensorID;
      currentSensorThickness = getSensorThickness(currentSensorID);
      currentSensorMass = getSensorMass(currentSensorID);
      currentSensorArea = getSensorArea(currentSensorID);
      currentLayerNumber = sensorID.getLayerNumber();
      currentLayerMass = getLayerMass(currentLayerNumber);
      currentLayerArea = getLayerArea(currentLayerNumber);
    }
    double hitEnergy = hit.getElectrons() * Const::ehEnergy;
    // Dose in Gy/smy, normalize by sensor mass
    bData.m_sensorData[currentSensorID].m_dose +=
      (hitEnergy / Unit::J) / (currentSensorMass / 1000) * (c_smy / currentComponentTime);
    // Exposition in GeV/cm2/s
    bData.m_sensorData[currentSensorID].m_expo += hitEnergy / currentSensorArea / (currentComponentTime / Unit::s);
    // Bar charts
    // Normalize by layer mass for bar charts
    bData.m_doseBars->Fill(currentLayerNumber,
                           (hitEnergy / Unit::J) / (currentLayerMass / 1000) * (c_smy / currentComponentTime));
    // Store things in an SVDEnergyDepositEvent object
    TVector3 localPos = 0.5 * (hit.getPosIn() + hit.getPosOut());
    const TVector3 globalPos = pointToGlobal(currentSensorID, localPos);
    float globalPosXYZ[3];
    globalPos.GetXYZ(globalPosXYZ);
    storeEnergyDeposits.appendNew(
      sensorID.getLayerNumber(), sensorID.getLadderNumber(), sensorID.getSensorNumber(),
      hit.getBackgroundTag(), hit.getPDGcode(), hit.getGlobalTime(),
      localPos.X(), localPos.Y(), globalPosXYZ, hitEnergy,
      (hitEnergy / Unit::J) / (currentSensorMass / 1000) / (currentComponentTime / Unit::s),
      (hitEnergy / Unit::J) / currentSensorArea / (currentComponentTime / Unit::s)
    );
  }

  // Neutron flux
  B2DEBUG(100, "Neutron flux")
  currentSensorID.setID(0);
  for (const SVDTrueHit & hit : storeTrueHits) {
    VxdID sensorID = hit.getSensorID();
    // Update if we are on a new sensor
    if (sensorID != currentSensorID) {
      currentSensorID = sensorID;
      currentSensorThickness = getSensorThickness(currentSensorID);
      currentSensorMass = getSensorMass(currentSensorID);
      currentSensorArea = getSensorArea(currentSensorID);
      currentLayerNumber = sensorID.getLayerNumber();
      currentLayerMass = getLayerMass(currentLayerNumber);
      currentLayerArea = getLayerArea(currentLayerNumber);
    }
    // J(TrueHit) = abs(step)/thickness * correctionFactor;
    TVector3 entryPos(hit.getEntryU(), hit.getEntryV(), hit.getEntryW());
    TVector3 exitPos(hit.getExitU(), hit.getExitV(), hit.getExitW());
    double stepLength = (exitPos - entryPos).Mag();
    // Identify what particle we've got. We need type and kinetic energy.
    // TODO: TrueHit must carry pdg!
    // NOTE: MCParticles may get remapped, then SimHits still carry correct pdg.
    const SVDSimHit* simhit = hit.getRelatedTo<SVDSimHit>();
    if (!simhit) { // never!
      B2ERROR("NO SIMHIT FOUND FOR A TRUEHIT. CAN'T HAPPEN!!!");
      continue;
    }
    // FIXME: Is there a difference between positrons and electrons wrt. NIEL?
    // We fill neutronFluxBars with summary NIEL deposit for all kinds of particles by layer and component.
    // Fluency plots are by component and are deposition histograms for a particular type of particle and compoonent.
    int pdg = abs(simhit->getPDGcode());
    double kineticEnergy(0.0);
    double nielWeight(0.0);
    if (pdg == 2112) {
      double m0 = 0.940;
      kineticEnergy = sqrt(hit.getMomentum().Mag2() + m0 * m0) - m0;
      nielWeight = m_nielNeutrons->getNielFactor(kineticEnergy / Unit::MeV);
      bData.m_neutronFluxBars->Fill(currentLayerNumber, nielWeight * stepLength / currentSensorThickness / currentLayerArea / currentComponentTime * c_smy);
      bData.m_neutronFlux[currentLayerNumber]->Fill(kineticEnergy / Unit::MeV,
                                                    stepLength / currentSensorThickness / currentLayerArea / currentComponentTime * c_smy);
      bData.m_neutronFluxNIEL[currentLayerNumber]->Fill(kineticEnergy / Unit::MeV,
                                                        nielWeight * stepLength / currentSensorThickness / currentLayerArea / currentComponentTime * c_smy);
    }
    if (pdg == 2212) {
      double m0 = 0.938;
      kineticEnergy = sqrt(hit.getMomentum().Mag2() + m0 * m0) - m0;
      nielWeight = m_nielProtons->getNielFactor(kineticEnergy / Unit::MeV);
      bData.m_neutronFluxBars->Fill(currentLayerNumber, nielWeight * stepLength / currentSensorThickness / currentLayerArea / currentComponentTime * c_smy);
      bData.m_protonFlux[currentLayerNumber]->Fill(kineticEnergy / Unit::MeV,
                                                   stepLength / currentSensorThickness / currentLayerArea / currentComponentTime * c_smy);
      bData.m_protonFluxNIEL[currentLayerNumber]->Fill(kineticEnergy / Unit::MeV,
                                                       nielWeight * stepLength / currentSensorThickness / currentLayerArea / currentComponentTime * c_smy);
    }
    if (pdg == 111 || pdg == 211) {
      double m0 = 0.135;
      kineticEnergy = sqrt(hit.getMomentum().Mag2() + m0 * m0) - m0;
      nielWeight = m_nielPions->getNielFactor(kineticEnergy / Unit::MeV);
      bData.m_neutronFluxBars->Fill(currentLayerNumber, nielWeight * stepLength / currentSensorThickness / currentLayerArea / currentComponentTime * c_smy);
      bData.m_pionFlux[currentLayerNumber]->Fill(kineticEnergy / Unit::MeV,
                                                 stepLength / currentSensorThickness / currentLayerArea / currentComponentTime * c_smy);
      bData.m_pionFluxNIEL[currentLayerNumber]->Fill(kineticEnergy / Unit::MeV,
                                                     nielWeight * stepLength / currentSensorThickness / currentLayerArea / currentComponentTime * c_smy);
    }
    if (pdg == 11) {
      double m0 = 0.511e-3;
      kineticEnergy = sqrt(hit.getMomentum().Mag2() + m0 * m0) - m0;
      nielWeight = m_nielElectrons->getNielFactor(kineticEnergy / Unit::MeV);
      bData.m_neutronFluxBars->Fill(currentLayerNumber, nielWeight * stepLength / currentSensorThickness / currentLayerArea / currentComponentTime * c_smy);
      bData.m_electronFlux[currentLayerNumber]->Fill(kineticEnergy / Unit::MeV,
                                                     stepLength / currentSensorThickness / currentLayerArea / currentComponentTime * c_smy);
      bData.m_electronFluxNIEL[currentLayerNumber]->Fill(kineticEnergy / Unit::MeV,
                                                         nielWeight * stepLength / currentSensorThickness / currentLayerArea / currentComponentTime * c_smy);
    }
    if (pdg == 22) {
      double m0 = 0.0;
      kineticEnergy = sqrt(hit.getMomentum().Mag2() + m0 * m0) - m0;
      bData.m_photonFlux[currentLayerNumber]->Fill(kineticEnergy / Unit::MeV,
                                                   stepLength / currentSensorThickness / currentLayerArea / currentComponentTime * c_smy);
    }

    // Only set weight for supported particles

    bData.m_sensorData[currentSensorID].m_neutronFlux += nielWeight * stepLength / currentSensorThickness / currentSensorArea / currentComponentTime * c_smy;

    // Store data in a SVDNeutronFluxEvent object
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
      simhit->getBackgroundTag(), simhit->getPDGcode(), simhit->getGlobalTime(),
      hit.getU(), hit.getV(), globalPosXYZ, globalMomXYZ, kineticEnergy,
      stepLength, nielWeight,
      stepLength / currentSensorThickness / currentSensorArea / (currentComponentTime / Unit::s),
      nielWeight * stepLength / currentSensorThickness / currentSensorArea / (currentComponentTime / Unit::s)
    );
  }

  // Fired strips and raw occupancy
  B2DEBUG(100, "Fired strips")
  currentSensorID.setID(0);
  // Store fired strips: set counts give occupancies, no double-counting
  std::map<VxdID, std::set<int> > firedStrips;
  // Collision time: if trigger arrives within tau_event, the APV readings will be distorted by this digit
  double tau_sensitive = (2 * c_nAPVSamples - 1) * c_APVSampleTime;
  double tau_event = currentComponentTime / numberOfEvents;
  // Number of background events that contribute to sensitive time window of SVD
  double w_event = tau_sensitive / tau_event;
  for (const SVDDigit & digit : storeDigits) {
    // Filter out digits that give less than 1 ADU
    if (digit.getCharge() < 1) continue;
    VxdID sensorID = digit.getSensorID();
    // Economize writing u- and v- strips by re-using the Segment field of VxdID
    VxdID writeID(sensorID);
    if (!digit.isUStrip())
      writeID.setSegmentNumber(1);
    else
      writeID.setSegmentNumber(0);
    firedStrips[writeID].insert(digit.getCellID());
  }
  // Process the map
  for (auto idAndSet : firedStrips) {
    bool isUStrip = (idAndSet.first.getSegmentNumber() == 0);
    VxdID sensorID = idAndSet.first;
    sensorID.setSegmentNumber(0);
    double sensorArea = getSensorArea(sensorID);
    currentLayerNumber = sensorID.getLayerNumber();
    int nFired = idAndSet.second.size();
    double fired = nFired / (currentComponentTime / Unit::s) / sensorArea;
    int nStrips = isUStrip ? getInfo(sensorID).getUCells() : getInfo(sensorID).getVCells();
    double occupancy = w_event * nFired / nStrips / numberOfEvents;
    if (isUStrip) {
      bData.m_sensorData[sensorID].m_firedU += fired;
      bData.m_uFiredBars->Fill(currentLayerNumber, fired);
      bData.m_sensorData[sensorID].m_occupancyU += occupancy;
      bData.m_uOccupancyBars->Fill(currentLayerNumber, occupancy);
    } else {
      bData.m_sensorData[sensorID].m_firedV += fired;
      bData.m_vFiredBars->Fill(currentLayerNumber, fired);
      bData.m_sensorData[sensorID].m_occupancyV += occupancy;
      bData.m_vOccupancyBars->Fill(currentLayerNumber, occupancy);
    }
  }
}

void SVDBackgroundModule::endRun()
{
}


void SVDBackgroundModule::terminate()
{
  // Write things to the TTree
  string componentName;
  m_treeSummary->Branch("component", &componentName);
  int layer;
  m_treeSummary->Branch("layer", &layer, "layer/I");
  int ladder;
  m_treeSummary->Branch("ladder", &ladder, "ladder/I");
  int sensor;
  m_treeSummary->Branch("sensor", &sensor, "sensor/I");
  double dose;
  m_treeSummary->Branch("dose", &dose, "dose/D");
  double expo;
  m_treeSummary->Branch("expo", &expo, "expo/D");
  double neutronFlux;
  m_treeSummary->Branch("neutronFlux", &neutronFlux, "neutronFlux/D");
  double firedU;
  m_treeSummary->Branch("firedU", &firedU, "firedU/D");
  double firedV;
  m_treeSummary->Branch("firedV", &firedV, "firedV/D");
  double occupancyU;
  m_treeSummary->Branch("occupancyU", &occupancyU, "occupancyU/D");
  double occupancyV;
  m_treeSummary->Branch("occupancyV", &occupancyV, "occupancyV/D");
  // Write to tree
  for (auto cmpBkg : m_data) {
    componentName = cmpBkg.first;
    BackgroundData bgData = cmpBkg.second;
    for (auto vxdSensor : bgData.m_sensorData) {
      layer = vxdSensor.first.getLayerNumber();
      ladder = vxdSensor.first.getLadderNumber();
      sensor = vxdSensor.first.getSensorNumber();
      dose = vxdSensor.second.m_dose;
      expo = vxdSensor.second.m_expo;
      neutronFlux = vxdSensor.second.m_neutronFlux;
      firedU = vxdSensor.second.m_firedU;
      firedV = vxdSensor.second.m_firedV;
      occupancyU = vxdSensor.second.m_occupancyU;
      occupancyV = vxdSensor.second.m_occupancyV;
      m_treeSummary->Fill();
    }
  }

  // Write out m_data
  ofstream outfile;
  outfile.open("BackgroundData.txt", ios::out | ios::app);
  outfile << "component_name "
          << "layer "
          << "ladder "
          << "sensor "
          << "dose "
          << "expo "
          << "neutronFlux "
          << "fired_u "
          << "fired_v "
          << "occupancy_u "
          << "occupancy_v"
          << endl;
  for (auto cmpBkg : m_data) {
    string componentName = cmpBkg.first;
    BackgroundData bgData = cmpBkg.second;
    for (auto vxdSensor : bgData.m_sensorData) {
      outfile << componentName.c_str() << " "
              << vxdSensor.first.getLayerNumber() << " "
              << vxdSensor.first.getLadderNumber() << " "
              << vxdSensor.first.getSensorNumber() << " "
              << vxdSensor.second.m_dose << " "
              << vxdSensor.second.m_expo << " "
              << vxdSensor.second.m_neutronFlux << " "
              << vxdSensor.second.m_firedU << " "
              << vxdSensor.second.m_firedV << " "
              << vxdSensor.second.m_occupancyU << " "
              << vxdSensor.second.m_occupancyV
              << endl;
    }
  }
  outfile << endl;
}
