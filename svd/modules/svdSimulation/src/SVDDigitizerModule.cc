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
#include <framework/gearbox/Const.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDDigit.h>
#include <boost/tuple/tuple.hpp>
#include <algorithm>
#include <numeric>
#include <deque>
#include <cmath>
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
int tree_vxdID = 0;        // VXD ID of a sensor
int tree_uv = 0;        // U or V coordinate
int tree_strip = 0;        // number of strip
double tree_signal[20];         // array for 20 samples of 10 ns

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDigitizerModule::SVDDigitizerModule() :
  Module(), m_rootFile(0), m_histDiffusion_u(0), m_histDiffusion_v(0), m_histLorentz_u(
    0), m_histLorentz_v(0), m_signalDist_u(0), m_signalDist_v(0)
{
  //Set module properties
  setDescription("Create SVDDigits from SVDSimHits");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Define module parameters

  // 1. Collections
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticle collection name", string(""));
  addParam("Digits", m_storeDigitsName, "Digits collection name", string(""));
  addParam("SimHits", m_storeSimHitsName, "SimHit collection name",
           string(""));
  addParam("TrueHits", m_storeTrueHitsName, "TrueHit collection name",
           string(""));

  // 2. Physics
  addParam("SegmentLength", m_segmentLength,
           "Maximum segment length (in millimeters)", 0.020);
  addParam("WidthOfDiffusCloud", m_widthOfDiffusCloud,
           "Integration range of charge cloud in sigmas", double(3.0));

  // 3. Noise
  addParam("PoissonSmearing", m_applyPoisson,
           "Apply Poisson smearing on chargelets", true);
  addParam("ElectronicEffects", m_applyNoise, "Apply electronic effects?",
           true);
  addParam("ZeroSuppressionCut", m_SNAdjacent,
           "Zero suppression cut in sigmas of strip noise", double(2.5));

  // 4. Timing
  addParam("APVShapingTime", m_shapingTime, "APV25 shpaing time in ns",
           double(50.0));
  addParam("ADCSamplingTime", m_samplingTime,
           "Interval between ADC samples in ns", double(31.44));
  addParam("UseIntegrationWindow", m_applyWindow, "Use integration window?",
           bool(true));
  addParam("StartSampling", m_startSampling,
           "Start of the sampling window, in ns", double(-31.44));
  addParam("nAPV25Samples", m_nAPV25Samples, "number of APV25 samples",
           6);
  addParam("RandomPhaseSampling", m_randomPhaseSampling,
           "Start sampling at a random time?", bool(false));

  // 5. Processing
  addParam("ADC", m_applyADC, "Simulate ADC?", bool(true));
  addParam("ADCLow", m_minADC, "Low end of ADC range", double(-96000.0));
  addParam("ADCHigh", m_maxADC, "High end of ADC range", double(288000.0));
  addParam("ADCbits", m_bitsADC, "Number of ADC bits", int(10));

  // 6. Reporting
  addParam("statisticsFilename", m_rootFilename,
           "ROOT Filename for statistics generation. If filename is empty, no statistics will be produced",
           string(""));
  addParam("storeWaveforms", m_storeWaveforms,
           "Store waveforms in a TTree in the statistics file.", bool(false));
}

void SVDDigitizerModule::initialize()
{
  //Register all required collections
  StoreArray<SVDDigit>storeDigits(m_storeDigitsName);
  storeDigits.registerInDataStore();

  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  storeDigits.registerRelationTo(storeMCParticles);

  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  storeDigits.registerRelationTo(storeTrueHits);

  //Set names in case default was used. We need the names to initialize the RelationIndices.
  m_relMCParticleSimHitName = DataStore::relationName(
                                DataStore::arrayName<MCParticle>(m_storeMCParticlesName),
                                DataStore::arrayName<SVDSimHit>(m_storeSimHitsName));
  m_relTrueHitSimHitName = DataStore::relationName(
                             DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName),
                             DataStore::arrayName<SVDSimHit>(m_storeSimHitsName));
  m_relDigitMCParticleName = DataStore::relationName(
                               DataStore::arrayName<SVDDigit>(m_storeDigitsName),
                               DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  m_relDigitTrueHitName = DataStore::relationName(
                            DataStore::arrayName<SVDDigit>(m_storeDigitsName),
                            DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName));

  //Convert parameters to correct units
  m_segmentLength *= Unit::mm;
  m_noiseFraction = TMath::Freq(m_SNAdjacent); // 0.9... !
  m_minADC = m_minADC * Unit::e;
  m_maxADC = m_maxADC * Unit::e;
  m_unitADC = (m_maxADC - m_minADC) / (pow(2.0, m_bitsADC) - 1);
  m_samplingTime *= Unit::ns;
  m_shapingTime *= Unit::ns;

  B2INFO(
    "SVDDigitizer parameters (in default system units, *=cannot be set directly):");
  B2INFO(" DATASTORE COLLECTIONS:");
  B2INFO(
    " -->  MCParticles:        " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2INFO(
    " -->  Digits:             " << DataStore::arrayName<SVDDigit>(m_storeDigitsName));
  B2INFO(
    " -->  SimHits:            " << DataStore::arrayName<SVDSimHit>(m_storeSimHitsName));
  B2INFO(
    " -->  TrueHits:           " << DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName));
  B2INFO(" -->  MCSimHitRel:        " << m_relMCParticleSimHitName);
  B2INFO(" -->  DigitMCRel:         " << m_relDigitMCParticleName);
  B2INFO(" -->  TrueSimRel:         " << m_relTrueHitSimHitName);
  B2INFO(" -->  DigitTrueRel:       " << m_relDigitTrueHitName);
  B2INFO(" PHYSICS: ");
  B2INFO(" -->  SegmentLength:      " << m_segmentLength);
  B2INFO(" -->  Charge int. range:  " << m_widthOfDiffusCloud);
  B2INFO(" NOISE: ");
  B2INFO(" -->  Add Poisson noise   " << (m_applyPoisson ? "true" : "false"));
  B2INFO(" -->  Add Gaussian noise: " << (m_applyNoise ? "true" : "false"));
  B2INFO(" -->  Zero suppression cut" << m_SNAdjacent);
  B2INFO(" -->  Noise fraction*:    " << 1.0 - m_noiseFraction);
  B2INFO(" TIMING: ");
  B2INFO(" -->  APV25 shaping time: " << m_shapingTime);
  B2INFO(" -->  Sampling time:      " << m_samplingTime);
  B2INFO(" -->  IntegrationWindow:  " << (m_applyWindow ? "true" : "false"));
  B2INFO(" -->  Start of int. wind.:" << m_startSampling);
  B2INFO(" -->  Number of samples:  " << m_nAPV25Samples);
  B2INFO(
    " -->  Random phase sampl.:" << (m_randomPhaseSampling ? "true" : "false"));
  B2INFO(" PROCESSING:");
  B2INFO(" -->  ADC:                " << (m_applyADC ? "true" : "false"));
  B2INFO(" -->  ADC range low (e-): " << m_minADC);
  B2INFO(" -->  ADC range high (e-):" << m_maxADC);
  B2INFO(" -->  ADC bits:           " << m_bitsADC);
  B2INFO(" -->  1 adu (e-)*:        " << m_unitADC);
  B2INFO(" REPORTING: ");
  B2INFO(" -->  statisticsFilename: " << m_rootFilename);
  B2INFO(
    " -->  storeWaveforms:     " << (m_storeWaveforms ? "true" : "false"));

  if (!m_rootFilename.empty()) {
    m_rootFile = new TFile(m_rootFilename.c_str(), "RECREATE");
    m_rootFile->cd();
    m_histDiffusion_v = new TH1D("h_holeDiffusion", "Diffusion distance, v",
                                 200, -100, 100);
    m_histDiffusion_v->GetXaxis()->SetTitle("Diffusion distance v [um]");
    m_histDiffusion_u = new TH1D("h_electronDiffusion",
                                 "Diffusion distance", 200, -100, 100);
    m_histDiffusion_v->GetXaxis()->SetTitle("Diffusion distance u [um]");
    m_histLorentz_u = new TH1D("h_LorentzAngle_u", "Lorentz angle, electrons",
                               100, -0.25, 0.25);
    m_histLorentz_u->GetXaxis()->SetTitle("Lorentz angle");
    m_histLorentz_v = new TH1D("h_LorentzAngle_v",
                               "Lorentz angle, holes", 100, -0.002, 0.002);
    m_histLorentz_v->GetXaxis()->SetTitle("Lorentz angle");
    m_signalDist_u = new TH1D("h_signalDist_u",
                              "Strip signals vs. TrueHits, electrons", 100, -400, 400);
    m_signalDist_u->GetXaxis()->SetTitle(
      "U strip position - TrueHit u [um]");
    m_signalDist_v = new TH1D("h_signalDist_v",
                              "Strip signals vs. TrueHits, holes", 400, -400, 400);
    m_signalDist_v->GetXaxis()->SetTitle(
      "V strip position - TrueHit v [um]");

    if (m_storeWaveforms) {
      m_waveTree = new TTree("waveTree", "SVD waveforms");
      m_waveTree->Branch("sensor", &tree_vxdID, "sensor/I");
      m_waveTree->Branch("u_or_v", &tree_uv, "u_or_v/I");
      m_waveTree->Branch("strip", &tree_strip, "strip/I");
      m_waveTree->Branch("signal", tree_signal, "signal[20]/D");
    }
  } else {
    // No waveforms can be stored if there is no statistics file.
    m_storeWaveforms = false;
  }
  // Check if the global random number generator is available.
  if (!gRandom)
    B2FATAL("gRandom not initialized, please set up gRandom first");
}

void SVDDigitizerModule::beginRun()
{
  //Fill map with all possible sensors This is too slow to be done every event so
  //we fill it once and only clear the content of the sensors per event, not
  //the whole map
  m_sensors.clear();
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (VxdID layer : geo.getLayers(SensorInfo::SVD)) {
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        m_sensors[sensor] = Sensor();
      }
    }
  }
}

void SVDDigitizerModule::event()
{
  //Clear sensors and process SimHits
  for (Sensors::value_type& sensor : m_sensors) {
    sensor.second.first.clear();  // u strips
    sensor.second.second.clear(); // v strips
  }
  m_currentSensor = 0;
  m_currentSensorInfo = 0;

  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<SVDSimHit> storeSimHits(m_storeSimHitsName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);

  RelationArray mcParticlesToSimHits(storeMCParticles, storeSimHits, m_relMCParticleSimHitName);
  RelationArray trueHitsToSimHits(storeTrueHits, storeSimHits, m_relTrueHitSimHitName);

  RelationIndex<MCParticle, SVDSimHit> relMCParticleSimHit(storeMCParticles, storeSimHits, m_relMCParticleSimHitName);
  RelationIndex<SVDTrueHit, SVDSimHit> relTrueHitSimHit(storeTrueHits, storeSimHits, m_relTrueHitSimHitName);

  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);

  RelationArray relDigitMCParticle(storeDigits, storeMCParticles, m_relDigitMCParticleName);

  RelationArray relDigitTrueHit(storeDigits, storeTrueHits, m_relDigitTrueHitName);


  unsigned int nSimHits = storeSimHits.getEntries();
  if (nSimHits == 0)
    return;

  //Check sensor info and set pointers to current sensor
  for (unsigned int i = 0; i < nSimHits; ++i) {
    m_currentHit = storeSimHits[i];
    const RelationIndex<MCParticle, SVDSimHit>::Element* mcRel =
      relMCParticleSimHit.getFirstElementTo(m_currentHit);
    if (mcRel) {
      m_currentParticle = mcRel->indexFrom;
      if (mcRel->weight < 0) {
        //This simhit is from a particle which was not saved by the simulation
        //so we do not take it into account for relations. Otherwise we might
        //end up adding positive and negative weights together
        m_currentParticle = -1;
      }
    } else {
      // Don't bother with warnings for background SimHits
      if (m_currentHit->getBackgroundTag() == SimHitBase::bg_none)
        B2WARNING(
          "Could not find MCParticle which produced SVDSimhit " << i);
      m_currentParticle = -1;
    }
    const RelationIndex<SVDTrueHit, SVDSimHit>::Element* trueRel =
      relTrueHitSimHit.getFirstElementTo(m_currentHit);
    //We only care about true hits from particles which have not been ignored
    if (trueRel && trueRel->weight > 0) {
      m_currentTrueHit = trueRel->indexFrom;
    } else {
      m_currentTrueHit = -1;
    }

    VxdID sensorID = m_currentHit->getSensorID();
    if (!m_currentSensorInfo || sensorID != m_currentSensorInfo->getID()) {
      m_currentSensorInfo =
        dynamic_cast<const SensorInfo*>(&VXD::GeoCache::get(
                                          sensorID));
      if (!m_currentSensorInfo)
        B2FATAL(
          "Sensor Information for Sensor " << sensorID << " not found, make sure that the geometry is set up correctly");

      const SensorInfo& info = *m_currentSensorInfo;
      // Publish some useful data
      m_sensorThickness = info.getThickness();
      m_depletionVoltage = info.getDepletionVoltage();
      m_biasVoltage = info.getBiasVoltage();
      m_backplaneCapacitanceU = info.getBackplaneCapacitanceU();
      m_interstripCapacitanceU = info.getInterstripCapacitanceU();
      m_couplingCapacitanceU = info.getCouplingCapacitanceU();
      m_backplaneCapacitanceV = info.getBackplaneCapacitanceV();
      m_interstripCapacitanceV = info.getInterstripCapacitanceV();
      m_couplingCapacitanceV = info.getCouplingCapacitanceV();
      m_elNoiseU = info.getElectronicNoiseU();
      m_elNoiseV = info.getElectronicNoiseV();

      m_currentSensor = &m_sensors[sensorID];
      B2DEBUG(20,
              "Sensor Parameters for Sensor " << sensorID << ": " << endl
              << " --> Width:          " << m_currentSensorInfo->getWidth() << endl
              << " --> Length:         " << m_currentSensorInfo->getLength() << endl
              << " --> uPitch:         " << m_currentSensorInfo->getUPitch() << endl
              << " --> vPitch:         " << m_currentSensorInfo->getVPitch(-m_currentSensorInfo->getLength() / 2.0)
              << ", " << m_currentSensorInfo->getVPitch(m_currentSensorInfo->getLength() / 2.0) << endl
              << " --> Thickness:      " << m_currentSensorInfo->getThickness() << endl
              << " --> Deplet. voltage:" << m_currentSensorInfo->getDepletionVoltage() << endl
              << " --> Bias voltage:   " << m_currentSensorInfo->getBiasVoltage() << endl
              << " --> Backplane cap.U: " << m_currentSensorInfo->getBackplaneCapacitanceU() << endl
              << " --> Interstrip cap.U:" << m_currentSensorInfo->getInterstripCapacitanceU() << endl
              << " --> Coupling cap.U:  " << m_currentSensorInfo->getCouplingCapacitanceU() << endl
              << " --> Backplane cap.V: " << m_currentSensorInfo->getBackplaneCapacitanceV() << endl
              << " --> Interstrip cap.V:" << m_currentSensorInfo->getInterstripCapacitanceV() << endl
              << " --> Coupling cap.V:  " << m_currentSensorInfo->getCouplingCapacitanceV() << endl
              << " --> El. noise U:    " << m_currentSensorInfo->getElectronicNoiseU() << endl
              << " --> El. noise V:    " << m_currentSensorInfo->getElectronicNoiseV() << endl
             );

    }
    B2DEBUG(10,
            "Processing hit " << i << " in Sensor " << sensorID << ", related to MCParticle " << m_currentParticle);
    processHit();
  }
  // If storage of waveforms is required, store them in the statistics file.
  if (m_storeWaveforms) {
    m_rootFile->cd();
    saveWaveforms();
  }

  saveDigits();
}

void SVDDigitizerModule::processHit()
{
  if (m_applyWindow) {
    //Ignore hits which are outside of the SVD active time frame.
    //Here we can only discard hits that arrived after the window was closed;
    //we will later start sampling only after the window opened.
    double stopSampling = m_startSampling + m_nAPV25Samples * m_samplingTime;
    B2DEBUG(30,
            "Checking if hit is in timeframe " << m_currentHit->getGlobalTime() << " <= " << stopSampling);

    if (m_currentHit->getGlobalTime() > stopSampling)
      return;
  }
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  // Set time of the event
  m_currentTime = m_currentHit->getGlobalTime();

  //Get Steplength and direction
  const TVector3& startPoint = m_currentHit->getPosIn();
  const TVector3& stopPoint = m_currentHit->getPosOut();
  TVector3 direction = stopPoint - startPoint;
  double trackLength = direction.Mag();

  if (m_currentHit->getPDGcode() == 22 || trackLength < 0.1 * Unit::um) {
    //Photons deposit the energy at the end of their step
    driftCharge(stopPoint, m_currentHit->getElectrons(), SVD::SensorInfo::electron);
    driftCharge(stopPoint, m_currentHit->getElectrons(), SVD::SensorInfo::hole);
  } else {
    //Otherwise, split into segments of (default) max. 5µm and
    //drift the charges from the center of each segment
    auto segments = m_currentHit->getElectronsConstantDistance(m_segmentLength);
    double lastFraction {0};
    double lastElectrons {0};

    for (auto& segment : segments) {
      //Simhit returns step fraction and cumulative electrons. We want the
      //center of these steps and electrons in this step
      const double f = (segment.first + lastFraction) / 2;
      const double e = segment.second - lastElectrons;
      //Update last values
      std::tie(lastFraction, lastElectrons) = segment;

      //And drift charge from that position
      const TVector3 position = startPoint + f * direction;
      driftCharge(position, e, SVD::SensorInfo::electron);
      driftCharge(position, e, SVD::SensorInfo::hole);
    }
  }
}


void SVDDigitizerModule::driftCharge(const TVector3& position, double carriers, SVD::SensorInfo::CarrierType carrierType)
{
  bool have_electrons = (carrierType == SVD::SensorInfo::electron);

  string carrierName = (have_electrons) ? "electron" : "hole";
  B2DEBUG(30,
          "Drifting " << carriers << " " << carrierName << "s at position (" << position.x() << ", " << position.y() << ", " << position.z()
          << ").");

  //Get references to current sensor/info for ease of use
  const SensorInfo& info = *m_currentSensorInfo;
  StripSignals& digits = (have_electrons) ? m_currentSensor->first : m_currentSensor->second;

  double distanceToPlane = (have_electrons) ?
                           0.5 * m_sensorThickness - position.Z() :
                           -0.5 * m_sensorThickness - position.Z();

  // Approximation: calculate drift velocity at the point halfway towards
  // the respective sensor surface.
  TVector3 mean_pos(position.X(), position.Y(), position.Z() + 0.5 * distanceToPlane);

  // Calculate drift times and widths of charge clouds.
  TVector3 v = info.getVelocity(carrierType, mean_pos);
  double driftTime = distanceToPlane / v.Z();
  TVector3 center = position + driftTime * v;
  double mobility = (have_electrons) ?
                    info.getElectronMobility(info.getEField(mean_pos).Mag()) :
                    info.getHoleMobility(info.getEField(mean_pos).Mag());
  double D = Const::kBoltzmann * info.getTemperature() / Unit::e * mobility;
  double sigma = std::max(1.0e-4, sqrt(2.0 * D * driftTime));
  double tanLorentz = (have_electrons) ? v.X() / v.Z() : v.Y() / v.Z();
  sigma *= sqrt(1.0 + tanLorentz * tanLorentz);
  if (m_histLorentz_u && have_electrons) m_histLorentz_u->Fill(tanLorentz);
  if (m_histLorentz_v && !have_electrons) m_histLorentz_v->Fill(tanLorentz);

  //Distribute carrier cloud on strips
  int vID = info.getVCellID(center.Y(), true);
  int uID = info.getUCellID(center.X(), center.Y(), true);
  int seedStrip = (have_electrons) ? uID : vID;
  double seedPos = (have_electrons) ?
                   info.getUCellPosition(seedStrip, vID) :
                   info.getVCellPosition(seedStrip);
  double geomPitch = (have_electrons) ? 0.5 * info.getUPitch(center.Y()) : 0.5 * info.getVPitch();
  int nCells = (have_electrons) ? info.getUCells() : info.getVCells();
  std::deque<double> stripCharges;
  std::deque<double> strips; // intermediate strips will be half-integers, like 2.5.
#define NORMAL_CDF(z) 0.5 * std::erfc( - (z) * 0.707107)
  double current_pos = (have_electrons) ? seedPos - center.X() : seedPos - center.Y();
  double current_strip = seedStrip;
  double cdf_low = NORMAL_CDF((current_pos - 0.5 * geomPitch) / sigma);
  double cdf_high = NORMAL_CDF((current_pos + 0.5 * geomPitch) / sigma);
  double charge = carriers * (cdf_high - cdf_low);
  stripCharges.push_back(charge);
  strips.push_back(current_strip);
  while (cdf_low > 1.0e-5) {
    current_pos -= geomPitch;
    current_strip -= 0.5;
    double cdf_current = NORMAL_CDF((current_pos - 0.5 * geomPitch) / sigma);
    charge = carriers * (cdf_low - cdf_current);
    stripCharges.push_front(charge);
    strips.push_front(current_strip);
    cdf_low = cdf_current;
  }
  current_pos = (have_electrons) ? seedPos - center.X() : seedPos - center.Y();
  current_strip = seedStrip;
  while (cdf_high < 1.0 - 1.0e-5) {
    current_pos += geomPitch;
    current_strip += 0.5;
    double cdf_current = NORMAL_CDF((current_pos + 0.5 * geomPitch) / sigma);
    charge = carriers * (cdf_current - cdf_high);
    stripCharges.push_back(charge);
    strips.push_back(current_strip);
    cdf_high = cdf_current;
  }
#undef NORMAL_CDF
  // Pad with zeros for smoothing
  int npads = (strips.front() - floor(strips.front()) == 0) ? 4 : 3;
  for (int i = 0; i < npads; ++i) {
    strips.push_front(strips.front() - 0.5);
    stripCharges.push_front(0);
  }
  npads = (strips.back() - floor(strips.back()) == 0) ? 4 : 3;
  for (int i = 0; i < npads; ++i) {
    strips.push_back(strips.back() + 0.5);
    stripCharges.push_back(0);
  }
  // Cross-talk
  // FIXME: Removed crosstalk between readout strips to have clear
  // charge loss on floating strips.
  double Cc = (have_electrons) ? info.getCouplingCapacitanceU() :
              info.getCouplingCapacitanceV();
  double Ci = (have_electrons) ? info.getInterstripCapacitanceU() :
              info.getInterstripCapacitanceV();
  double Cb = (have_electrons) ? info.getBackplaneCapacitanceU() :
              info.getBackplaneCapacitanceV();

  double cNeighbour2 = 0.0; // 0.5*Ci/(Ci+Cb);
  double cNeighbour1 = Ci / (2 * Ci + Cb);
  double cSelf = 3 * Cc / (3 * Cc + Ci + 3 * Cb);

  std::deque<double> readoutCharges;
  std::deque<int> readoutStrips;
  for (std::size_t index = 2; index <  strips.size() - 2; index += 2) {
    readoutCharges.push_back(cSelf * (
                               cNeighbour2 * stripCharges[index - 2]
                               + cNeighbour1 * stripCharges[index - 1]
                               + stripCharges[index]
                               + cNeighbour1 * stripCharges[index + 1]
                               + cNeighbour2 * stripCharges[index + 2]
                             ));
    readoutStrips.push_back(static_cast<int>(strips[index]));
  }
  // Trim at sensor edges
  double tail = 0;
  while (readoutStrips.size() > 0 && readoutStrips.front() < 0) {
    readoutStrips.pop_front();
    tail += readoutCharges.front();
    readoutCharges.pop_front();
  }
  readoutCharges.front() += tail;
  tail = 0;
  while (readoutStrips.size() > 0 && readoutStrips.back() > nCells - 1) {
    readoutStrips.pop_back();
    tail += readoutCharges.back();
    readoutCharges.pop_back();
  }
  readoutCharges.back() += tail;
  // Poisson smearing - Gaussian approximation
  if (m_applyPoisson)
    for (auto& c : readoutCharges)
      c = (c <= 0) ? 0 : std::max(0.0, gRandom->Gaus(c, std::sqrt(info.c_fanoFactorSi * c)));
  // Fill diagnostic charts
  if (m_histDiffusion_u && m_histDiffusion_v) {
    TH1D* histo = (have_electrons) ? m_histDiffusion_u : m_histDiffusion_v;
    double d = (have_electrons) ? seedPos - center.X() : seedPos - center.Y();
    for (std::size_t index = 0; index < readoutStrips.size(); ++ index) {
      double dist = d + (readoutStrips[index] - seedStrip) * 2 * geomPitch;
      histo->Fill(dist / Unit::um, readoutCharges[index]);
    }
  }
  // Store
  double recoveredCharge = 0;
  for (std::size_t index = 0; index <  readoutStrips.size(); index ++) {
    digits[readoutStrips[index]].add(m_currentTime + driftTime, readoutCharges[index],
                                     m_shapingTime, m_currentParticle, m_currentTrueHit);
    recoveredCharge += readoutCharges[index];
    B2DEBUG(30, "strip: " << readoutStrips[index] << " charge: " << readoutCharges[index]);
  }
  B2DEBUG(30, "Digitized " << recoveredCharge << " of " << carriers << " original carriers.");
}

double SVDDigitizerModule::addNoise(double charge, double noise)
{
  if (charge < 0) {
    //Noise digit, add noise to exceed Noise Threshold;
    double p = gRandom->Uniform(m_noiseFraction, 1.0);
    charge = TMath::NormQuantile(p) * noise;
  } else {
    if (m_applyNoise) {
      charge += gRandom->Gaus(0., noise);
    }
  }
  return charge;
}

void SVDDigitizerModule::saveDigits()
{
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  RelationArray relDigitMCParticle(storeDigits, storeMCParticles,
                                   m_relDigitMCParticleName);
  RelationArray relDigitTrueHit(storeDigits, storeTrueHits,
                                m_relDigitTrueHitName);


  //Set time of the first sample
  double initTime = m_startSampling;
  if (m_randomPhaseSampling) {
    initTime = gRandom->Uniform(0.0, m_samplingTime);
  }

  // Take samples at the desired times, add noise, zero-suppress and save digits.

  for (Sensors::value_type& sensor : m_sensors) {
    int sensorID = sensor.first;
    const SensorInfo& info =
      dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));
    // u-side digits:
    double elNoiseU = info.getElectronicNoiseU();
    double charge_thresholdU = m_SNAdjacent * elNoiseU;
    // Add noisy digits
    if (m_applyNoise) {
      double fraction = 1.0 - m_noiseFraction;
      int nU = info.getUCells();
      int uNoisyStrips = gRandom->Poisson(fraction * nU);
      for (short ns = 0; ns < uNoisyStrips; ++ns) {
        short iStrip = gRandom->Integer(nU);
        sensor.second.first[iStrip].add(0, -1, 0, 0, 0);
      } // for ns
    } // Add noise digits
    // Cycle through signals and generate samples
    for (StripSignals::value_type& stripSignal : sensor.second.first) {
      short int iStrip = stripSignal.first;
      SVDSignal& s = stripSignal.second;
      // Now generate samples in time and save as digits.
      vector<double> samples;
      // For noise digits, just generate random variates on randomly selected samples
      if (s.isNoise()) {
        double pSelect = 1.0 / m_nAPV25Samples;
        for (int iSample = 0; iSample < m_nAPV25Samples; iSample++) {
          if (gRandom->Uniform() < pSelect)
            samples.push_back(addNoise(-1, elNoiseU));
          else
            samples.push_back(gRandom->Gaus(0, elNoiseU));
        }
      } else {
        double t = initTime;
        for (int iSample = 0; iSample < m_nAPV25Samples; iSample ++) {
          samples.push_back(addNoise(s(t), elNoiseU));
          t += m_samplingTime;
        }
      }
      // Check that at least three consecutive samples are over threshold
      auto it = search_n(
                  samples.begin(), samples.end(), 3, charge_thresholdU,
      [](double x, double y) { return x > y; }
                );
      if (it == samples.end()) continue;
      // Save samples and relations
      SVDSignal::relations_map particles = s.getMCParticleRelations();
      SVDSignal::relations_map truehits = s.getTrueHitRelations();
      for (int iSample = 0; iSample < m_nAPV25Samples; iSample++) {
        double sampleCharge = samples.at(iSample);
        //Limit signal to ADC steps
        if (m_applyADC)
          sampleCharge = eToADU(sampleCharge);
        // Save as a new digit
        int digIndex = storeDigits.getEntries();
        storeDigits.appendNew(
          SVDDigit(sensorID, true, iStrip,
                   info.getUCellPosition(iStrip), sampleCharge, iSample));
        //If the digit has any relations to MCParticles, add the Relation
        if (particles.size() > 0) {
          relDigitMCParticle.add(digIndex, particles.begin(), particles.end());
        }
        //If the digit has any relations to truehits, add the Relations.
        if (truehits.size() > 0) {
          relDigitTrueHit.add(digIndex, truehits.begin(), truehits.end());
          // Add reporting data
          if (m_signalDist_u) {
            for (SVDSignal::relation_value_type trueRel : truehits) {
              int iTrueHit = trueRel.first;
              float trueWeight = trueRel.second;
              if (iTrueHit > -1) {
                m_signalDist_u->Fill(
                  (info.getUCellPosition(iStrip)
                   - storeTrueHits[iTrueHit]->getU())
                  / Unit::um, trueWeight);
              } // if iTrieHit
            } // for trueRel
          } // if m_signalDist_u
        } // if truehits.size()
      } // for iSample
    } // for stripSignals
    // v-side digits:
    double elNoiseV = info.getElectronicNoiseV();
    double charge_thresholdV = m_SNAdjacent * elNoiseV;
    // Add noisy digits
    if (m_applyNoise) {
      double fraction = 1.0 - m_noiseFraction;
      int nV = info.getVCells();
      int vNoisyStrips = gRandom->Poisson(fraction * nV);
      for (short ns = 0; ns < vNoisyStrips; ++ns) {
        short iStrip = gRandom->Integer(nV);
        sensor.second.second[iStrip].add(0, -1, 0, 0, 0);
      } // for ns
    } // Add noise digits
    // Cycle through signals and generate samples
    for (StripSignals::value_type& stripSignal : sensor.second.second) {
      short int iStrip = stripSignal.first;
      SVDSignal& s = stripSignal.second;
      // Now generate samples in time and save as digits.
      vector<double> samples;
      // For noise digits, just generate random variates on randomly selected samples
      if (s.isNoise()) {
        double pSelect = 1.0 / m_nAPV25Samples;
        for (int iSample = 0; iSample < m_nAPV25Samples; iSample++) {
          if (gRandom->Uniform() < pSelect)
            samples.push_back(addNoise(-1, elNoiseV));
          else
            samples.push_back(gRandom->Gaus(0, elNoiseV));
        }
      } else {
        double t = initTime;
        for (int iSample = 0; iSample < m_nAPV25Samples; iSample ++) {
          samples.push_back(addNoise(s(t), elNoiseV));
          t += m_samplingTime;
        }
      }
      // Check that at least three samples are over threshold
      auto it = search_n(
                  samples.begin(), samples.end(), 3, charge_thresholdV,
      [](double x, double y) { return x > y; }
                );
      if (it == samples.end()) continue;
      // Save samples and relations
      SVDSignal::relations_map particles = s.getMCParticleRelations();
      SVDSignal::relations_map truehits = s.getTrueHitRelations();
      for (int iSample = 0; iSample < m_nAPV25Samples; iSample++) {
        double sampleCharge = samples.at(iSample);
        //Limit signal to ADC steps
        if (m_applyADC)
          sampleCharge = eToADU(sampleCharge);
        // Save as a new digit
        int digIndex = storeDigits.getEntries();
        storeDigits.appendNew(
          SVDDigit(sensorID, false, iStrip,
                   info.getVCellPosition(iStrip), sampleCharge, iSample));
        //If the digit has any relations to MCParticles, add the Relation
        if (particles.size() > 0) {
          relDigitMCParticle.add(digIndex, particles.begin(), particles.end());
        }
        //If the digit has any relations to truehits, add the Relations.
        if (truehits.size() > 0) {
          relDigitTrueHit.add(digIndex, truehits.begin(), truehits.end());
          // Add reporting data
          if (m_signalDist_v) {
            for (SVDSignal::relation_value_type trueRel : truehits) {
              int iTrueHit = trueRel.first;
              float trueWeight = trueRel.second;
              if (iTrueHit > -1) {
                m_signalDist_v->Fill(
                  (info.getVCellPosition(iStrip)
                   - storeTrueHits[iTrueHit]->getV())
                  / Unit::um, trueWeight);
              } // if iTrieHit
            } // for trueRel
          } // if m_signalDist_v
        } // if truehits.size()
      } // for iSample
    } // for stripSignals
  } // FOREACH sensor
}

void SVDDigitizerModule::saveWaveforms()
{
  for (Sensors::value_type& sensor : m_sensors) {
    tree_vxdID = sensor.first;
    const SensorInfo& info =
      dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensor.first));
    // u-side digits:
    tree_uv = 1;
    double thresholdU = 3.0 * info.getElectronicNoiseU();
    for (StripSignals::value_type& stripSignal : sensor.second.first) {
      tree_strip = stripSignal.first;
      SVDSignal& s = stripSignal.second;
      // Read the value only if the signal is large enough.
      if (s.getCharge() < thresholdU)
        continue;
      for (int iTime = 0; iTime < 20; ++iTime) {
        tree_signal[iTime] = s(10 * iTime);
      }
      m_waveTree->Fill();
    } // FOREACH stripSignal
    // v-side digits:
    tree_uv = 0;
    double thresholdV = 3.0 * info.getElectronicNoiseV();
    for (StripSignals::value_type& stripSignal : sensor.second.second) {
      tree_strip = stripSignal.first;
      SVDSignal& s = stripSignal.second;
      // Read the values only if the signal is large enough
      if (s.getCharge() < thresholdV)
        continue;
      for (int iTime = 0; iTime < 20; ++iTime) {
        tree_signal[iTime] = s(10. * iTime);
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

