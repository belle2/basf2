/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Zbynek Drasal, Martin Ritter, Peter Kvasnicka            *
 *                                                                        *
 **************************************************************************/

#include <svd/modules/svdSimulation/SVDDigitizerModule.h>
#include <vxd/geometry/GeoCache.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <generators/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDDigit.h>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <vector>
#include <set>
#include <math.h>
#include <root/TMath.h>
#include <root/TRandom.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;


//-----------------------------------------------------------------
//                Auxiliaries for waveform storage
//-----------------------------------------------------------------
// FIXME: I believe this is wrong, these things must be created separately for
// each instance of the digitizer, otherwise it can crash in parallel mode.
int   tree_vxdID    = 0;        // VXD ID of a sensor
int   tree_uv       = 0;        // U or V coordinate
int   tree_strip    = 0;        // number of strip
double tree_signal[20];         // array for 20 samples of 10 ns

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDigitizerModule::SVDDigitizerModule() : Module(), m_rootFile(0), m_histDiffusion_u(0), m_histDiffusion_v(0)
{
  //Set module properties
  setDescription("Create SVDDigits from SVDSimHits");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  // Define module parameters

  // 1. Collections
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticle collection name", string(""));
  addParam("Digits", m_storeDigitsName,
           "Digits collection name", string(""));
  addParam("SimHits", m_storeSimHitsName,
           "SimHit collection name", string(""));
  addParam("TrueHits", m_storeTrueHitsName,
           "TrueHit collection name", string(""));
  addParam("MCSimHitRel", m_relMCParticleSimHitName,
           "Relation between MCParticles and SimHits", string(""));
  addParam("DigitMCRel", m_relDigitMCParticleName,
           "Relation between Digits and MCParticles", string(""));
  addParam("TrueSimRel", m_relTrueHitSimHitName,
           "Relation between TrueHits and SimHits", string(""));
  addParam("DigitTrueRel", m_relDigitTrueHitName,
           "Relation between Digits and TrueHits", string(""));

  // 2. Physics
  addParam("Temperature", m_temperature,
           "Operating temperature of a sensor in Kelvins", double(300));
  addParam("SegmentLength", m_segmentLength,
           "Maximum segment length (in millimeters)", 0.020);
  addParam("WidthOfDiffusCloud", m_widthOfDiffusCloud,
           "Integration range of charge cloud in sigmas", double(3.0));

  // 3. Noise
  addParam("PoissonSmearing", m_applyPoisson,
           "Apply Poisson smearing on chargelets", true);
  addParam("ElectronicEffects", m_applyNoise,
           "Apply electronic effects?", true);
  addParam("ElectronicNoise", m_elNoise,
           "Electronic noise on strip signals in e-", double(2000.0));
  addParam("ZeroSuppressionCut", m_SNAdjacent,
           "Zero suppression cut in sigmas of strip noise", double(2.5));

  // 4. Timing
  addParam("APVShapingTime", m_shapingTime,
           "APV25 shpaing time in ns", double(50.0));
  addParam("ADCSamplingTime", m_samplingTime,
           "Interval between ADC samples in ns", double(30.0));
  addParam("UseIntegrationWindow", m_applyWindow,
           "Use integration window?", bool(true));
  addParam("WindowStart", m_startSampling,
           "Start of the sampling window, in ns", double(0.0));
  addParam("WindowEnd", m_stopSampling,
           "End of the sampling window, in ns", double(200.0));
  addParam("RandomPhaseSampling", m_randomPhaseSampling,
           "Start sampling at a random time?", bool(false));

  // 5. Processing
  addParam("ADC", m_applyADC,
           "Simulate ADC?", bool(false));
  addParam("ADCLow", m_minADC,
           "Low end of ADC range", double(-64000.0));
  addParam("ADCHigh", m_maxADC,
           "High end of ADC range", double(224000.0));
  addParam("ADCbits", m_bitsADC,
           "Number of ADC bits", int(10));

  // 6. Reporting
  addParam("statisticsFilename", m_rootFilename,
           "ROOT Filename for statistics generation. If filename is empty, no statistics will be produced", string(""));
  addParam("storeWaveforms", m_storeWaveforms,
           "Store waveforms in a TTree in the statistics file.", bool(false));
}

void SVDDigitizerModule::initialize()
{
  //Register all required collections
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<SVDSimHit>  storeSimHits(m_storeSimHitsName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<SVDDigit>   storeDigits(m_storeDigitsName);
  RelationArray relMCParticleSimHit(storeMCParticles, storeSimHits, m_relMCParticleSimHitName);
  RelationArray relTrueHitSimHit(storeTrueHits, storeSimHits, m_relTrueHitSimHitName);
  RelationArray relDigitMCParticle(storeDigits, storeMCParticles, m_relDigitMCParticleName);
  RelationArray relDigitTrueHit(storeDigits, storeTrueHits, m_relDigitTrueHitName);

  //Set names in case default was used
  m_relMCParticleSimHitName = relMCParticleSimHit.getName();
  m_relTrueHitSimHitName    = relTrueHitSimHit.getName();
  m_relDigitMCParticleName  = relDigitMCParticle.getName();
  m_relDigitTrueHitName     = relDigitTrueHit.getName();

  //Convert parameters to correct units
  m_segmentLength *= Unit::mm;
  m_noiseFraction = TMath::Freq(m_SNAdjacent); // 0.9... !
  m_elNoise *= Unit::e;
  m_minADC = m_minADC * Unit::e;
  m_maxADC = m_maxADC * Unit::e;
  m_unitADC = (m_maxADC - m_minADC) / (pow(2.0, m_bitsADC) - 1);
  m_samplingTime *= Unit::ns;
  m_shapingTime *= Unit::ns;

  B2INFO("SVDDigitizer parameters (in default system units, *=cannot be set directly):");
  B2INFO(" DATASTORE COLLECTIONS:")
  B2INFO(" -->  MCParticles:        " << storeMCParticles.getName());
  B2INFO(" -->  Digits:             " << storeDigits.getName());
  B2INFO(" -->  SimHits:            " << storeSimHits.getName());
  B2INFO(" -->  TrueHits:           " << storeTrueHits.getName());
  B2INFO(" -->  MCSimHitRel:        " << m_relMCParticleSimHitName);
  B2INFO(" -->  DigitMCRel:         " << m_relDigitMCParticleName);
  B2INFO(" -->  TrueSimRel:         " << m_relTrueHitSimHitName);
  B2INFO(" -->  DigitTrueRel:       " << m_relDigitTrueHitName);
  B2INFO(" PHYSICS: ");
  B2INFO(" -->  Temperature:        " << m_temperature);
  B2INFO(" -->  SegmentLength:      " << m_segmentLength);
  B2INFO(" -->  Charge int. range:  " << m_widthOfDiffusCloud);
  B2INFO(" NOISE: ");
  B2INFO(" -->  Add Poisson noise   " << (m_applyPoisson ? "true" : "false"));
  B2INFO(" -->  Add Gaussian noise: " << (m_applyNoise ? "true" : "false"));
  B2INFO(" -->  Gaussian noise:     " << m_elNoise);
  B2INFO(" -->  Zero suppression cut" << m_SNAdjacent);
  B2INFO(" -->  Noise fraction*:    " << 1.0 - m_noiseFraction);
  B2INFO(" TIMING: ");
  B2INFO(" -->  APV25 shaping time: " << m_shapingTime);
  B2INFO(" -->  Sampling time:      " << m_samplingTime);
  B2INFO(" -->  IntegrationWindow:  " << (m_applyWindow ? "true" : "false"));
  B2INFO(" -->  Start of int. wind.:" << m_startSampling);
  B2INFO(" -->  End of int. window: " << m_stopSampling);
  B2INFO(" -->  Random phase sampl.:" << (m_randomPhaseSampling ? "true" : "false"));
  B2INFO(" PROCESSING:");
  B2INFO(" -->  ADC:                " << (m_applyADC ? "true" : "false"));
  B2INFO(" -->  ADC range low (e-): " << m_minADC);
  B2INFO(" -->  ADC range high (e-):" << m_maxADC);
  B2INFO(" -->  ADC bits:           " << m_bitsADC);
  B2INFO(" -->  1 adu (e-)*:        " << m_unitADC);
  B2INFO(" REPORTING: ");
  B2INFO(" -->  statisticsFilename: " << m_rootFilename);
  B2INFO(" -->  storeWaveforms:     " << (m_storeWaveforms ? "true" : "false"));

  if (!m_rootFilename.empty()) {
    m_rootFile = new TFile(m_rootFilename.c_str(), "RECREATE");
    m_rootFile->cd();
    m_histDiffusion_u = new TH1D("h_holeDiffusion", "Diffusion distance, u", 200, -100, 100);
    m_histDiffusion_u->GetXaxis()->SetTitle("Diffusion distance u [um]");
    m_histDiffusion_v = new TH1D("h_electronDiffusion", "Diffusion distance", 200, -100, 100);
    m_histDiffusion_v->GetXaxis()->SetTitle("Diffusion distance v [um]");
    m_histLorentz_u = new TH1D("h_LorentzAngle_u", "Lorentz angle, holes", 100, -0.1, 0.0);
    m_histLorentz_u->GetXaxis()->SetTitle("Lorentz angle");
    m_histLorentz_v = new TH1D("h_LorentzAngle_v", "Lorentz angle, electrons", 100, -0.02, 0.02);
    m_histLorentz_v->GetXaxis()->SetTitle("Lorentz angle");
    m_signalDist_u = new TH1D("h_signalDist_u", "Strip signals vs. TrueHits, holes", 100, -400, 400);
    m_signalDist_u->GetXaxis()->SetTitle("U strip position - TrueHit u [um]");
    m_signalDist_v = new TH1D("h_signalDist_v", "Strip signals vs. TrueHits, electrons", 100, -400, 400);
    m_signalDist_v->GetXaxis()->SetTitle("V strip position - TrueHit v [um]");

    if (m_storeWaveforms) {
      m_waveTree = new TTree("waveTree", "SVD waveforms");
      m_waveTree->Branch("sensor", &tree_vxdID, "sensor/I");
      m_waveTree->Branch("u_or_v", &tree_uv, "u_or_v/I");
      m_waveTree->Branch("strip", &tree_strip, "strip/I");
      m_waveTree->Branch("signal", tree_signal, "signal[20]/D");
    }
  }
  // Check if the global random number generator is available.
  if (!gRandom) B2FATAL("gRandom not initialized, please set up gRandom first");
}

void SVDDigitizerModule::beginRun()
{
  //Fill map with all possible sensors This is too slow to be done every event so
  //we fill it once and only clear the content of the sensors per event, not
  //the whole map
  m_sensors.clear();
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  BOOST_FOREACH(VxdID layer, geo.getLayers(SensorInfo::SVD)) {
    BOOST_FOREACH(VxdID ladder, geo.getLadders(layer)) {
      BOOST_FOREACH(VxdID sensor, geo.getSensors(ladder)) {
        m_sensors[sensor] = Sensor();
      }
    }
  }
}


void SVDDigitizerModule::event()
{
  //Clear sensors and process SimHits
  BOOST_FOREACH(Sensors::value_type & sensor, m_sensors) {
    sensor.second.first.clear();  // u strips
    sensor.second.second.clear(); // v strips
  }
  m_currentSensor = 0;
  m_currentSensorInfo = 0;

  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<SVDSimHit>  storeSimHits(m_storeSimHitsName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  unsigned int nSimHits = storeSimHits->GetEntries();
  if (nSimHits == 0) return;

  RelationIndex<MCParticle, SVDSimHit> relMCParticleSimHit(storeMCParticles, storeSimHits, m_relMCParticleSimHitName);
  RelationIndex<SVDTrueHit, SVDSimHit> relTrueHitSimHit(storeTrueHits, storeSimHits, m_relTrueHitSimHitName);

  // Clear old SVDDigits
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  RelationArray relDigitMCParticle(storeDigits, storeMCParticles, m_relDigitMCParticleName);
  RelationArray relDigitTrueHit(storeDigits, storeTrueHits, m_relDigitTrueHitName);
  storeDigits->Clear();
  relDigitMCParticle.clear();
  relDigitTrueHit.clear();

  //Check sensor info and set pointers to current sensor
  for (unsigned int i = 0; i < nSimHits; ++i) {
    m_currentHit = storeSimHits[i];
    const RelationIndex<MCParticle, SVDSimHit>::Element* mcRel = relMCParticleSimHit.getFirstFrom(m_currentHit);
    if (mcRel) {
      m_currentParticle = mcRel->indexFrom;
    } else {
      B2ERROR("Could not find MCParticle which produced SVDSimhit " << i);
      m_currentParticle = -1;
    }
    const RelationIndex<SVDTrueHit, SVDSimHit>::Element* trueRel = relTrueHitSimHit.getFirstFrom(m_currentHit);
    if (trueRel) {
      m_currentTrueHit = trueRel->indexFrom;
    } else {
      m_currentTrueHit = -1;
    }

    VxdID sensorID = m_currentHit->getSensorID();
    if (!m_currentSensorInfo || sensorID != m_currentSensorInfo->getID()) {
      m_currentSensorInfo = dynamic_cast<const SensorInfo*>(&VXD::GeoCache::get(sensorID));
      if (!m_currentSensorInfo)
        B2FATAL("Sensor Information for Sensor " << sensorID << " not found, make sure that the geometry is set up correctly");

      const SensorInfo& info  = *m_currentSensorInfo;
      // Publish some useful data
      m_sensorThickness       = info.getThickness();
      m_depletionVoltage      = info.getDepletionVoltage();
      m_biasVoltage           = info.getBiasVoltage();
      m_backplaneCapacitance  = info.getBackplaneCapacitance();
      m_interstripCapacitance = info.getInterstripCapacitance();
      m_couplingCapacitance   = info.getCouplingCapacitance();

      m_currentSensor = &m_sensors[sensorID];
      B2DEBUG(20, "Sensor Parameters for Sensor " << sensorID << ": " << endl
              << " --> Width:          " << m_currentSensorInfo->getWidth() << endl
              << " --> Length:         " << m_currentSensorInfo->getLength() << endl
              << " --> uPitch:         " << m_currentSensorInfo->getUPitch() << endl
              << " --> vPitch:         " << m_currentSensorInfo->getVPitch(-m_currentSensorInfo->getLength() / 2.0)
              << ", " << m_currentSensorInfo->getVPitch(m_currentSensorInfo->getLength() / 2.0) << endl
              << " --> Thickness:      " << m_currentSensorInfo->getThickness() << endl
              << " --> Deplet. voltage:" << m_currentSensorInfo->getDepletionVoltage() << endl
              << " --> Bias voltage:   " << m_currentSensorInfo->getBiasVoltage() << endl
              << " --> Backplane cap.: " << m_currentSensorInfo->getBackplaneCapacitance() << endl
              << " --> Interstrip cap.:" << m_currentSensorInfo->getInterstripCapacitance() << endl
              << " --> Coupling cap.:  " << m_currentSensorInfo->getCouplingCapacitance() << endl
             );

    }
    B2DEBUG(10, "Processing hit " << i << " in Sensor " << sensorID << ", related to MCParticle " << m_currentParticle);
    processHit();
  }
  // If storage of waveforms is required, store them in the statistics file.
  if (m_waveTree) {
    m_rootFile->cd();
    saveWaveforms();
  }
  // Take samples of acquired signals and store as digits.
  double initTime = m_startSampling;
  if (m_randomPhaseSampling) {
    initTime = gRandom->Uniform(0.0, m_samplingTime);
  }
  double time = initTime;
  while (time < m_stopSampling) {
    saveDigits(time);
    time += m_samplingTime;
  }
}

void SVDDigitizerModule::processHit()
{
  if (m_applyWindow) {
    //Ignore hits which are outside of the SVD active time frame.
    //Here we can only discard hits that arrived after the window was closed;
    //we will later start sampling only after the window opened.
    B2DEBUG(30, "Checking if hit is in timeframe "
            << m_currentHit->getGlobalTime() << " <= " << m_stopSampling
           );

    if (m_currentHit->getGlobalTime() > m_stopSampling) return;
  }
  // Set time of the event
  m_currentTime = m_currentHit->getGlobalTime();

  //Get Steplength and direction
  const TVector3& startPoint = m_currentHit->getPosIn();
  const TVector3& stopPoint = m_currentHit->getPosOut();
  TVector3 direction = stopPoint - startPoint;
  double trackLength = direction.Mag();
  //Calculate the number of electrons and holes
  double carriers = m_currentHit->getEnergyDep() * Unit::GeV / Unit::ehEnergy;

  if (m_currentHit->getPDGcode() == 22 || trackLength == 0) {
    //Photons deposit the energy at the end of their step
    driftCharge(stopPoint, carriers);
  } else {
    //Otherwise, split into segments of (default) max. 5µm and
    //drift the charges from the center of each segment
    int numberOfSegments = (int)(trackLength / m_segmentLength) + 1;
    double segmentLength = trackLength / numberOfSegments;
    carriers /= numberOfSegments;
    direction.SetMag(1.0);

    for (int segment = 0; segment < numberOfSegments; ++segment) {
      TVector3 position = startPoint + direction * segmentLength * (segment + 0.5);
      driftCharge(position, carriers);
    }
  }
}


double SVDDigitizerModule::getElectronMobility(double E) const
{
  // Electron parameters - maximum velocity, critical intensity, beta factor
  static double vmElec   = 1.53 * pow(m_temperature, -0.87) * 1.E9 * Unit::cm / Unit::s;
  static double EcElec   = 1.01 * pow(m_temperature, +1.55) * Unit::V / Unit::cm;
  static double betaElec = 2.57 * pow(m_temperature, +0.66) * 1.E-2;

  return (vmElec / EcElec * 1. / (pow(1. + pow((fabs(E) / EcElec), betaElec), (1. / betaElec))));
}

double SVDDigitizerModule::getHoleMobility(double E) const
{
  // Hole parameters - maximum velocity, critical intensity, beta factor
  static double vmHole   = 1.62 * pow(m_temperature, -0.52) * 1.E8 * Unit::cm / Unit::s;
  static double EcHole   = 1.24 * pow(m_temperature, +1.68) * Unit::V / Unit::cm;
  static double betaHole = 0.46 * pow(m_temperature, +0.17);

  return (vmHole / EcHole * 1. / (pow(1. + pow((fabs(E) / EcHole), betaHole), (1. / betaHole))));
}

const TVector3 SVDDigitizerModule::getEField(const TVector3& point) const
{
  TVector3 E(0, 0, 2.0 * m_depletionVoltage / m_sensorThickness * (point.Z() + 0.5 * m_sensorThickness)
             - (m_biasVoltage - m_depletionVoltage) / m_sensorThickness);
  return E;
}

const TVector3 SVDDigitizerModule::getBField(const TVector3& point) const
{
  TVector3 pointGlobal = m_currentSensorInfo->pointToGlobal(point);
  TVector3 bGlobal = BFieldMap::Instance().getBField(pointGlobal);
  TVector3 bLocal = m_currentSensorInfo->vectorToLocal(bGlobal);
  // FIXME: Unit::T is wrong, Tesla = V.s/m^2. Take care when it changes!!!
  double Unit_T = Unit::V * Unit::s / Unit::m2;
  return Unit_T * bLocal;
}

const TVector3 SVDDigitizerModule::getVelocity(CarrierType carrier, const TVector3& point) const
{
  TVector3 E = getEField(point);
  TVector3 B = getBField(point);
  // Set mobility parameters
  double mobility = 0;
  double hallFactor = 0;
  if (carrier == electron) {
    mobility = - getElectronMobility(E.Mag());
    hallFactor = 1.13 + 0.0008 * (m_temperature - 273) * Unit::m2 / Unit::s;
  } else {
    mobility = getHoleMobility(E.Mag());
    hallFactor = 0.72 - 0.0005 * (m_temperature - 273) * Unit::m2 / Unit::s;
  }
  double mobilityH = hallFactor * mobility;
  // Calculate products
  TVector3 EcrossB = E.Cross(B);
  TVector3 BEdotB = E.Dot(B) * B;
  TVector3 v = mobility * E + mobility * mobilityH * EcrossB +
               + mobility * mobilityH * mobilityH * BEdotB;
  v *= 1.0 / (1.0 + mobilityH * mobilityH * B.Mag2());
  return v;
}


void SVDDigitizerModule::driftCharge(const TVector3& position, double carriers)
{
  B2DEBUG(30, "Drifting " << carriers << " carriers at position ("
          << position.x() << ", "
          << position.y() << ", "
          << position.z() << ")."
         );

  //Get references to current sensor/info for ease of use
  const SensorInfo& info = *m_currentSensorInfo;
  Sensor& sensor = *m_currentSensor;

  double distanceToFrontPlane = 0.5 * m_sensorThickness - position.Z();
  double distanceToBackPlane  = 0.5 * m_sensorThickness + position.Z();

  // Approximation: calculate drift velocity at the point halfway towards
  // the respective sensor surface.
  TVector3 mean_e(position.X(), position.Y(), 0.5 * (position.Z() + m_sensorThickness));
  TVector3 mean_h(position.X(), position.Y(), 0.5 * (position.Z() - m_sensorThickness));

  // Calculate drift times and widths of charge clouds.
  // Electrons:
  TVector3 v_e = getVelocity(electron, mean_e);
  double driftTime_e = distanceToFrontPlane / v_e.Z();
  TVector3 center_e = position + driftTime_e * v_e;
  double D_e = Unit::kBoltzmann * m_temperature / Unit::e * getElectronMobility(getEField(mean_e).Mag());
  double sigma_e = sqrt(2.0 * D_e * driftTime_e);
  double tanLorentz_e = v_e.Y() / v_e.Z();
  sigma_e *= sqrt(1.0 + tanLorentz_e * tanLorentz_e);
  if (m_histLorentz_v) m_histLorentz_v->Fill(tanLorentz_e);

  // Holes
  TVector3 v_h = getVelocity(hole, mean_h);
  double driftTime_h = - distanceToBackPlane / v_h.Z();
  TVector3 center_h = position + driftTime_h * v_h;
  double D_h = Unit::kBoltzmann * m_temperature / Unit::e * getElectronMobility(getEField(mean_h).Mag());
  double sigma_h = sqrt(2.0 * D_h * driftTime_h);
  double tanLorentz_h = v_h.X() / v_h.Z(); // Is this OK?
  sigma_h *= sqrt(1.0 + tanLorentz_h * tanLorentz_h);
  if (m_histLorentz_u) m_histLorentz_u->Fill(tanLorentz_h);

  //Determine strips hit by the electron cloud
  double vLow    = center_e.Y() - m_widthOfDiffusCloud * sigma_e;
  double vHigh   = center_e.Y() + m_widthOfDiffusCloud * sigma_e;
  int    vIDLow  = info.getVCellID(vLow, true);
  int    vIDHigh = info.getVCellID(vHigh, true);
  B2DEBUG(30, "Size of diffusion cloud (e): " << sigma_e);
  B2DEBUG(30, "vID from " << vIDLow << " to " << vIDHigh);

  //Determine strips hit by the hole cloud
  double uLow    = center_h.X() - m_widthOfDiffusCloud * sigma_h;
  double uHigh   = center_h.X() + m_widthOfDiffusCloud * sigma_h;
  int    uIDLow  = info.getUCellID(uLow, center_h.Y(), true);
  int    uIDHigh = info.getUCellID(uHigh, center_h.Y(), true);
  B2DEBUG(30, "Size of diffusion cloud (h): " << sigma_h);
  B2DEBUG(30, "uID from " << uIDLow << " to " << uIDHigh);

  // FIXME: Doing the following separately for electrons and holes loses the
  // correlation between Poisson fluctuations in numbers of electrons and holes.
  // But Poisson fluctuations are apparently unimportant as they are applied here.

  //Now loop over strips and calculate the integral of the gaussian charge distributions.
  //Deposit the charge corresponding to the per strip integral in each strip.
  //This is currently the only place where intermediate strips come into play:

#define NORMAL_CDF(mean,sigma,x) TMath::Freq(((x)-(mean))/(sigma))

  // Electrons
  // Add one more readout strip on each end of the region to make place for charges
  // induced by capacitive coupling.
  if (vIDLow > 0) vIDLow--;
  if (vIDHigh < info.getVCells() - 1) vIDHigh++;
  double vGeomPitch = 0.5 * info.getVPitch();
  // We have to store the strip charges to calculate cross-talk.
  double fraction = 0;
  int nStrips_e = 2 * (vIDHigh - vIDLow) + 1;
  std::vector<double> eStripCharges(nStrips_e);
  double vPos = info.getVCellPosition(vIDLow);
  double vLowerTail = NORMAL_CDF(center_e.Y(), sigma_e, vPos - 0.5 * vGeomPitch);
  for (int vID = 0; vID < nStrips_e; ++vID) {
    double vUpperTail = NORMAL_CDF(center_e.Y(), sigma_e, vPos + 0.5 * vGeomPitch);
    double vIntegral  = vUpperTail - vLowerTail;
    // Can fail on far tails
    if (TMath::IsNaN(vIntegral)) vIntegral = 0.0;
    vLowerTail = vUpperTail;
    double charge = carriers * vIntegral;
    if (m_applyPoisson) {
      //Actually, Poisson takes really long to calculate for large values.
      //We use a (truncated) gaussian instead.
      charge = gRandom->Gaus(charge, sqrt(FanoFactorSi * charge));
      if (charge < 0.0) charge = 0.0;
    }
    eStripCharges[vID] = charge;
    B2DEBUG(80, "Relative charge for strip (" << vIDLow + 0.5 * vID << "): " << vIntegral);
    fraction += vIntegral;
    if (m_histDiffusion_v && charge >= 1.0)
      m_histDiffusion_v->Fill((vPos - center_e.Y()) / Unit::um, charge);
    vPos += vGeomPitch;
  }
  B2DEBUG(30, "Fraction of charge (e): " << fraction);
  // The strip signals combine due to capacitive coupling.
  // FIXME: We have the same parameters for n and p strips. Is this correct?
  double correctedCharge = 0;
  double cNeighbour2 = 0.5 * info.getInterstripCapacitance()
                       / (0.5 * info.getInterstripCapacitance() + info.getBackplaneCapacitance() + info.getCouplingCapacitance());
  double cNeighbour1 = 0.5;
  double cSelf = 1.0 - 2.0 * cNeighbour2;
  //Leftmost strip (there must be at least one readout strip to the right):
  int arrayIndex = 0;
  if (vIDLow == 0) {
    correctedCharge =
      (cNeighbour2 + cSelf) * eStripCharges[arrayIndex]
      + cNeighbour1 * eStripCharges[arrayIndex + 1]
      + cNeighbour2 * eStripCharges[arrayIndex + 2];
  } else {
    correctedCharge =
      cSelf * eStripCharges[arrayIndex]
      + cNeighbour1 * eStripCharges[arrayIndex + 1]
      + cNeighbour2 * eStripCharges[arrayIndex + 2];
  }
  sensor.second[vIDLow].add(m_currentTime + driftTime_e, correctedCharge, m_shapingTime, m_currentParticle, m_currentTrueHit);

  for (int vID = vIDLow + 1; vID < vIDHigh; ++vID) {
    arrayIndex = 2 * (vID - vIDLow);
    correctedCharge =
      cNeighbour2 * eStripCharges[arrayIndex - 2]
      + cNeighbour1 * eStripCharges[arrayIndex - 1]
      + cSelf * eStripCharges[arrayIndex]
      + cNeighbour1 * eStripCharges[arrayIndex + 1]
      + cNeighbour2 * eStripCharges[arrayIndex + 2];
    sensor.second[vID].add(m_currentTime + driftTime_e, correctedCharge, m_shapingTime, m_currentParticle, m_currentTrueHit);
  }
  // Rightmost strip (there must be at least one readout strip to the left):
  arrayIndex = 2 * (vIDHigh - vIDLow);
  if (vIDHigh == info.getVCells()) {
    correctedCharge =
      cNeighbour2 * eStripCharges[arrayIndex - 2]
      + cNeighbour2 * eStripCharges[arrayIndex - 1]
      + (cSelf + cNeighbour2) * eStripCharges[arrayIndex];
  } else {
    correctedCharge =
      cNeighbour2 * eStripCharges[arrayIndex - 2]
      + cNeighbour2 * eStripCharges[arrayIndex - 1]
      + cNeighbour2 * eStripCharges[arrayIndex];
  }
  sensor.second[vIDHigh].add(m_currentTime + driftTime_e, correctedCharge, m_shapingTime, m_currentParticle, m_currentTrueHit);

  //Holes
  // Add one more readout strip on each end of the region to make place for charges
  // induced by capacitive coupling.
  if (uIDLow > 0) uIDLow--;
  if (uIDHigh < info.getUCells() - 1) uIDHigh++;
  double uGeomPitch = 0.5 * info.getUPitch(center_h.Y());
  // We have to store the strip charges to later calculate cross-talk.
  fraction = 0;
  int nStrips_h = 2 * (uIDHigh - uIDLow) + 1;
  std::vector<double> hStripCharges(nStrips_h);
  double uPos = info.getUCellPosition(uIDLow, info.getVCellID(center_h.Y()));
  double uLowerTail = NORMAL_CDF(center_h.X(), sigma_h, uPos - 0.5 * uGeomPitch);
  for (int uID = 0; uID < nStrips_h; ++uID) {
    double uUpperTail = NORMAL_CDF(center_h.X(), sigma_h, uPos + 0.5 * uGeomPitch);
    double uIntegral  = uUpperTail - uLowerTail;
    // Can fail on far tails
    if (TMath::IsNaN(uIntegral)) uIntegral = 0.0;
    uLowerTail = uUpperTail;
    double charge = carriers * uIntegral;
    if (m_applyPoisson) {
      //Actually, Poisson takes really long to calculate for large values.
      //We use a (truncated) gaussian instead.
      charge = gRandom->Gaus(charge, sqrt(FanoFactorSi * charge));
      if (charge < 0.0) charge = 0.0;
    }
    hStripCharges[uID] = charge;
    B2DEBUG(80, "Relative charge for strip (" << uIDLow + 0.5 * uID << "): " << uIntegral);
    fraction += uIntegral;
    if (m_histDiffusion_u && charge >= 1.0)
      m_histDiffusion_u->Fill((uPos - center_h.X()) / Unit::um, charge);
    uPos += uGeomPitch;
  }
  B2DEBUG(30, "Fraction of charge (h): " << fraction);
  // The strip signals combine due to capacitive coupling.
  //Leftmost strip (there must be at least one readout strip to the right):
  arrayIndex = 0;
  if (uIDLow == 0) {
    correctedCharge =
      cSelf * hStripCharges[arrayIndex]
      + cNeighbour1 * hStripCharges[arrayIndex + 1]
      + cNeighbour2 * hStripCharges[arrayIndex + 2];
  } else {
    correctedCharge =
      (cNeighbour2 + cSelf) * hStripCharges[arrayIndex]
      + cNeighbour1 * hStripCharges[arrayIndex + 1]
      + cNeighbour2 * hStripCharges[arrayIndex + 2];
  }
  sensor.first[uIDLow].add(m_currentTime + driftTime_h, correctedCharge, m_shapingTime, m_currentParticle, m_currentTrueHit);
  for (int uID = uIDLow + 1; uID < uIDHigh; ++uID) {
    arrayIndex = 2 * (uID - uIDLow);
    correctedCharge =
      cNeighbour2 * hStripCharges[arrayIndex - 2]
      + cNeighbour1 * hStripCharges[arrayIndex - 1]
      + cSelf * hStripCharges[arrayIndex]
      + cNeighbour1 * hStripCharges[arrayIndex + 1]
      + cNeighbour2 * hStripCharges[arrayIndex + 2];
    sensor.first[uID].add(m_currentTime + driftTime_h, correctedCharge, m_shapingTime, m_currentParticle, m_currentTrueHit);
  }
  // Rightmost strip (there must be at least one readout strip to the left):
  arrayIndex = 2 * (uIDHigh - uIDLow);
  if (uIDHigh == info.getUCells()) {
    correctedCharge =
      cNeighbour2 * hStripCharges[arrayIndex - 2]
      + cNeighbour2 * hStripCharges[arrayIndex - 1]
      + (cSelf + cNeighbour2) * hStripCharges[arrayIndex];
  } else {
    correctedCharge =
      cNeighbour2 * hStripCharges[arrayIndex - 2]
      + cNeighbour2 * hStripCharges[arrayIndex - 1]
      + cSelf * hStripCharges[arrayIndex];
  }
  sensor.first[uIDHigh].add(m_currentTime + driftTime_h, correctedCharge, m_shapingTime, m_currentParticle, m_currentTrueHit);

#undef NORMAL_CDF
}


double SVDDigitizerModule::addNoise(double charge)
{
  if (charge < 0) {
    //Noise Pixel, add noise to exceed Noise Threshold;
    double p = gRandom->Uniform(m_noiseFraction, 1.0);
    charge = TMath::NormQuantile(p) * m_elNoise;
  } else {
    if (m_applyNoise) {
      charge += gRandom->Gaus(0., m_elNoise);
    }
  }
  return charge;
}


void SVDDigitizerModule::saveDigits(double time)
{
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<SVDDigit>   storeDigits(m_storeDigitsName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  RelationArray relDigitMCParticle(storeDigits, storeMCParticles, m_relDigitMCParticleName);
  RelationArray relDigitTrueHit(storeDigits, storeTrueHits, m_relDigitTrueHitName);

  //Zero suppression cut in electrons
  double charge_threshold = m_SNAdjacent * m_elNoise;

  // Take sample at the desired time, add noise, zero-suppress and save digits.

  BOOST_FOREACH(Sensors::value_type & sensor, m_sensors) {
    int sensorID = sensor.first;
    const SensorInfo& info = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));
    // u-side digits:
    // Remember which strips are occupied by signal
    std::set<short> occupied_u;
    BOOST_FOREACH(StripSignals::value_type & stripSignal, sensor.second.first) {
      short int iStrip = stripSignal.first;
      SVDSignal& s = stripSignal.second;
      // Read the value
      double charge;
      SVDSignal::relations_map particles;
      SVDSignal::relations_map truehits;
      boost::tie(charge, particles,
                 truehits) = s(time);
      // Add noise to charge.
      double sampleCharge = addNoise(charge);
      // Zero-suppress if too small signal
      if (sampleCharge < charge_threshold) continue;
      //Limit signal to ADC steps
      if (m_applyADC)
        sampleCharge = round(min(m_maxADC, max(m_minADC, sampleCharge)) / m_unitADC) * m_unitADC;
      // Remember the strip as occupied
      occupied_u.insert(iStrip);

      // Save as a new digit
      int digIndex = storeDigits->GetLast() + 1;
      new(storeDigits->AddrAt(digIndex)) SVDDigit(
        sensorID, true, iStrip, info.getUCellPosition(iStrip), sampleCharge,
        time
      );

      //If the digit has any relations to MCParticles, add the Relation
      if (particles.size() > 0) {
        relDigitMCParticle.add(digIndex, particles.begin(), particles.end());
      }
      //If the digit has any relations to truehits, add the Relations.
      if (truehits.size() > 0) {
        relDigitTrueHit.add(digIndex, truehits.begin(), truehits.end());
        // Add reporting data
        if (m_signalDist_u) {
          BOOST_FOREACH(SVDSignal::relation_value_type trueRel, truehits) {
            int iTrueHit = trueRel.first;
            float trueWeight = trueRel.second;
            if (iTrueHit > -1)
              m_signalDist_u->Fill(
                (info.getUCellPosition(iStrip) - storeTrueHits[iTrueHit]->getU()) / Unit::um,
                trueWeight
              );
          }
        }
      }
    } // FOREACH stripSignal
    // Add noisy digits
    if (m_applyNoise) {
      double fraction = 1.0 - m_noiseFraction;
      int nU = info.getUCells();
      int uNoisyStrips = gRandom->Poisson(fraction * nU);
      for (short ns = 0; ns < uNoisyStrips; ++ns) {
        short iStrip = gRandom->Integer(nU);
        if (occupied_u.count(iStrip) > 0) continue;
        // Add a noisy digit, no relations.
        int digIndex = storeDigits->GetLast() + 1;
        new(storeDigits->AddrAt(digIndex)) SVDDigit(
          sensorID, true, iStrip, info.getUCellPosition(iStrip), addNoise(-1.0),
          time
        );
      } // for ns
    } // apply noise

    // v-side digits:
    // Remember which strips are occupied by signal
    std::set<short> occupied_v;
    BOOST_FOREACH(StripSignals::value_type & stripSignal, sensor.second.second) {
      short int iStrip = stripSignal.first;
      SVDSignal& s = stripSignal.second;
      // Read the value
      double charge;
      SVDSignal::relations_map particles;
      SVDSignal::relations_map truehits;
      boost::tie(charge, particles, truehits) = s(time);
      // Add noise to charge.
      double sampleCharge = addNoise(charge);
      // Zero-suppress if too small signal
      if (sampleCharge < charge_threshold) continue;
      //Limit signal to ADC steps
      if (m_applyADC)
        sampleCharge = round(min(m_maxADC, max(m_minADC, sampleCharge)) / m_unitADC) * m_unitADC;
      // Remember the strip as occupied
      occupied_v.insert(iStrip);

      // Save as a new digit
      int digIndex = storeDigits->GetLast() + 1;
      new(storeDigits->AddrAt(digIndex)) SVDDigit(
        sensorID, false, iStrip, info.getVCellPosition(iStrip), sampleCharge,
        time
      );

      //If the digit has any relations to MCParticles, add the Relation
      if (particles.size() > 0) {
        relDigitMCParticle.add(digIndex, particles.begin(), particles.end());
      }
      //If the digit has any relations to truehits, add the Relations
      if (truehits.size() > 0) {
        relDigitTrueHit.add(digIndex, truehits.begin(), truehits.end());
        // Add reporting data
        if (m_signalDist_v) {
          BOOST_FOREACH(SVDSignal::relation_value_type trueRel, truehits) {
            int iTrueHit = trueRel.first;
            float trueWeight = trueRel.second;
            if (iTrueHit > -1)
              m_signalDist_v->Fill(
                (info.getVCellPosition(iStrip) - storeTrueHits[iTrueHit]->getV()) / Unit::um,
                trueWeight
              );
          }
        }
      }
    } // FOREACH stripSignal
    // Add noisy digits
    if (m_applyNoise) {
      double fraction = 1.0 - m_noiseFraction;
      int nV = info.getVCells();
      int vNoisyStrips = gRandom->Poisson(fraction * nV);
      for (short ns = 0; ns < vNoisyStrips; ++ns) {
        short iStrip = gRandom->Integer(nV);
        if (occupied_v.count(iStrip) > 0) continue;
        // Add a noisy digit, no relations.
        int digIndex = storeDigits->GetLast() + 1;
        new(storeDigits->AddrAt(digIndex)) SVDDigit(
          sensorID, false, iStrip, info.getVCellPosition(iStrip), addNoise(-1.0),
          time
        );
      } // for ns
    } // apply noise

  } // FOREACH sensor
}

void SVDDigitizerModule::saveWaveforms()
{
  //Only store large enough signals
  double charge_threshold = 3. * m_elNoise;

  BOOST_FOREACH(Sensors::value_type & sensor, m_sensors) {
    tree_vxdID = sensor.first;
    // u-side digits:
    tree_uv = 1;
    BOOST_FOREACH(StripSignals::value_type & stripSignal, sensor.second.first) {
      tree_strip = stripSignal.first;
      SVDSignal& s = stripSignal.second;
      // Read the value only if the signal is large enough.
      if (s.getCharge() < charge_threshold) continue;
      double charge;
      SVDSignal::relations_map particles;
      SVDSignal::relations_map truehits;
      for (int iTime = 0; iTime < 20; ++iTime) {
        boost::tie(charge, particles, truehits) = s(10 * iTime);
        tree_signal[iTime] = charge;
      }
      m_waveTree->Fill();
    } // FOREACH stripSignal
    // v-side digits:
    tree_uv = 0;
    BOOST_FOREACH(StripSignals::value_type & stripSignal, sensor.second.second) {
      tree_strip = stripSignal.first;
      SVDSignal& s = stripSignal.second;
      // Read the values only if the signal is large enough
      if (s.getCharge() < charge_threshold) continue;
      double charge;
      SVDSignal::relations_map particles;
      SVDSignal::relations_map truehits;
      for (int iTime = 0; iTime < 20; ++iTime) {
        boost::tie(charge, particles, truehits) = s(10.*iTime);
        tree_signal[iTime] = charge;
      }
      m_waveTree->Fill();
    } // FOREACH stripSignal
  } // FOREACH sensor
  m_rootFile->Flush();
}


void SVDDigitizerModule::terminate()
{
  if (m_rootFile) {
    m_rootFile->Write();
    m_rootFile->Close();
  }
}


