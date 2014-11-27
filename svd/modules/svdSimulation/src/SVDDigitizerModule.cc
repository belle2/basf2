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
#include <vector>
#include <set>
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
  addParam("ElectronicNoise", m_elNoise,
           "Electronic noise on strip signals in e-", double(2000.0));
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
           "Start of the sampling window, in ns", double(0.0));
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
  m_elNoise *= Unit::e;
  m_minADC = m_minADC * Unit::e;
  m_maxADC = m_maxADC * Unit::e;
  m_unitADC = (m_maxADC - m_minADC) / (pow(2.0, m_bitsADC) - 1);
  m_samplingTime *= Unit::ns;
  m_shapingTime *= Unit::ns;

  B2INFO(
    "SVDDigitizer parameters (in default system units, *=cannot be set directly):");
  B2INFO(" DATASTORE COLLECTIONS:")
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
  B2INFO(" -->  Gaussian noise:     " << m_elNoise);
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
                              "Strip signals vs. TrueHits, holes", 100, -400, 400);
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
  for (Sensors::value_type & sensor : m_sensors) {
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
    } else {
      // Don't bother with warnings for background SimHits
      if (m_currentHit->getBackgroundTag() == SimHitBase::bg_none)
        B2WARNING(
          "Could not find MCParticle which produced SVDSimhit " << i);
      m_currentParticle = -1;
    }
    const RelationIndex<SVDTrueHit, SVDSimHit>::Element* trueRel =
      relTrueHitSimHit.getFirstElementTo(m_currentHit);
    if (trueRel) {
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
      m_backplaneCapacitance = info.getBackplaneCapacitance();
      m_interstripCapacitance = info.getInterstripCapacitance();
      m_couplingCapacitance = info.getCouplingCapacitance();

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
              << " --> Backplane cap.: " << m_currentSensorInfo->getBackplaneCapacitance() << endl
              << " --> Interstrip cap.:" << m_currentSensorInfo->getInterstripCapacitance() << endl
              << " --> Coupling cap.:  " << m_currentSensorInfo->getCouplingCapacitance() << endl);
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
  // Set time of the event
  m_currentTime = m_currentHit->getGlobalTime();

  //Get Steplength and direction
  const TVector3& startPoint = m_currentHit->getPosIn();
  const TVector3& stopPoint = m_currentHit->getPosOut();
  TVector3 direction = stopPoint - startPoint;
  double trackLength = direction.Mag();

  if (m_currentHit->getPDGcode() == 22 || trackLength < 0.1 * Unit::um) {
    //Photons deposit the energy at the end of their step
    driftCharge(stopPoint, m_currentHit->getElectrons());
  } else {
    //Otherwise, split into segments of (default) max. 5µm and
    //drift the charges from the center of each segment
    auto segments = m_currentHit->getElectronsConstantDistance(m_segmentLength);
    double lastFraction {0};
    double lastElectrons {0};

    for (auto & segment : segments) {
      //Simhit returns step fraction and cumulative electrons. We want the
      //center of these steps and electrons in this step
      const double f = (segment.first + lastFraction) / 2;
      const double e = segment.second - lastElectrons;
      //Update last values
      std::tie(lastFraction, lastElectrons) = segment;

      //And drift charge from that position
      const TVector3 position = startPoint + f * direction;
      driftCharge(position, e);
    }
  }
}


void SVDDigitizerModule::driftCharge(const TVector3& position,
                                     double carriers)
{
  B2DEBUG(30,
          "Drifting " << carriers << " carriers at position (" << position.x() << ", " << position.y() << ", " << position.z() << ").");

  //Get references to current sensor/info for ease of use
  const SensorInfo& info = *m_currentSensorInfo;
  Sensor& sensor = *m_currentSensor;

  double distanceToFrontPlane = 0.5 * m_sensorThickness - position.Z();
  double distanceToBackPlane = 0.5 * m_sensorThickness + position.Z();

  // Approximation: calculate drift velocity at the point halfway towards
  // the respective sensor surface.
  TVector3 mean_e(position.X(), position.Y(), position.Z() + 0.5 * distanceToFrontPlane);
  TVector3 mean_h(position.X(), position.Y(), position.Z() - 0.5 * distanceToBackPlane);

  // Calculate drift times and widths of charge clouds.
  // Electrons:
  TVector3 v_e = info.getVelocity(info.electron, mean_e);
  double driftTime_e = distanceToFrontPlane / v_e.Z();
  TVector3 center_e = position + driftTime_e * v_e;
  double D_e = Const::kBoltzmann * info.getTemperature() / Unit::e
               * info.getElectronMobility(info.getEField(mean_e).Mag());
  double sigma_e = sqrt(2.0 * D_e * driftTime_e);
  double tanLorentz_e = v_e.X() / v_e.Z();
  sigma_e *= sqrt(1.0 + tanLorentz_e * tanLorentz_e);
  if (m_histLorentz_u)
    m_histLorentz_u->Fill(tanLorentz_e);

  // Holes
  TVector3 v_h = info.getVelocity(info.hole, mean_h);
  double driftTime_h = -distanceToBackPlane / v_h.Z();
  TVector3 center_h = position + driftTime_h * v_h;
  double D_h = Const::kBoltzmann * info.getTemperature() / Unit::e
               * info.getElectronMobility(info.getEField(mean_h).Mag());
  double sigma_h = sqrt(2.0 * D_h * driftTime_h);
  double tanLorentz_h = v_h.Y() / v_h.Z();
  sigma_h *= sqrt(1.0 + tanLorentz_h * tanLorentz_h);
  if (m_histLorentz_v)
    m_histLorentz_v->Fill(tanLorentz_h);

  //Determine strips hit by the hole cloud
  double vLow = center_h.Y() - m_widthOfDiffusCloud * sigma_h;
  double vHigh = center_h.Y() + m_widthOfDiffusCloud * sigma_h;
  int vIDLow = info.getVCellID(vLow, true);
  int vIDHigh = info.getVCellID(vHigh, true);
  B2DEBUG(30, "Size of diffusion cloud (h): " << sigma_h);
  B2DEBUG(30, "vID from " << vIDLow << " to " << vIDHigh);

  //Determine strips hit by the electron cloud
  double uLow = center_e.X() - m_widthOfDiffusCloud * sigma_e;
  double uHigh = center_e.X() + m_widthOfDiffusCloud * sigma_e;
  int uIDLow = info.getUCellID(uLow, center_e.Y(), true);
  int uIDHigh = info.getUCellID(uHigh, center_e.Y(), true);
  B2DEBUG(30, "Size of diffusion cloud (e): " << sigma_e);
  B2DEBUG(30, "uID from " << uIDLow << " to " << uIDHigh);

  //Now loop over strips and calculate the integral of the gaussian charge distributions.
  //Deposit the charge corresponding to the per strip integral in each strip.
  //This is currently the only place where intermediate strips come into play:

#define NORMAL_CDF(mean,sigma,x) TMath::Freq(((x)-(mean))/(sigma))

  // Holes
  // Add one more readout strip on each end of the region to make place for charges
  // induced by capacitive coupling.
  if (vIDLow > 0)
    vIDLow--;
  if (vIDHigh < info.getVCells() - 1)
    vIDHigh++;
  double vGeomPitch = 0.5 * info.getVPitch();
  // We have to store the strip charges to calculate cross-talk.
  double fraction = 0;
  int nStrips_h = 2 * (vIDHigh - vIDLow) + 1;
  std::vector<double> hStripCharges(nStrips_h);
  double vPos = info.getVCellPosition(vIDLow);
  double vLowerTail =
    NORMAL_CDF(center_h.Y(), sigma_h, vPos - 0.5 * vGeomPitch);
  for (int vID = 0; vID < nStrips_h; ++vID) {
    double vUpperTail =
      NORMAL_CDF(center_h.Y(), sigma_h, vPos + 0.5 * vGeomPitch);
    double vIntegral = vUpperTail - vLowerTail;
    // Can fail on far tails
    if (TMath::IsNaN(vIntegral)) vIntegral = 0.0;
    vLowerTail = vUpperTail;
    double charge = carriers * vIntegral;
    if (m_applyPoisson) {
      //Actually, Poisson takes really long to calculate for large values.
      //We use a (truncated) gaussian instead.
      charge = gRandom->Gaus(charge, sqrt(info.c_fanoFactorSi * charge));
      if (charge < 0.0) charge = 0.0;
    }
    hStripCharges[vID] = charge;
    B2DEBUG(80, "Relative charge for strip (" << vIDLow + 0.5 * vID << "): " << vIntegral);
    fraction += vIntegral;
    if (m_histDiffusion_v && charge >= 1.0)
      m_histDiffusion_v->Fill((vPos - center_h.Y()) / Unit::um, charge);
    vPos += vGeomPitch;
  }
  B2DEBUG(30, "Fraction of charge (e): " << fraction);
  // The strip signals combine due to capacitive coupling.
  // FIXME: We have the same parameters for n and p strips. That can't be true,
  // letting alone wedge sensors.
  // Moreover, is it wise to combine things for each charglet?
  double correctedCharge = 0;
  double cNeighbour2 = 0.5 * info.getInterstripCapacitance()
                       / (0.5 * info.getInterstripCapacitance()
                          + info.getBackplaneCapacitance()
                          + info.getCouplingCapacitance());
  double cNeighbour1 = 0.5;
  double cSelf = 1.0 - 2.0 * cNeighbour2;
  //Leftmost strip (there must be at least one readout strip to the right):
  int arrayIndex = 0;
  if (vIDLow == 0) {
    correctedCharge = (cNeighbour2 + cSelf) * hStripCharges[arrayIndex]
                      + cNeighbour1 * hStripCharges[arrayIndex + 1]
                      + cNeighbour2 * hStripCharges[arrayIndex + 2];
  } else {
    correctedCharge = cSelf * hStripCharges[arrayIndex]
                      + cNeighbour1 * hStripCharges[arrayIndex + 1]
                      + cNeighbour2 * hStripCharges[arrayIndex + 2];
  }
  sensor.second[vIDLow].add(m_currentTime + driftTime_h, correctedCharge,
                            m_shapingTime, m_currentParticle, m_currentTrueHit);

  for (int vID = vIDLow + 1; vID < vIDHigh; ++vID) {
    arrayIndex = 2 * (vID - vIDLow);
    correctedCharge = cNeighbour2 * hStripCharges[arrayIndex - 2]
                      + cNeighbour1 * hStripCharges[arrayIndex - 1]
                      + cSelf * hStripCharges[arrayIndex]
                      + cNeighbour1 * hStripCharges[arrayIndex + 1]
                      + cNeighbour2 * hStripCharges[arrayIndex + 2];
    sensor.second[vID].add(m_currentTime + driftTime_h, correctedCharge,
                           m_shapingTime, m_currentParticle, m_currentTrueHit);
  }
  // Rightmost strip (there must be at least one readout strip to the left):
  arrayIndex = 2 * (vIDHigh - vIDLow);
  if (vIDHigh == info.getVCells()) {
    correctedCharge = cNeighbour2 * hStripCharges[arrayIndex - 2]
                      + cNeighbour2 * hStripCharges[arrayIndex - 1]
                      + (cSelf + cNeighbour2) * hStripCharges[arrayIndex];
  } else {
    correctedCharge = cNeighbour2 * hStripCharges[arrayIndex - 2]
                      + cNeighbour2 * hStripCharges[arrayIndex - 1]
                      + cNeighbour2 * hStripCharges[arrayIndex];
  }
  sensor.second[vIDHigh].add(m_currentTime + driftTime_h, correctedCharge,
                             m_shapingTime, m_currentParticle, m_currentTrueHit);

  //Electrons
  // Add one more readout strip on each end of the region to make place for charges
  // induced by capacitive coupling.
  if (uIDLow > 0)
    uIDLow--;
  if (uIDHigh < info.getUCells() - 1)
    uIDHigh++;
  double uGeomPitch = 0.5 * info.getUPitch(center_e.Y());
  // We have to store the strip charges to later calculate cross-talk.
  fraction = 0;
  int nStrips_e = 2 * (uIDHigh - uIDLow) + 1;
  std::vector<double> eStripCharges(nStrips_e);
  double uPos = info.getUCellPosition(uIDLow, info.getVCellID(center_e.Y()));
  double uLowerTail =
    NORMAL_CDF(center_e.X(), sigma_e, uPos - 0.5 * uGeomPitch);
  for (int uID = 0; uID < nStrips_e; ++uID) {
    double uUpperTail =
      NORMAL_CDF(center_e.X(), sigma_e, uPos + 0.5 * uGeomPitch);
    double uIntegral = uUpperTail - uLowerTail;
    // Can fail on far tails
    if (TMath::IsNaN(uIntegral)) uIntegral = 0.0;
    uLowerTail = uUpperTail;
    double charge = carriers * uIntegral;
    if (m_applyPoisson) {
      //Actually, Poisson takes really long to calculate for large values.
      //We use a (truncated) gaussian instead.
      charge = gRandom->Gaus(charge, sqrt(info.c_fanoFactorSi * charge));
      if (charge < 0.0) charge = 0.0;
    }
    eStripCharges[uID] = charge;
    B2DEBUG(80, "Relative charge for strip (" << uIDLow + 0.5 * uID << "): " << uIntegral);
    fraction += uIntegral;
    if (m_histDiffusion_u && charge >= 1.0)
      m_histDiffusion_u->Fill((uPos - center_e.X()) / Unit::um, charge);
    uPos += uGeomPitch;
  }
  B2DEBUG(30, "Fraction of charge (h): " << fraction);
  // The strip signals combine due to capacitive coupling.
  //Leftmost strip (there must be at least one readout strip to the right):
  arrayIndex = 0;
  if (uIDLow == 0) {
    correctedCharge = cSelf * eStripCharges[arrayIndex]
                      + cNeighbour1 * eStripCharges[arrayIndex + 1]
                      + cNeighbour2 * eStripCharges[arrayIndex + 2];
  } else {
    correctedCharge = (cNeighbour2 + cSelf) * eStripCharges[arrayIndex]
                      + cNeighbour1 * eStripCharges[arrayIndex + 1]
                      + cNeighbour2 * eStripCharges[arrayIndex + 2];
  }
  sensor.first[uIDLow].add(m_currentTime + driftTime_e, correctedCharge,
                           m_shapingTime, m_currentParticle, m_currentTrueHit);
  for (int uID = uIDLow + 1; uID < uIDHigh; ++uID) {
    arrayIndex = 2 * (uID - uIDLow);
    correctedCharge = cNeighbour2 * eStripCharges[arrayIndex - 2]
                      + cNeighbour1 * eStripCharges[arrayIndex - 1]
                      + cSelf * eStripCharges[arrayIndex]
                      + cNeighbour1 * eStripCharges[arrayIndex + 1]
                      + cNeighbour2 * eStripCharges[arrayIndex + 2];
    sensor.first[uID].add(m_currentTime + driftTime_e, correctedCharge,
                          m_shapingTime, m_currentParticle, m_currentTrueHit);
  }
  // Rightmost strip (there must be at least one readout strip to the left):
  arrayIndex = 2 * (uIDHigh - uIDLow);
  if (uIDHigh == info.getUCells()) {
    correctedCharge = cNeighbour2 * eStripCharges[arrayIndex - 2]
                      + cNeighbour2 * eStripCharges[arrayIndex - 1]
                      + (cSelf + cNeighbour2) * eStripCharges[arrayIndex];
  } else {
    correctedCharge = cNeighbour2 * eStripCharges[arrayIndex - 2]
                      + cNeighbour2 * eStripCharges[arrayIndex - 1]
                      + cSelf * eStripCharges[arrayIndex];
  }
  sensor.first[uIDHigh].add(m_currentTime + driftTime_e, correctedCharge,
                            m_shapingTime, m_currentParticle, m_currentTrueHit);

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

void SVDDigitizerModule::saveDigits()
{
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  RelationArray relDigitMCParticle(storeDigits, storeMCParticles,
                                   m_relDigitMCParticleName);
  RelationArray relDigitTrueHit(storeDigits, storeTrueHits,
                                m_relDigitTrueHitName);

  //Zero suppression cut in electrons
  double charge_threshold = m_SNAdjacent * m_elNoise;

  //Set time of the first sample
  double initTime = m_startSampling;
  if (m_randomPhaseSampling) {
    initTime = gRandom->Uniform(0.0, m_samplingTime);
  }

  // Take samples at the desired times, add noise, zero-suppress and save digits.

  for (Sensors::value_type & sensor : m_sensors) {
    int sensorID = sensor.first;
    const SensorInfo& info =
      dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));
    // u-side digits:
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
    for (StripSignals::value_type & stripSignal : sensor.second.first) {
      short int iStrip = stripSignal.first;
      SVDSignal& s = stripSignal.second;
      // Now generate samples in time and save as digits.
      vector<double> samples;
      // For noise digits, just generate random variates on randomly selected samples
      if (s.isNoise()) {
        double pSelect = 1.0 / m_nAPV25Samples;
        for (int iSample = 0; iSample < m_nAPV25Samples; iSample++) {
          if (gRandom->Uniform() < pSelect)
            samples.push_back(addNoise(-1));
          else
            samples.push_back(gRandom->Gaus(0, m_elNoise));
        }
      } else {
        double t = initTime;
        for (int iSample = 0; iSample < m_nAPV25Samples; iSample ++) {
          samples.push_back(addNoise(s(t)));
          t += m_samplingTime;
        }
      }
      // Check that at least one sample is over threshold
      bool isOverThreshold = false;
      for (double value : samples)
        isOverThreshold = isOverThreshold || (value > charge_threshold);
      if (!isOverThreshold) continue;
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
    for (StripSignals::value_type & stripSignal : sensor.second.second) {
      short int iStrip = stripSignal.first;
      SVDSignal& s = stripSignal.second;
      // Now generate samples in time and save as digits.
      vector<double> samples;
      // For noise digits, just generate random variates on randomly selected samples
      if (s.isNoise()) {
        double pSelect = 1.0 / m_nAPV25Samples;
        for (int iSample = 0; iSample < m_nAPV25Samples; iSample++) {
          if (gRandom->Uniform() < pSelect)
            samples.push_back(addNoise(-1));
          else
            samples.push_back(gRandom->Gaus(0, m_elNoise));
        }
      } else {
        double t = initTime;
        for (int iSample = 0; iSample < m_nAPV25Samples; iSample ++) {
          samples.push_back(addNoise(s(t)));
          t += m_samplingTime;
        }
      }
      // Check that at least one sample is over threshold
      bool isOverThreshold = false;
      for (double value : samples)
        isOverThreshold = isOverThreshold || (value > charge_threshold);
      if (!isOverThreshold) continue;
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
  //Only store large enough signals
  double charge_threshold = 3. * m_elNoise;

  for (Sensors::value_type & sensor : m_sensors) {
    tree_vxdID = sensor.first;
    // u-side digits:
    tree_uv = 1;
    for (StripSignals::value_type & stripSignal : sensor.second.first) {
      tree_strip = stripSignal.first;
      SVDSignal& s = stripSignal.second;
      // Read the value only if the signal is large enough.
      if (s.getCharge() < charge_threshold)
        continue;
      for (int iTime = 0; iTime < 20; ++iTime) {
        tree_signal[iTime] = s(10 * iTime);
      }
      m_waveTree->Fill();
    } // FOREACH stripSignal
    // v-side digits:
    tree_uv = 0;
    for (StripSignals::value_type & stripSignal : sensor.second.second) {
      tree_strip = stripSignal.first;
      SVDSignal& s = stripSignal.second;
      // Read the values only if the signal is large enough
      if (s.getCharge() < charge_threshold)
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

