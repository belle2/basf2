/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Zbynek Drasal, Martin Ritter, Peter Kvasnicka,           *
 *               Giulia Casarosa
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
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
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

std::string Belle2::SVD::SVDDigitizerModule::m_xmlFileName = std::string("SVDChannelMapping.xml");
SVDDigitizerModule::SVDDigitizerModule() : Module(),
  m_currentSensorWaveforms(nullptr),
  m_mapping(m_xmlFileName)
{
  //Set module properties
  setDescription("Creates SVDShaperDigits from SVDSimHits");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Define module parameters

  // 1. Collections
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticle collection name", m_storeMCParticlesName);
  addParam("SimHits", m_storeSimHitsName, "SimHit collection name",
           m_storeSimHitsName);
  addParam("TrueHits", m_storeTrueHitsName, "TrueHit collection name",
           m_storeTrueHitsName);
  addParam("ShaperDigits", m_storeShaperDigitsName, "ShaperDigits collection name", m_storeShaperDigitsName);
  addParam("SVDEventInfo", m_svdEventInfoName, "SVDEventInfo name", m_svdEventInfoName);

  // 2. Physics
  addParam("SegmentLength", m_segmentLength,
           "Maximum segment length (in millimeters)", m_segmentLength);

  // 3. Noise
  addParam("PoissonSmearing", m_applyPoisson,
           "Apply Poisson smearing on chargelets", m_applyPoisson);
  addParam("ZeroSuppressionCut", m_SNAdjacent,
           "Zero suppression cut in sigmas of strip noise", m_SNAdjacent);
  addParam("FADCmode", m_roundZS,
           "FADC mode: if True, ZS cut is rounded to nearest ADU ", m_roundZS);
  addParam("numberOfSamples", m_nSamplesOverZS,
           "Keep digit if numberOfSamples or more samples are over ZS threshold",
           m_nSamplesOverZS);

  // 4. Timing
  addParam("APVShapingTime", m_shapingTime, "APV25 shaping time in ns",
           m_shapingTime);
  addParam("ADCSamplingTime", m_samplingTime,
           "Interval between ADC samples in ns, if = -1 taken from HardwareClockSettings payload (default).", m_samplingTime);
  addParam("StartSampling", m_startSampling,
           "Start of the sampling window, in ns. Used to tune the SVD latency.", m_startSampling);
  addParam("RandomizeEventTimes", m_randomizeEventTimes,
           "Randomize event times over a frame interval", m_randomizeEventTimes);
  addParam("TimeFrameLow", m_minTimeFrame,
           "Left edge of event time randomization window, ns", m_minTimeFrame);
  addParam("TimeFrameHigh", m_maxTimeFrame,
           "Right edge of event time randomization window, ns", m_maxTimeFrame);


  // 6. Reporting
  addParam("statisticsFilename", m_rootFilename,
           "ROOT Filename for statistics generation. If filename is empty, no statistics will be produced",
           m_rootFilename);
  addParam("storeWaveforms", m_storeWaveforms,
           "Store waveforms in a TTree in the statistics file.", m_storeWaveforms);
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

  if (m_randomizeEventTimes)
    StoreObjPtr<EventMetaData> storeEvents;

  //Set names in case default was used. We need the names to initialize the RelationIndices.
  m_relMCParticleSimHitName = DataStore::relationName(
                                DataStore::arrayName<MCParticle>(m_storeMCParticlesName),
                                DataStore::arrayName<SVDSimHit>(m_storeSimHitsName));
  m_relTrueHitSimHitName = DataStore::relationName(
                             DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName),
                             DataStore::arrayName<SVDSimHit>(m_storeSimHitsName));

  // Convert parameters to correct units
  m_segmentLength *= Unit::mm;
  m_noiseFraction = TMath::Freq(m_SNAdjacent); // 0.9... !
  m_samplingTime *= Unit::ns;
  m_shapingTime *= Unit::ns;
  m_minTimeFrame *= Unit::ns;
  m_maxTimeFrame *= Unit::ns;

  B2DEBUG(29,
          "SVDDigitizer parameters (in default system units, *=cannot be set directly):");
  B2DEBUG(29, " DATASTORE COLLECTIONS:");
  B2DEBUG(29,
          " -->  MCParticles:        " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2DEBUG(29,
          " -->  Digits:             " << DataStore::arrayName<SVDShaperDigit>(m_storeShaperDigitsName));
  B2DEBUG(29,
          " -->  SimHits:            " << DataStore::arrayName<SVDSimHit>(m_storeSimHitsName));
  B2DEBUG(29,
          " -->  TrueHits:           " << DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName));
  B2DEBUG(29, " -->  MCSimHitRel:        " << m_relMCParticleSimHitName);
  B2DEBUG(29, " -->  DigitMCRel:         " << m_relShaperDigitMCParticleName);
  B2DEBUG(29, " -->  TrueSimRel:         " << m_relTrueHitSimHitName);
  B2DEBUG(29, " -->  DigitTrueRel:       " << m_relShaperDigitTrueHitName);
  B2DEBUG(29, " PHYSICS: ");
  B2DEBUG(29, " -->  SegmentLength:      " << m_segmentLength);
  B2DEBUG(29, " NOISE: ");
  B2DEBUG(29, " -->  Add Poisson noise   " << (m_applyPoisson ? "true" : "false"));
  B2DEBUG(29, " -->  Zero suppression cut" << m_SNAdjacent);
  B2DEBUG(29, " -->  Round ZS cut:       " << (m_roundZS ? "true" : "false"));
  B2DEBUG(29, " -->  Samples over ZS cut:" << m_nSamplesOverZS);
  B2DEBUG(29, " -->  Noise fraction*:    " << 1.0 - m_noiseFraction);
  B2DEBUG(29, " TIMING: ");
  B2DEBUG(29, " -->  APV25 shaping time: " << m_shapingTime);
  B2DEBUG(29, " -->  Sampling time:      " << m_samplingTime);
  B2DEBUG(29, " -->  Start of int. wind.:" << m_startSampling);
  B2DEBUG(29, " -->  Random event times. " << (m_randomizeEventTimes ? "true" : "false"));
  B2DEBUG(29, " REPORTING: ");
  B2DEBUG(29, " -->  statisticsFilename: " << m_rootFilename);
  B2DEBUG(29,
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

    m_histHitTime = new TH1D("h_startAPVTime", "start APV Time",
                             200, -100, 100);
    m_histHitTime->GetXaxis()->SetTitle("time (ns)");
    m_histHitTimeTB = new TH2F("h_startAPVTimeTB", "start APV Time vs TB",
                               200, -100, 100, 4, -0.5, 3.5);
    m_histHitTimeTB->GetXaxis()->SetTitle("time (ns)");
    m_histHitTimeTB->GetYaxis()->SetTitle("TB");

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
}

void SVDDigitizerModule::beginRun()
{

  if (m_mapping.hasChanged()) { m_map = std::make_unique<SVDOnlineToOfflineMap>(m_mapping->getFileName()); }

  //read sampling time from HardwareClockSettings
  if (m_samplingTime == -1 && m_hwClock.isValid())
    m_samplingTime = 1. / m_hwClock->getClockFrequency(Const::EDetector::SVD, "sampling");
  else if (m_samplingTime == -1)
    m_samplingTime = 16000. / 509;

  //Fill map with all possible sensors This is too slow to be done every event so
  //we fill it once and only clear the content of the sensors per event, not
  //the whole map
  m_waveforms.clear();
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (VxdID layer : geo.getLayers(SensorInfo::SVD)) {
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        m_waveforms[sensor] = SensorWaveforms();
      }
    }
  }

  if (!m_MaskedStr.isValid())
    B2WARNING("No valid SVDFADCMaskedStrip for the requested IoV -> no strips masked");
  if (!m_map)
    B2WARNING("No valid channel mapping -> all APVs will be enabled");


}

void SVDDigitizerModule::event()
{

  //get number of samples and relativeShift
  StoreObjPtr<SVDEventInfo> storeSVDEvtInfo(m_svdEventInfoName);
  SVDModeByte modeByte = storeSVDEvtInfo->getModeByte();
  m_relativeShift = storeSVDEvtInfo->getRelativeShift();
  m_nAPV25Samples = storeSVDEvtInfo->getNSamples();

  //Compute time of the first sample, update latency
  const double systemClockPeriod = 1. / m_hwClock->getGlobalClockFrequency();
  int triggerBin = modeByte.getTriggerBin();

  m_initTime = m_startSampling - systemClockPeriod * triggerBin;

  m_is3sampleEvent = false;
  if (m_nAPV25Samples == 3) {
    m_is3sampleEvent = true;
    m_startingSample = getFirstSample(triggerBin, m_relativeShift);
    B2DEBUG(25, "3-sample event, starting sample = " << m_startingSample);
  } else m_startingSample = 0; //not used

  // set APV mode for background overlay
  SVDShaperDigit::setAPVMode(m_nAPV25Samples, m_startingSample);

  // Generate current event time
  if (m_randomizeEventTimes) {
    StoreObjPtr<EventMetaData> storeEvent;
    m_currentEventTime = gRandom->Uniform(m_minTimeFrame, m_maxTimeFrame);
    // We have negative event times, so we have to encode!
    storeEvent->setTime(static_cast<unsigned long>(1000 + m_currentEventTime));
  } else
    m_currentEventTime = 0.0;

  // Clear sensors' waveforms and process SimHits
  for (Waveforms::value_type& sensorWaveforms : m_waveforms) {
    sensorWaveforms.second.first.clear();  // u-side channels
    sensorWaveforms.second.second.clear(); // v-side channels
  }
  // m_currentSensorWaveforms = 0;
  // m_currentSensorInfo = 0;

  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<SVDSimHit> storeSimHits(m_storeSimHitsName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);

  RelationArray mcParticlesToSimHits(storeMCParticles, storeSimHits, m_relMCParticleSimHitName);
  RelationArray trueHitsToSimHits(storeTrueHits, storeSimHits, m_relTrueHitSimHitName);

  RelationIndex<MCParticle, SVDSimHit> relMCParticleSimHit(storeMCParticles, storeSimHits, m_relMCParticleSimHitName);
  RelationIndex<SVDTrueHit, SVDSimHit> relTrueHitSimHit(storeTrueHits, storeSimHits, m_relTrueHitSimHitName);

  unsigned int nSimHits = storeSimHits.getEntries();
  if (nSimHits == 0) {
    return;
  }

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
      if (m_currentHit->getBackgroundTag() == BackgroundMetaData::bg_none)
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
        dynamic_cast<const SensorInfo*>(&VXD::GeoCache::get(sensorID));
      if (!m_currentSensorInfo)
        B2FATAL(
          "Sensor Information for Sensor " << sensorID << " not found, make sure that the geometry is set up correctly");

      const SensorInfo& info = *m_currentSensorInfo;
      // Publish some useful data
      m_sensorThickness = info.getThickness();
      m_currentSensorWaveforms = &m_waveforms[sensorID];
      B2DEBUG(29,
              "Sensor Parameters for Sensor " << sensorID << ": " << endl
              << " --> Width:          " << m_currentSensorInfo->getWidth() << endl
              << " --> Length:         " << m_currentSensorInfo->getLength() << endl
              << " --> uPitch:         " << m_currentSensorInfo->getUPitch() << endl
              << " --> vPitch:         " << m_currentSensorInfo->getVPitch(-m_currentSensorInfo->getLength() / 2.0)
              << ", " << m_currentSensorInfo->getVPitch(m_currentSensorInfo->getLength() / 2.0) << endl
              << " --> Thickness:      " << m_currentSensorInfo->getThickness() << endl
              << " --> Deplet. voltage:" << m_currentSensorInfo->getDepletionVoltage() << endl
              << " --> Bias voltage:   " << m_currentSensorInfo->getBiasVoltage() << endl
             );

    }
    B2DEBUG(28,
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

  if (m_currentHit->getPDGcode() == Const::photon.getPDGCode() || trackLength < 0.1 * Unit::um) {
    //Photons deposit energy at the end of their step
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
  B2DEBUG(29,
          "Drifting " << carriers << " " << carrierName << "s at position (" << position.x() << ", " << position.y() << ", " << position.z()
          << ").");
  B2DEBUG(29, "@@@ driftCharge: drifting " << carriers << " " << carrierName << "s at position (" << position.x() << ", " <<
          position.y() << ", " << position.z()
          << ").");

  // Get references to current sensor/info for ease of use
  const SensorInfo& info = *m_currentSensorInfo;
  StripWaveforms& waveforms = (!have_electrons) ? m_currentSensorWaveforms->first : m_currentSensorWaveforms->second;

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

  B2DEBUG(29, "velocity (" << v.X() / Unit::um << ", " << v.Y() / Unit::um << ", " << v.Z() / Unit::um << ") um/ns");
  B2DEBUG(29, "D = " << D << ", driftTime = " << driftTime / Unit::ns << " ns");
  B2DEBUG(29, "sigma = " << sigma / Unit::um << " um");
  B2DEBUG(29, "tan Lorentz = " << tanLorentz);

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

  B2DEBUG(29, "geomPitch = " << geomPitch / Unit::um << " um");
  B2DEBUG(29, "charge = " << charge << " = " << carriers << "(carriers) * (" << cdf_high << "(cdf_high) - " << cdf_low <<
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
  int npads = (strips.front() - floor(strips.front()) == 0) ? 5 : 4;
  for (int i = 0; i < npads; ++i) {
    strips.push_front(strips.front() - 0.5);
    stripCharges.push_front(0);
  }
  npads = (strips.back() - floor(strips.back()) == 0) ? 5 : 4;
  for (int i = 0; i < npads; ++i) {
    strips.push_back(strips.back() + 0.5);
    stripCharges.push_back(0);
  }
  // Charge sharing
  B2DEBUG(29, "  --> charge sharing simulation, # strips = " << strips.size());
  std::deque<double> readoutCharges;
  std::deque<int> readoutStrips;
  VxdID currentSensorID = m_currentHit->getSensorID();
  for (std::size_t index = 3; index <  strips.size() - 3; index += 2) {
    B2DEBUG(29, "  index = " << index << ", strip = " << strips[index] << ", stripCharge = " << stripCharges[index]);
    int currentStrip = static_cast<int>(strips[index]);

    double c0 = m_ChargeSimCal.getCouplingConstant(currentSensorID, !have_electrons, "C0");
    double c1 = m_ChargeSimCal.getCouplingConstant(currentSensorID, !have_electrons, "C1");
    double c2 = m_ChargeSimCal.getCouplingConstant(currentSensorID, !have_electrons, "C2");
    double c3 = m_ChargeSimCal.getCouplingConstant(currentSensorID, !have_electrons, "C3");

    B2DEBUG(29, "  current strip = " << currentStrip);
    B2DEBUG(29, "     index-3 = " << index - 3 << ", strip = " << strips[index - 3] << ", stripCharge = " << stripCharges[index - 3]);
    B2DEBUG(29, "     index-2 = " << index - 2 << ", strip = " << strips[index - 2] << ", stripCharge = " << stripCharges[index - 2]);
    B2DEBUG(29, "     index-1 = " << index - 1 << ", strip = " << strips[index - 1] << ", stripCharge = " << stripCharges[index - 1]);
    B2DEBUG(29, "     index   = " << index << ", strip = " << strips[index] << ", stripCharge = " << stripCharges[index]);
    B2DEBUG(29, "     index+1 = " << index + 1 << ", strip = " << strips[index + 1] << ", stripCharge = " << stripCharges[index + 1]);
    B2DEBUG(29, "     index+2 = " << index + 2 << ", strip = " << strips[index + 2] << ", stripCharge = " << stripCharges[index + 2]);
    B2DEBUG(29, "     index+3 = " << index + 3 << ", strip = " << strips[index + 3] << ", stripCharge = " << stripCharges[index + 3]);

    readoutCharges.push_back(c3 * stripCharges[index - 3]
                             + c2 * stripCharges[index - 2]
                             + c1 * stripCharges[index - 1]
                             + c0 * stripCharges[index]
                             + c1 * stripCharges[index + 1]
                             + c2 * stripCharges[index + 2]
                             + c3 * stripCharges[index + 3]
                            );
    readoutStrips.push_back(currentStrip);
    B2DEBUG(29, "    post simulation: " << index << ", strip = " << currentStrip << ", readoutCharge = " <<
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
  if (m_histHitTime) {
    m_histHitTime->Fill(m_currentTime);
    StoreObjPtr<SVDEventInfo> storeSVDEvtInfo(m_svdEventInfoName);
    SVDModeByte modeByte = storeSVDEvtInfo->getModeByte();
    m_histHitTimeTB->Fill(m_currentTime, modeByte.getTriggerBin());
  }

  // Store
  B2DEBUG(29, "currentTime = " << m_currentTime << " + 0.5 driftTime = " << 0.5 * driftTime << " = " << m_currentTime + 0.5 *
          driftTime);

  double apvCoupling = m_ChargeSimCal.getCouplingConstant(currentSensorID, !have_electrons, "APVCoupling");

  double recoveredCharge = 0;
  for (std::size_t index = 0; index <  readoutStrips.size(); index ++) {
    // NB> To first approximation, we assign to the signal 1/2*driftTime.
    // This doesn't change the charge collection, only charge collection timing.
    waveforms[readoutStrips[index]].add(m_currentTime + 0.5 * driftTime, readoutCharges[index],
                                        m_shapingTime, m_currentParticle, m_currentTrueHit, w_betaprime);
    recoveredCharge += readoutCharges[index];
    B2DEBUG(29, "strip: " << readoutStrips[index] << " charge: " << readoutCharges[index]);
  }
  B2DEBUG(29, "Digitized " << recoveredCharge << " of " << carriers << " original carriers.");
}

double SVDDigitizerModule::addNoise(double charge, double noise)
{
  charge += gRandom->Gaus(0., noise);
  return charge;
}

void SVDDigitizerModule::saveDigits()
{

  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<SVDShaperDigit> storeShaperDigits(m_storeShaperDigitsName);
  RelationArray relShaperDigitMCParticle(storeShaperDigits, storeMCParticles,
                                         m_relShaperDigitMCParticleName);
  RelationArray relShaperDigitTrueHit(storeShaperDigits, storeTrueHits,
                                      m_relShaperDigitTrueHitName);

  //Get SVD config from SVDEventInfo
  //  int runType = (int) modeByte.getRunType();
  //  int eventType = (int) modeByte.getEventType();


  // ... to store digit-digit relations
  vector<pair<unsigned int, float> > digit_weights;

  // Take samples at the desired times, add noise, zero-suppress and save digits.
  for (Waveforms::value_type& sensorWaveforms : m_waveforms) {
    int sensorID = sensorWaveforms.first;
    // u-side digits:

    // Cycle through signals and generate samples
    for (StripWaveforms::value_type& stripWaveform : sensorWaveforms.second.first) {
      short int iStrip = stripWaveform.first;
      SVDWaveform& s = stripWaveform.second;
      // Now generate samples in time and save as digits.
      vector<double> samples;
      // ... to store digit-digit relations
      digit_weights.clear();
      digit_weights.reserve(SVDShaperDigit::c_nAPVSamples);

      double elNoise = m_NoiseCal.getNoiseInElectrons(sensorID, true, iStrip);
      double gain = 1 / m_PulseShapeCal.getChargeFromADC(sensorID, true, iStrip, 1);
      double electronWeight = m_ChargeSimCal.getElectronWeight(sensorID, true);

      double t = m_initTime;
      B2DEBUG(25, "start sampling at " << m_initTime);
      for (int iSample = 0; iSample < (int) SVDShaperDigit::c_nAPVSamples; iSample ++) {
        samples.push_back(addNoise(electronWeight * s(t), elNoise));
        t += m_samplingTime;
      }

      SVDWaveform::relations_map particles = s.getMCParticleRelations();
      SVDWaveform::relations_map truehits = s.getTrueHitRelations();

      // Save SVDShaperDigits

      // 1. Convert to ADU
      SVDShaperDigit::APVRawSamples rawSamples;
      std::transform(samples.begin(), samples.end(), rawSamples.begin(),
      [&](double x)->SVDShaperDigit::APVRawSampleType {
        return SVDShaperDigit::trimToSampleRange(x * gain);
      });

      // 2.a Check if over threshold
      auto rawThreshold = m_SNAdjacent * elNoise * gain;
      if (m_roundZS) rawThreshold = round(rawThreshold);
      auto n_over = std::count_if(rawSamples.begin(), rawSamples.end(),
                                  std::bind2nd(std::greater<double>(), rawThreshold)
                                 );
      if (n_over < m_nSamplesOverZS) continue;

      // 2.b check if the strip is masked
      if (m_MaskedStr.isMasked(sensorID, true, iStrip)) continue;

      // 2.c check if the APV is disabled
      if (!m_map->isAPVinMap(sensorID, true, iStrip)) continue;

      // 2.d.1 check if it's a 3-sample event
      if (m_is3sampleEvent) {
        rawSamples[0] = rawSamples[m_startingSample];
        rawSamples[1] = rawSamples[m_startingSample + 1];
        rawSamples[2] = rawSamples[m_startingSample + 2];
        rawSamples[3] = 0.;
        rawSamples[4] = 0.;
        rawSamples[5] = 0.;
        //2.d.2 check if still over threshold
        n_over = std::count_if(rawSamples.begin(), rawSamples.end(),
                               std::bind2nd(std::greater<double>(), rawThreshold)
                              );
        if (n_over < m_nSamplesOverZS) continue;

      }

      // 3. Save as a new digit
      int digIndex = storeShaperDigits.getEntries();
      storeShaperDigits.appendNew(sensorID, true, iStrip, rawSamples, 0);

      //If the digit has any relations to MCParticles, add the Relation
      if (particles.size() > 0) {
        relShaperDigitMCParticle.add(digIndex, particles.begin(), particles.end());
      }
      //If the digit has any relations to truehits, add the Relations.
      if (truehits.size() > 0) {
        relShaperDigitTrueHit.add(digIndex, truehits.begin(), truehits.end());
      }
      // generate SVDShaperDigits
    } // for stripSignals

    // v-side digits:

    // Cycle through signals and generate samples
    for (StripWaveforms::value_type& stripWaveform : sensorWaveforms.second.second) {
      short int iStrip = stripWaveform.first;
      SVDWaveform& s = stripWaveform.second;
      // Now generate samples in time and save as digits.
      vector<double> samples;
      // ... to store digit-digit relations
      digit_weights.clear();
      digit_weights.reserve(SVDShaperDigit::c_nAPVSamples);

      double elNoise = m_NoiseCal.getNoiseInElectrons(sensorID, false, iStrip);
      double gain = 1 / m_PulseShapeCal.getChargeFromADC(sensorID, false, iStrip, 1);
      double electronWeight = m_ChargeSimCal.getElectronWeight(sensorID, false);

      double t = m_initTime;
      for (int iSample = 0; iSample < (int)SVDShaperDigit::c_nAPVSamples; iSample ++) {
        samples.push_back(addNoise(electronWeight * s(t), elNoise));
        t += m_samplingTime;
      }

      SVDWaveform::relations_map particles = s.getMCParticleRelations();
      SVDWaveform::relations_map truehits = s.getTrueHitRelations();

      // Save SVDShaperDigits
      // 1. Convert to ADU
      SVDShaperDigit::APVRawSamples rawSamples;
      std::transform(samples.begin(), samples.end(), rawSamples.begin(),
      [&](double x)->SVDShaperDigit::APVRawSampleType {
        return SVDShaperDigit::trimToSampleRange(x * gain);
      });

      // 2.a Check if over threshold
      auto rawThreshold = m_SNAdjacent * elNoise * gain;
      if (m_roundZS) rawThreshold = round(rawThreshold);
      auto n_over = std::count_if(rawSamples.begin(), rawSamples.end(),
                                  std::bind2nd(std::greater<double>(), rawThreshold)
                                 );
      if (n_over < m_nSamplesOverZS) continue;

      // 2.b check if the strip is masked
      if (m_MaskedStr.isMasked(sensorID, false, iStrip)) continue;

      // 2.c check if the APV is disabled
      if (!m_map->isAPVinMap(sensorID, false, iStrip)) continue;

      // 2.d.1 check if it's a 3-sample event
      if (m_is3sampleEvent) {
        rawSamples[0] = rawSamples[m_startingSample];
        rawSamples[1] = rawSamples[m_startingSample + 1];
        rawSamples[2] = rawSamples[m_startingSample + 2];
        rawSamples[3] = 0.;
        rawSamples[4] = 0.;
        rawSamples[5] = 0.;
        //2.d.2 check if still over threshold
        n_over = std::count_if(rawSamples.begin(), rawSamples.end(),
                               std::bind2nd(std::greater<double>(), rawThreshold)
                              );
        if (n_over < m_nSamplesOverZS) continue;
      }

      // 3. Save as a new digit
      int digIndex = storeShaperDigits.getEntries();
      storeShaperDigits.appendNew(sensorID, false, iStrip, rawSamples, 0);

      //If the digit has any relations to MCParticles, add the Relation
      if (particles.size() > 0) {
        relShaperDigitMCParticle.add(digIndex, particles.begin(), particles.end());
      }
      //If the digit has any relations to truehits, add the Relations.
      if (truehits.size() > 0) {
        relShaperDigitTrueHit.add(digIndex, truehits.begin(), truehits.end());
      }
    } // for stripSignals
  } // FOREACH sensor
}

void SVDDigitizerModule::saveWaveforms()
{
  for (Waveforms::value_type& sensorWaveforms : m_waveforms) {
    tree_vxdID = sensorWaveforms.first;
    const SensorInfo& info =
      dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorWaveforms.first));
    // u-side digits:
    tree_uv = 1;
    double thresholdU = 3.0 * info.getElectronicNoiseU();
    for (StripWaveforms::value_type& stripWaveform : sensorWaveforms.second.first) {
      tree_strip = stripWaveform.first;
      SVDWaveform& s = stripWaveform.second;
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
    for (StripWaveforms::value_type& stripWaveform : sensorWaveforms.second.second) {
      tree_strip = stripWaveform.first;
      SVDWaveform& s = stripWaveform.second;
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
  for (Waveforms::value_type& sensorWaveforms : m_waveforms) {
    VxdID sensorID(sensorWaveforms.first);
    const SensorInfo& info =
      dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorWaveforms.first));
    // u-side digits:
    size_t isU = 1;
    double thresholdU = 3.0 * info.getElectronicNoiseU();
    for (StripWaveforms::value_type& stripWaveform : sensorWaveforms.second.first) {
      size_t strip = stripWaveform.first;
      SVDWaveform& s = stripWaveform.second;
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
    for (StripWaveforms::value_type& stripWaveform : sensorWaveforms.second.second) {
      size_t strip = stripWaveform.first;
      SVDWaveform& s = stripWaveform.second;
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

int SVDDigitizerModule::getFirstSample(int triggerBin, int relativeShift)
{
  int nTriggerClocks = triggerBin + relativeShift;
  return floor(nTriggerClocks / 4);
}
