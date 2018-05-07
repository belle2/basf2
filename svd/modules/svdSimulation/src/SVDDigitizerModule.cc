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

#include <framework/dataobjects/EventMetaData.h>
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
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDModeByte.h>
#include <boost/tuple/tuple.hpp>
#include <fstream>
#include <sstream>
#include <regex>
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
  Module(), m_generateDigits(false)
{
  //Set module properties
  setDescription("Create SVDDigits from SVDSimHits");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Define module parameters

  // 1. Collections
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticle collection name", string(""));
  addParam("SimHits", m_storeSimHitsName, "SimHit collection name",
           string(""));
  addParam("TrueHits", m_storeTrueHitsName, "TrueHit collection name",
           string(""));
  addParam("GenerateDigits", m_generateDigits, "Generate SVDDigits", bool(false));
  addParam("Digits", m_storeDigitsName, "Digits collection name", string(""));
  addParam("ShaperDigits", m_storeShaperDigitsName, "ShaperDigits collection name", string(""));

  // 2. Physics
  addParam("SegmentLength", m_segmentLength,
           "Maximum segment length (in millimeters)", 0.020);
  addParam("WidthOfDiffusCloud", m_widthOfDiffusCloud,
           "Integration range of charge cloud in sigmas", double(3.0));

  // 3. Noise
  addParam("PoissonSmearing", m_applyPoisson,
           "Apply Poisson smearing on chargelets", bool(true));
  addParam("ElectronicEffects", m_applyNoise, "Generate noise digits",
           bool(false));
  addParam("ZeroSuppressionCut", m_SNAdjacent,
           "Zero suppression cut in sigmas of strip noise", double(3.0));
  addParam("Use3SampleFilter", m_3sampleFilter,
           "A digit must have at least 3 consecutive samples over threshold",
           bool(true));

  // 4. Timing
  addParam("APVShapingTime", m_shapingTime, "APV25 shpaing time in ns",
           double(250.0));
  addParam("ADCSamplingTime", m_samplingTime,
           "Interval between ADC samples in ns", double(31.44));
  addParam("StartSampling", m_startSampling,
           "Start of the sampling window, in ns", double(-31.44));
  addParam("nAPV25Samples", m_nAPV25Samples, "number of APV25 samples",
           6);
  addParam("RandomizeEventTimes", m_randomizeEventTimes,
           "Randomize event times over a frame interval", bool(false));
  addParam("TimeFrameLow", m_minTimeFrame,
           "Left edge of event time randomization window, ns", m_minTimeFrame);
  addParam("TimeFrameHigh", m_maxTimeFrame,
           "Right edge of event time randomization window, ns", m_maxTimeFrame);

  // 5. Reporting
  addParam("statisticsFilename", m_rootFilename,
           "ROOT Filename for statistics generation. If filename is empty, no statistics will be produced",
           string(""));
  addParam("storeWaveforms", m_storeWaveforms,
           "Store waveforms in a TTree in the statistics file.", bool(false));
  addParam("signalsList", m_signalsList,
           "Store signals (time/charge/tau) in a tab-delimited file",
           m_signalsList);
}

void SVDDigitizerModule::initialize()
{
  //Register all required collections
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);

  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);

  StoreArray<SVDShaperDigit>storeShaperDigits(m_storeShaperDigitsName);
  storeShaperDigits.registerInDataStore();
  storeShaperDigits.registerRelationTo(storeMCParticles);
  storeShaperDigits.registerRelationTo(storeTrueHits);

  m_relShaperDigitMCParticleName = DataStore::relationName(
                                     DataStore::arrayName<SVDShaperDigit>(m_storeShaperDigitsName),
                                     DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  m_relShaperDigitTrueHitName = DataStore::relationName(
                                  DataStore::arrayName<SVDShaperDigit>(m_storeShaperDigitsName),
                                  DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName));
  //  m_relShaperDigitDigitName = DataStore::relationName(
  //                  DataStore::arrayName<SVDShaperDigit>(m_storeShaperDigitsName),
  //                  DataStore::arrayName<SVDDigit>(m_storeDigitsName));

  if (m_randomizeEventTimes)
    StoreObjPtr<EventMetaData> storeEvents;

  //Set names in case default was used. We need the names to initialize the RelationIndices.
  m_relMCParticleSimHitName = DataStore::relationName(
                                DataStore::arrayName<MCParticle>(m_storeMCParticlesName),
                                DataStore::arrayName<SVDSimHit>(m_storeSimHitsName));
  m_relTrueHitSimHitName = DataStore::relationName(
                             DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName),
                             DataStore::arrayName<SVDSimHit>(m_storeSimHitsName));

  if (m_generateDigits) {
    StoreArray<SVDDigit>storeDigits(m_storeDigitsName);
    storeDigits.registerInDataStore();
    storeDigits.registerRelationTo(storeMCParticles);
    storeDigits.registerRelationTo(storeTrueHits);
    storeDigits.registerRelationTo(storeShaperDigits);

    m_relDigitMCParticleName = DataStore::relationName(
                                 DataStore::arrayName<SVDDigit>(m_storeDigitsName),
                                 DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
    m_relDigitTrueHitName = DataStore::relationName(
                              DataStore::arrayName<SVDDigit>(m_storeDigitsName),
                              DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName));
  }

  // Convert parameters to correct units
  m_segmentLength *= Unit::mm;
  m_noiseFraction = TMath::Freq(m_SNAdjacent); // 0.9... !
  m_samplingTime *= Unit::ns;
  m_shapingTime *= Unit::ns;
  m_minTimeFrame *= Unit::ns;
  m_maxTimeFrame *= Unit::ns;

  B2DEBUG(1,
          "SVDDigitizer parameters (in default system units, *=cannot be set directly):");
  B2DEBUG(1, " DATASTORE COLLECTIONS:");
  B2DEBUG(1,
          " -->  MCParticles:        " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2DEBUG(1,
          " -->  Digits:             " << DataStore::arrayName<SVDDigit>(m_storeDigitsName));
  B2DEBUG(1,
          " -->  SimHits:            " << DataStore::arrayName<SVDSimHit>(m_storeSimHitsName));
  B2DEBUG(1,
          " -->  TrueHits:           " << DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName));
  B2DEBUG(1, " -->  MCSimHitRel:        " << m_relMCParticleSimHitName);
  B2DEBUG(1, " -->  DigitMCRel:         " << m_relDigitMCParticleName);
  B2DEBUG(1, " -->  TrueSimRel:         " << m_relTrueHitSimHitName);
  B2DEBUG(1, " -->  DigitTrueRel:       " << m_relDigitTrueHitName);
  B2DEBUG(1, " PHYSICS: ");
  B2DEBUG(1, " -->  SegmentLength:      " << m_segmentLength);
  B2DEBUG(1, " -->  Charge int. range:  " << m_widthOfDiffusCloud);
  B2DEBUG(1, " NOISE: ");
  B2DEBUG(1, " -->  Add Poisson noise   " << (m_applyPoisson ? "true" : "false"));
  B2DEBUG(1, " -->  Add Gaussian noise: " << (m_applyNoise ? "true" : "false"));
  B2DEBUG(1, " -->  Zero suppression cut" << m_SNAdjacent);
  B2DEBUG(1, " -->  3-sample filter:    " << (m_3sampleFilter ? "on" : "off"));
  B2DEBUG(1, " -->  Noise fraction*:    " << 1.0 - m_noiseFraction);
  B2DEBUG(1, " TIMING: ");
  B2DEBUG(1, " -->  APV25 shaping time: " << m_shapingTime);
  B2DEBUG(1, " -->  Sampling time:      " << m_samplingTime);
  B2DEBUG(1, " -->  Start of int. wind.:" << m_startSampling);
  B2DEBUG(1, " -->  Number of samples:  " << m_nAPV25Samples);
  B2DEBUG(1, " -->  Random event times. " << (m_randomizeEventTimes ? "true" : "false"));
  B2DEBUG(1, " REPORTING: ");
  B2DEBUG(1, " -->  statisticsFilename: " << m_rootFilename);
  B2DEBUG(1,
          " -->  storeWaveforms:     " << (m_storeWaveforms ? "true" : "false"));

  if (!m_rootFilename.empty()) {
    m_rootFile = new TFile(m_rootFilename.c_str(), "RECREATE");
    m_rootFile->cd();
    m_histChargeSharing_v = new TH1D("h_Diffusion_v", " 'Diffusion' distance, v",
                                     200, -500, 500);
    m_histChargeSharing_v->GetXaxis()->SetTitle(" distance v [um]");
    m_histChargeSharing_u = new TH1D("h_Diffusion_u",
                                     " 'Diffusion' distance, u", 100, -200, 200);
    m_histChargeSharing_u->GetXaxis()->SetTitle("distance u [um]");
    m_histLorentz_u = new TH1D("h_LorentzAngle_u", "Lorentz angle, holes",
                               100, -0.08, 0);
    m_histLorentz_u->GetXaxis()->SetTitle("Lorentz angle");
    m_histLorentz_v = new TH1D("h_LorentzAngle_v",
                               "Lorentz angle, electrons", 100, -0.002, 0.002);
    m_histLorentz_v->GetXaxis()->SetTitle("Lorentz angle");
    m_signalDist_u = new TH1D("h_signalDist_u",
                              "Strip signals vs. TrueHits, holes", 100, -400, 400);
    m_signalDist_u->GetXaxis()->SetTitle("U strip position - TrueHit u [um]");
    m_signalDist_v = new TH1D("h_signalDist_v",
                              "Strip signals vs. TrueHits, electrons", 100, -400, 400);
    m_signalDist_v->GetXaxis()->SetTitle("V strip position - TrueHit v [um]");

    m_histMobility_e = new TH1D("h_elecMobility", "electron Mobility",
                                30, 900, 1200);
    m_histMobility_e->GetXaxis()->SetTitle("Electron Mobility");
    m_histMobility_h = new TH1D("h_holeMobility", "hole Mobility",
                                30, 400, 500);
    m_histMobility_h->GetXaxis()->SetTitle("Holes Mobility");

    m_histDistanceToPlane_e = new TH1D("h_elecDistToPlane", "electron Distance to Plane",
                                       50, -0.05, 0.05);
    m_histDistanceToPlane_e->GetXaxis()->SetTitle("Electron Distance To Plane [cm]");
    m_histDistanceToPlane_h = new TH1D("h_holeDistToPlane", "holes Distance to Plane",
                                       50, -0.05, 0.05);
    m_histDistanceToPlane_h->GetXaxis()->SetTitle("Holes Distance To Plane [cm]");

    m_histVelocity_e = new TH1D("h_elecVelocity", "electrons Velocity (z)",
                                100, 0.001, 0.01);

    m_histVelocity_e->GetXaxis()->SetTitle("Electron Velocity [cm/s]");
    m_histVelocity_h = new TH1D("h_holeVelocity", "holes Velocity (z)",
                                30, -0.002, -0.0004);
    m_histVelocity_h->GetXaxis()->SetTitle("holes Velocity [cm/s]");

    m_histDriftTime_e = new TH1D("h_elecDriftTime", "electron Drift Time",
                                 30, 0, 30);
    m_histDriftTime_e->GetXaxis()->SetTitle("Electron Drift Time");
    m_histDriftTime_h = new TH1D("h_holeDriftTime", "hole Drift Time",
                                 30, 0, 30);
    m_histDriftTime_h->GetXaxis()->SetTitle("Hole Drift Time");

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
  // Generate current event time
  if (m_randomizeEventTimes) {
    StoreObjPtr<EventMetaData> storeEvent;
    m_currentEventTime = gRandom->Uniform(m_minTimeFrame, m_maxTimeFrame);
    // We have negative event times, so we have to encode!
    storeEvent->setTime(static_cast<unsigned long>(1000 + m_currentEventTime));
  } else
    m_currentEventTime = 0.0;

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
  if (m_signalsList != "")
    saveSignals();

  saveDigits();
}

void SVDDigitizerModule::processHit()
{
  // Set time of the hit
  m_currentTime = m_currentEventTime + m_currentHit->getGlobalTime();

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
  B2DEBUG(30, "@@@ driftCharge: drifting " << carriers << " " << carrierName << "s at position (" << position.x() << ", " <<
          position.y() << ", " << position.z()
          << ").");

  //Get references to current sensor/info for ease of use
  const SensorInfo& info = *m_currentSensorInfo;
  StripSignals& digits = (!have_electrons) ? m_currentSensor->first : m_currentSensor->second;

  double distanceToPlane = (have_electrons) ?
                           0.5 * m_sensorThickness - position.Z() :
                           -0.5 * m_sensorThickness - position.Z(); //cm

  if (m_histDistanceToPlane_e && have_electrons) m_histDistanceToPlane_e->Fill(distanceToPlane);
  if (m_histDistanceToPlane_h && !have_electrons) m_histDistanceToPlane_h->Fill(distanceToPlane);

  // Approximation: calculate drift velocity at the point halfway towards
  // the respective sensor surface.
  TVector3 mean_pos(position.X(), position.Y(), position.Z() + 0.5 * distanceToPlane);

  // Calculate drift times and widths of charge clouds.
  TVector3 v = info.getVelocity(carrierType, mean_pos);
  if (m_histVelocity_e && have_electrons) m_histVelocity_e->Fill(v.Z()); //Unit::cm/Unit::cm*Unit::eV/Unit::e*Unit::s);
  if (m_histVelocity_h && !have_electrons) m_histVelocity_h->Fill(v.Z()); //Unit::cm/Unit::cm*Unit::eV/Unit::e*Unit::s);

  double driftTime = distanceToPlane / v.Z(); //ns
  if (m_histDriftTime_e && have_electrons) m_histDriftTime_e->Fill(driftTime); //ns
  if (m_histDriftTime_h && !have_electrons) m_histDriftTime_h->Fill(driftTime); //ns

  TVector3 center = position + driftTime * v; //cm
  double mobility = (have_electrons) ?
                    info.getElectronMobility(info.getEField(mean_pos).Mag()) :
                    info.getHoleMobility(info.getEField(mean_pos).Mag());

  if (m_histMobility_e && have_electrons) m_histMobility_e->Fill(mobility); //cm2/V/ns
  if (m_histMobility_h && !have_electrons) m_histMobility_h->Fill(mobility); //cm2/V/ns

  double D = Const::kBoltzmann * info.getTemperature() / Unit::e * mobility;
  double sigma = std::max(1.0e-4, sqrt(2.0 * D * driftTime));
  double tanLorentz = (!have_electrons) ? v.X() / v.Z() : v.Y() / v.Z();

  B2DEBUG(30, "velocity (" << v.X() / Unit::um << ", " << v.Y() / Unit::um << ", " << v.Z() / Unit::um << ") um/ns");
  B2DEBUG(30, "D = " << D << ", driftTime = " << driftTime / Unit::ns << " ns");
  B2DEBUG(30, "sigma = " << sigma / Unit::um << " um");
  B2DEBUG(30, "tan Lorentz = " << tanLorentz);

  sigma *= sqrt(1.0 + tanLorentz * tanLorentz);
  if (m_histLorentz_u && !have_electrons) m_histLorentz_u->Fill(tanLorentz);
  if (m_histLorentz_v && have_electrons) m_histLorentz_v->Fill(tanLorentz);

  //Distribute carrier cloud on strips
  int vID = info.getVCellID(center.Y(), true);
  int uID = info.getUCellID(center.X(), center.Y(), true);
  int seedStrip = (!have_electrons) ? uID : vID;
  double seedPos = (!have_electrons) ?
                   info.getUCellPosition(seedStrip, vID) :
                   info.getVCellPosition(seedStrip);
  double geomPitch = (!have_electrons) ? 0.5 * info.getUPitch(center.Y()) : 0.5 * info.getVPitch();
  int nCells = (!have_electrons) ? info.getUCells() : info.getVCells();
  std::deque<double> stripCharges;
  std::deque<double> strips; // intermediate strips will be half-integers, like 2.5.
#define NORMAL_CDF(z) 0.5 * std::erfc( - (z) * 0.707107)
  double current_pos = (!have_electrons) ? seedPos - center.X() : seedPos - center.Y();
  double current_strip = seedStrip;
  double cdf_low = NORMAL_CDF((current_pos - 0.5 * geomPitch) / sigma);
  double cdf_high = NORMAL_CDF((current_pos + 0.5 * geomPitch) / sigma);
  double charge = carriers * (cdf_high - cdf_low);

  B2DEBUG(30, "geomPitch = " << geomPitch / Unit::um << " um");
  B2DEBUG(30, "charge = " << charge << " = " << carriers << "(carriers) * (" << cdf_high << "(cdf_high) - " << cdf_low <<
          "(cdf_low));");

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
  current_pos = (!have_electrons) ? seedPos - center.X() : seedPos - center.Y();
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
  B2DEBUG(30, "  --> charge sharing simulation, # strips = " << strips.size());
  std::deque<double> readoutCharges;
  std::deque<int> readoutStrips;
  for (std::size_t index = 2; index <  strips.size() - 2; index += 2) {
    B2DEBUG(30, "  index = " << index << ", strip = " << strips[index] << ", stripCharge = " << stripCharges[index]);
    int currentStrip = static_cast<int>(strips[index]);
    double Cc = (!have_electrons) ? info.getCouplingCapacitanceU(currentStrip) :
                info.getCouplingCapacitanceV(currentStrip);
    double Ci = (!have_electrons) ? info.getInterstripCapacitanceU(currentStrip) :
                info.getInterstripCapacitanceV(currentStrip);
    double Cb = (!have_electrons) ? info.getBackplaneCapacitanceU(currentStrip) :
                info.getBackplaneCapacitanceV(currentStrip);

    double cNeighbour2 = 0.5 * Ci / (Ci + Cb + Cc);
    double cNeighbour1 = Ci / (2 * Ci + Cb);
    double cSelf = Cc / (Ci + Cb + Cc);

    B2DEBUG(30, "  current strip = " << currentStrip);
    B2DEBUG(30, "     index-2 = " << index - 2 << ", strip = " << strips[index - 2] << ", stripCharge = " << stripCharges[index - 2]);
    B2DEBUG(30, "     index-1 = " << index - 1 << ", strip = " << strips[index - 1] << ", stripCharge = " << stripCharges[index - 1]);
    B2DEBUG(30, "     index   = " << index << ", strip = " << strips[index] << ", stripCharge = " << stripCharges[index]);
    B2DEBUG(30, "     index+1 = " << index + 1 << ", strip = " << strips[index + 1] << ", stripCharge = " << stripCharges[index + 1]);
    B2DEBUG(30, "     index+2 = " << index + 2 << ", strip = " << strips[index + 2] << ", stripCharge = " << stripCharges[index + 2]);
    //se index e' di readout va bene
    readoutCharges.push_back(cSelf * (
                               cNeighbour2 * stripCharges[index - 2]
                               + cNeighbour1 * stripCharges[index - 1]
                               + stripCharges[index]
                               + cNeighbour1 * stripCharges[index + 1]
                               + cNeighbour2 * stripCharges[index + 2]
                             ));
    readoutStrips.push_back(currentStrip);
    B2DEBUG(30, "    post simulation: " << index << ", strip = " << currentStrip << ", readoutCharge = " <<
            readoutCharges[readoutCharges.size() - 1]);
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
  if (m_histChargeSharing_u && m_histChargeSharing_v) {
    TH1D* histo = (!have_electrons) ? m_histChargeSharing_u : m_histChargeSharing_v;
    double d = (!have_electrons) ? seedPos - center.X() : seedPos - center.Y();
    for (std::size_t index = 0; index < readoutStrips.size(); ++ index) {
      double dist = d + (readoutStrips[index] - seedStrip) * 2 * geomPitch;
      histo->Fill(dist / Unit::um, readoutCharges[index]);
    }
  }
  // Store
  double recoveredCharge = 0;
  for (std::size_t index = 0; index <  readoutStrips.size(); index ++) {
    // NB> To first approximation, we assign to the signal 1/2*driftTime.
    // This doesn't change the charge collection, only charge collection timing.
    digits[readoutStrips[index]].add(m_currentTime + 0.5 * driftTime, readoutCharges[index],
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
    charge += gRandom->Gaus(0., noise);
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

  StoreArray<SVDShaperDigit> storeShaperDigits(m_storeShaperDigitsName);
  RelationArray relShaperDigitMCParticle(storeShaperDigits, storeMCParticles,
                                         m_relShaperDigitMCParticleName);
  RelationArray relShaperDigitTrueHit(storeShaperDigits, storeTrueHits,
                                      m_relShaperDigitTrueHitName);
  //    RelationArray relShaperDigitDigits(storeShaperDigits, storeDigits,
  //               m_relShaperDigitDigitName);

  //Set time of the first sample
  const double bunchTimeSep = 2 * 1.96516; //in ns
  const int bunchXingsInAPVclock = 8; //m_samplingTime/bunchTimeSep;
  int bunchXingsSinceAPVstart = gRandom->Integer(bunchXingsInAPVclock);
  double initTime = m_startSampling - bunchTimeSep * bunchXingsSinceAPVstart;

  //set the DAQ mode to 1, 3, or 6-samples:
  int daqMode = 3;  //does not correspond to anything expected on data
  if (m_nAPV25Samples == 6)
    daqMode = 2;
  else if (m_nAPV25Samples == 3)
    daqMode = 1;
  else if (m_nAPV25Samples == 1)
    daqMode = 0;
  else
    B2WARNING("The number of APV samples that you are simulating is not expected! If you are using the CoG in recontruction, do not expect to get reasonable RecoDigits");

  // ... to store digit-digit relations
  vector<pair<unsigned int, float> > digit_weights;

  // Take samples at the desired times, add noise, zero-suppress and save digits.

  for (Sensors::value_type& sensor : m_sensors) {
    int sensorID = sensor.first;
    const SensorInfo& info =
      dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));
    // u-side digits:
    double elNoiseU = info.getElectronicNoiseU();
    double aduEquivalentU = info.getAduEquivalentU();
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
      // ... to store digit-digit relations
      digit_weights.clear();
      digit_weights.reserve(SVDShaperDigit::c_nAPVSamples);
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
      if (m_3sampleFilter) {
        auto it = search_n(
                    samples.begin(), samples.end(), 3, charge_thresholdU,
        [](double x, double y) { return x > y; }
                  );
        if (it == samples.end()) continue;
      }

      SVDSignal::relations_map particles = s.getMCParticleRelations();
      SVDSignal::relations_map truehits = s.getTrueHitRelations();

      // Save samples and relations
      if (m_generateDigits) {
        for (int iSample = 0; iSample < m_nAPV25Samples; iSample++) {
          double sampleCharge = samples.at(iSample);
          //NB: no longer optional!!! Limit signal to ADC steps
          sampleCharge = floor(max(0.0, min(255.0, sampleCharge / aduEquivalentU)));
          // Save as a new digit
          int digIndex = storeDigits.getEntries();
          digit_weights.emplace_back(digIndex, sampleCharge);
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
                    (info.getUCellPosition(iStrip) - storeTrueHits[iTrueHit]->getU()) / Unit::um, trueWeight);
                } // if iTrieHit
              } // for trueRel
            } // if m_signalDist_u
          } // if truehits.size()
        } // for iSample
      } // Save SVDDigits if required

      // Save SVDShaperDigits
      SVDShaperDigit::APVRawSamples rawSamples;
      std::transform(samples.begin(), samples.end(), rawSamples.begin(),
      [&](double x)->SVDShaperDigit::APVRawSampleType {
        return SVDShaperDigit::trimToSampleRange(x / aduEquivalentU);
      });
      // Save as a new digit
      int digIndex = storeShaperDigits.getEntries();
      storeShaperDigits.appendNew(SVDShaperDigit(sensorID, true, iStrip, rawSamples, 0, SVDModeByte(0, 0, daqMode,
                                                 bunchXingsSinceAPVstart >> 1)));
      //If the digit has any relations to MCParticles, add the Relation
      if (particles.size() > 0) {
        relShaperDigitMCParticle.add(digIndex, particles.begin(), particles.end());
      }
      //If the digit has any relations to truehits, add the Relations.
      if (truehits.size() > 0) {
        relShaperDigitTrueHit.add(digIndex, truehits.begin(), truehits.end());
      }
      //      relShaperDigitDigits.add(digIndex, digit_weights.begin(), digit_weights.end());
      // generate SVDShaperDigits
    } // for stripSignals

    // v-side digits:
    double elNoiseV = info.getElectronicNoiseV();
    double aduEquivalentV = info.getAduEquivalentV();
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
      // ... to store digit-digit relations
      digit_weights.clear();
      digit_weights.reserve(SVDShaperDigit::c_nAPVSamples);
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
      if (m_3sampleFilter) {
        auto it = search_n(
                    samples.begin(), samples.end(), 3, charge_thresholdV,
        [](double x, double y) { return x > y; }
                  );
        if (it == samples.end()) continue;
      }

      SVDSignal::relations_map particles = s.getMCParticleRelations();
      SVDSignal::relations_map truehits = s.getTrueHitRelations();

      // Save samples and relations
      if (m_generateDigits) {
        for (int iSample = 0; iSample < m_nAPV25Samples; iSample++) {
          double sampleCharge = samples.at(iSample);
          //NB: no longer optional!!! Limit signal to 8-bit ADC steps
          sampleCharge = floor(max(0.0, min(255.0, sampleCharge / aduEquivalentV)));
          // Save as a new digit
          int digIndex = storeDigits.getEntries();
          digit_weights.emplace_back(digIndex, sampleCharge);
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
                    (info.getVCellPosition(iStrip) - storeTrueHits[iTrueHit]->getV()) / Unit::um, trueWeight);
                } // if iTrieHit
              } // for trueRel
            } // if m_signalDist_v
          } // if truehits.size()
        } // for iSample
      } // Save SVDShaperDigits if required

      // Save SVDShaperDigits
      SVDShaperDigit::APVRawSamples rawSamples;
      std::transform(samples.begin(), samples.end(), rawSamples.begin(),
      [&](double x)->SVDShaperDigit::APVRawSampleType {
        return SVDShaperDigit::trimToSampleRange(x / aduEquivalentV);
      });
      // Save as a new digit
      int digIndex = storeShaperDigits.getEntries();
      storeShaperDigits.appendNew(SVDShaperDigit(sensorID, false, iStrip, rawSamples, 0, SVDModeByte(0, 0, daqMode,
                                                 bunchXingsSinceAPVstart >> 1)));
      //If the digit has any relations to MCParticles, add the Relation
      if (particles.size() > 0) {
        relShaperDigitMCParticle.add(digIndex, particles.begin(), particles.end());
      }
      //If the digit has any relations to truehits, add the Relations.
      if (truehits.size() > 0) {
        relShaperDigitTrueHit.add(digIndex, truehits.begin(), truehits.end());
      }
      //      relShaperDigitDigits.add(digIndex, digit_weights.begin(), digit_weights.end());
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

void SVDDigitizerModule::saveSignals()
{
  static size_t recordNo = 0;
  static const string header("Event\tSensor\tSide\tStrip\tContrib\tTime\tCharge\tTau");
  regex startLine("^|\n"); // for inserting event/sensor/etc info
  ofstream outfile(m_signalsList, ios::out | ios::app);
  if (recordNo == 0) outfile << header << endl;
  for (Sensors::value_type& sensor : m_sensors) {
    VxdID sensorID(sensor.first);
    const SensorInfo& info =
      dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensor.first));
    // u-side digits:
    size_t isU = 1;
    double thresholdU = 3.0 * info.getElectronicNoiseU();
    for (StripSignals::value_type& stripSignal : sensor.second.first) {
      size_t strip = stripSignal.first;
      SVDSignal& s = stripSignal.second;
      // Read the value only if the signal is large enough.
      if (s.getCharge() < thresholdU)
        continue;
      // Else print to a string
      ostringstream preamble;
      // We don't have eventNo, but we don't care about event boundaries.
      preamble << "$&" << recordNo << '\t' << sensorID << '\t' << isU << '\t' << strip << '\t';
      string signalString = s.toString();
      signalString.pop_back(); // remove the last newline!!!
      string tableString = regex_replace(signalString, startLine, preamble.str());
      outfile << tableString << endl; // now we have to add the newline back.
    } // FOREACH stripSignal
    // x-side digits:
    isU = 0;
    double thresholdV = 3.0 * info.getElectronicNoiseV();
    for (StripSignals::value_type& stripSignal : sensor.second.second) {
      size_t strip = stripSignal.first;
      SVDSignal& s = stripSignal.second;
      // Read the value only if the signal is large enough.
      if (s.getCharge() < thresholdV)
        continue;
      // Else print to a string
      ostringstream preamble;
      // We don't have eventNo, but we don't care about event boundaries.
      preamble << "$&" << recordNo << '\t' << sensorID << '\t' << isU << '\t' << strip << '\t';
      string signalString = s.toString();
      signalString.pop_back(); // remove the last newline!!!
      string tableString = regex_replace(signalString, startLine, preamble.str());
      outfile << tableString << endl; // now we have to add the newline back.
    } // FOREACH stripSignal
  } // for sensors
  outfile.close();
  recordNo++;
}

void SVDDigitizerModule::terminate()
{
  if (m_rootFile) {
    m_rootFile->Write();
    m_rootFile->Close();
  }
}

