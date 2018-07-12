/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Benjamin Schwenker                        *
 *                                                                        *
 **************************************************************************/

#include <pxd/modules/pxdSimulation/PXDDigitizerModule.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/GeoTools.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>

#include <mdst/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <cmath>

#include <TRandom.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDDigitizerModule::PXDDigitizerModule() :
  Module()
{
  //Set module properties
  setDescription("Digitize PXDSimHits");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("ElectronicEffects", m_applyNoise, "Apply electronic effects?",
           true);
  addParam("ElectronicNoise", m_elNoise,
           "Noise added by the electronics, set in ENC", 150.0);

  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticle collection name", string(""));
  addParam("Digits", m_storeDigitsName, "Digits collection name", string(""));
  addParam("SimHits", m_storeSimHitsName, "SimHit collection name",
           string(""));
  addParam("TrueHits", m_storeTrueHitsName, "TrueHit collection name",
           string(""));

  addParam("PoissonSmearing", m_applyPoisson,
           "Apply Poisson smearing of electrons collected on pixels?", true);
  addParam("IntegrationWindow", m_applyWindow, "Use integration window?",
           true);

  addParam("SegmentLength", m_segmentLength, "Maximum segment length (in mm)",
           0.005);
  addParam("ElectronGroupSize", m_elGroupSize,
           "Split Signalpoints in smaller groups of N electrons (in e)", 100);
  addParam("ElectronStepTime", m_elStepTime,
           "Time step for tracking electron groups in readout plane (in ns)",
           1.0);
  addParam("ElectronMaxSteps", m_elMaxSteps,
           "Maximum number of steps when propagating electrons", 200);
  addParam("Gq", m_gq, "Gq of a pixel in nA/electron", 0.6);
  addParam("ADCUnit", m_ADCUnit, "Slope of the linear ADC transfer curve in nA/ADU", 130.0);
  addParam("PedestalMean", m_pedestalMean, "Mean of pedestals in ADU", 100.0);
  addParam("PedestalRMS", m_pedestalRMS, "RMS of pedestals in ADU", 30.0);
  addParam("GatingTime", m_gatingTime,
           "Time window during which the PXD is not collecting charge in nano seconds", 400.0);
  addParam("GatingWithoutReadout", m_gatingWithoutReadout,
           "Digits from gated rows not sent to DHH ", true);
  addParam("GatingWithoutReadoutTime", m_gatingWithoutReadoutTime,
           "Time window during which digits from gated rows are not sent to DHH in nano seconds", 1400.0);
  addParam("HardwareDelay", m_hwdelay,
           "Constant time delay between bunch crossing and switching on triggergate in nano seconds", 0.0);

}

void PXDDigitizerModule::initialize()
{
  //Register output collections
  StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  storeDigits.registerInDataStore();
  StoreArray<MCParticle> storeParticles(m_storeMCParticlesName);
  storeDigits.registerRelationTo(storeParticles);
  StoreArray<PXDTrueHit> storeTrueHits(m_storeTrueHitsName);
  storeDigits.registerRelationTo(storeTrueHits);

  m_storePXDTiming.isOptional();

  //Set default names for the relations
  m_relMCParticleSimHitName = DataStore::relationName(
                                DataStore::arrayName<MCParticle>(m_storeMCParticlesName),
                                DataStore::arrayName<PXDSimHit>(m_storeSimHitsName));
  m_relTrueHitSimHitName = DataStore::relationName(
                             DataStore::arrayName<PXDTrueHit>(m_storeTrueHitsName),
                             DataStore::arrayName<PXDSimHit>(m_storeSimHitsName));
  m_relDigitMCParticleName = DataStore::relationName(
                               DataStore::arrayName<PXDDigit>(m_storeDigitsName),
                               DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  m_relDigitTrueHitName = DataStore::relationName(
                            DataStore::arrayName<PXDDigit>(m_storeDigitsName),
                            DataStore::arrayName<PXDTrueHit>(m_storeTrueHitsName));

  //Convert parameters to correct units
  m_elNoise *= Unit::e;
  m_eToADU = m_ADCUnit / m_gq;
  m_elStepTime *= Unit::ns;
  m_segmentLength *= Unit::mm;

  // Get number of PXD gates
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  m_nGates = gTools->getNumberOfPXDReadoutGates();

  // Active integration time (per gate) in ns
  m_pxdIntegrationTime = 20000.0;
  // Readout time per gate (sample - clear - cycle of rolling shutter) in ns
  m_timePerGate = m_pxdIntegrationTime / m_nGates;

  B2DEBUG(20, "PXDDigitizer Parameters (in system units, *=calculated +=set in xml):");
  B2DEBUG(20, " -->  ElectronicEffects:  " << (m_applyNoise ? "true" : "false"));
  B2DEBUG(20, " -->  ElectronicNoise:    " << m_elNoise << " e-");
  B2DEBUG(20, " --> +ChargeThreshold:    " << "set in xml by sensor, nominal 4 ADU");
  B2DEBUG(20, " --> *NoiseFraction:      " << "set in xml by sensor, nominal 1.0e-5");
  B2DEBUG(20,
          " -->  MCParticles:        " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2DEBUG(20,
          " -->  SimHits:            " << DataStore::arrayName<PXDSimHit>(m_storeSimHitsName));
  B2DEBUG(20,
          " -->  Digits:             " << DataStore::arrayName<PXDDigit>(m_storeDigitsName));
  B2DEBUG(20,
          " -->  TrueHits:           " << DataStore::arrayName<PXDTrueHit>(m_storeTrueHitsName));
  B2DEBUG(20, " -->  MCSimHitRel:        " << m_relMCParticleSimHitName);
  B2DEBUG(20, " -->  DigitMCRel:         " << m_relDigitMCParticleName);
  B2DEBUG(20, " -->  TrueSimRel:         " << m_relTrueHitSimHitName);
  B2DEBUG(20, " -->  DigitTrueRel:       " << m_relDigitTrueHitName);
  B2DEBUG(20, " -->  PoissonSmearing:    " << (m_applyPoisson ? "true" : "false"));
  B2DEBUG(20, " --> +IntegrationWindow:  " << (m_applyWindow ? "true" : "false") << ", size defined in xml");
  B2DEBUG(20, " -->  SegmentLength:      " << m_segmentLength << " cm");
  B2DEBUG(20, " -->  ElectronGroupSize:  " << m_elGroupSize << " e-");
  B2DEBUG(20, " -->  ElectronStepTime:   " << m_elStepTime << " ns");
  B2DEBUG(20, " -->  ElectronMaxSteps:   " << m_elMaxSteps);
  B2DEBUG(20, " -->  ADU unit:           " << m_eToADU << " e-/ADU");
}

void PXDDigitizerModule::beginRun()
{
  //Fill map with all possible sensors This is too slow to be done every event so
  //we fill it once and only clear the content of the sensors per event, not
  //the whole map.
  // NOTE: Some VXDIDs will be added to the map on the way if we have multiple
  // frames, but they will stay and will be cleared appropriately, so this is not
  // a problem, and after a few events the performance will stabilize.
  m_sensors.clear();
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (VxdID layer : geo.getLayers(SensorInfo::PXD)) {
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        m_sensors[sensor] = Sensor();
      }
    }
  }
}

void PXDDigitizerModule::event()
{

  //Check for injection vetos
  if (m_storePXDTiming.isValid()) {
    m_gated = (*m_storePXDTiming).isGated();
    m_triggerGate = (*m_storePXDTiming).getTriggerGate();
    m_gatingStartTimes = (*m_storePXDTiming).getGatingStartTimes();
    m_gatedChannelIntervals.clear();

    B2DEBUG(20, "Reading from PXDInjectionBGTiming: Event has TriggerGate " << m_triggerGate << " and is gated " << m_gated);

    if (m_gatingWithoutReadout) {
      for (auto gatingStartTime : m_gatingStartTimes) {
        B2DEBUG(20, "Gating start time " << gatingStartTime << "ns ");

        // Gated readout can only come from gating in the time period from t=0us to t=20us.
        if (gatingStartTime >= 0) {
          // Compute the gate where gating started. Note the wrap around of the rolling shutter
          int firstGated = (m_triggerGate + int(gatingStartTime / m_timePerGate)) % m_nGates ;
          // Compute the gate where gating stopped. Note the wrap around of the rolling shutter
          int lastGated = (m_triggerGate + int((gatingStartTime + m_gatingWithoutReadoutTime) / m_timePerGate)) % m_nGates ;

          if (lastGated  >= firstGated) {
            // Gated channels in the same frame
            B2DEBUG(20, "Gated channel interval from " << firstGated << " to " << lastGated);
            m_gatedChannelIntervals.push_back(pair<int, int>(firstGated, lastGated));
          } else {
            // Gated channels in two consecutive frames
            B2DEBUG(20, "Gated channel interval from " << firstGated << " to " << m_nGates);
            B2DEBUG(20, "Gated channel interval from " << 0 << " to " << lastGated);
            m_gatedChannelIntervals.push_back(pair<int, int>(firstGated, m_nGates));
            m_gatedChannelIntervals.push_back(pair<int, int>(0, lastGated));
          }
        }
      }
    }
  } else {
    m_gated = false;
    m_triggerGate = gRandom->Integer(m_nGates);
    m_gatingStartTimes.clear();
    m_gatedChannelIntervals.clear();
  }

  //Clear sensors and process SimHits
  for (Sensors::value_type& sensor : m_sensors) {
    sensor.second.clear();
  }
  m_currentSensor = 0;
  m_currentSensorInfo = 0;
  //Clear return value
  setReturnValue(0);

  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<PXDSimHit> storeSimHits(m_storeSimHitsName);
  StoreArray<PXDTrueHit> storeTrueHits(m_storeTrueHitsName);

  RelationArray mcParticlesToSimHits(storeMCParticles, storeSimHits,
                                     m_relMCParticleSimHitName);
  RelationArray mcParticlesToTrueHits(storeMCParticles, storeTrueHits); // not used here
  RelationArray trueHitsToSimHits(storeTrueHits, storeSimHits,
                                  m_relTrueHitSimHitName);

  StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  storeDigits.clear();

  RelationArray relDigitMCParticle(storeDigits, storeMCParticles,
                                   m_relDigitMCParticleName);
  relDigitMCParticle.clear();

  RelationArray relDigitTrueHit(storeDigits, storeTrueHits,
                                m_relDigitTrueHitName);
  relDigitTrueHit.clear();

  unsigned int nSimHits = storeSimHits.getEntries();
  if (nSimHits == 0)
    return;

  RelationIndex<MCParticle, PXDSimHit> relMCParticleSimHit(storeMCParticles,
                                                           storeSimHits, m_relMCParticleSimHitName);
  RelationIndex<PXDTrueHit, PXDSimHit> relTrueHitSimHit(storeTrueHits,
                                                        storeSimHits, m_relTrueHitSimHitName);

  //Check sensor info and set pointers to current sensor
  for (unsigned int i = 0; i < nSimHits; ++i) {
    m_currentHit = storeSimHits[i];
    const RelationIndex<MCParticle, PXDSimHit>::Element* mcRel =
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
      // Don't warn if this is a background SimHit
      if (m_currentHit->getBackgroundTag() == SimHitBase::bg_none)
        B2WARNING(
          "Could not find MCParticle which produced PXDSimhit " << i);
      m_currentParticle = -1;
    }
    const RelationIndex<PXDTrueHit, PXDSimHit>::Element* trueRel =
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
          "SensorInformation for Sensor " << sensorID << " not found, make sure that the geometry is set up correctly");
      m_currentSensor = &m_sensors[sensorID];
      B2DEBUG(20, "Sensor Parameters for Sensor " << sensorID << ": " << endl
              << " --> Width:        " << m_currentSensorInfo->getWidth() << endl
              << " --> Length:       " << m_currentSensorInfo->getLength() << endl
              << " --> uPitch:       " << m_currentSensorInfo->getUPitch() << endl
              << " --> vPitch        " << m_currentSensorInfo->getVPitch(-m_currentSensorInfo->getLength() / 2.0)
              << ", " << m_currentSensorInfo->getVPitch(m_currentSensorInfo->getLength() / 2.0) << endl
              << " --> Thickness:    " << m_currentSensorInfo->getThickness() << endl
              << " --> BulkDoping:   " << m_currentSensorInfo->getBulkDoping() << endl
              << " --> BackVoltage:  " << m_currentSensorInfo->getBackVoltage() << endl
              << " --> TopVoltage:   " << m_currentSensorInfo->getTopVoltage() << endl
              << " --> SourceBorder: " << m_currentSensorInfo->getSourceBorder(-0.4 * m_currentSensorInfo->getLength())
              << ", "                  << m_currentSensorInfo->getSourceBorder(+0.4 * m_currentSensorInfo->getLength()) << endl
              << " --> ClearBorder: " << m_currentSensorInfo->getClearBorder(-0.4 * m_currentSensorInfo->getLength())
              << ", "                  << m_currentSensorInfo->getClearBorder(+0.4 * m_currentSensorInfo->getLength()) << endl
              << " --> DrainBorder: " << m_currentSensorInfo->getDrainBorder(-0.4 * m_currentSensorInfo->getLength())
              << ", "                  << m_currentSensorInfo->getDrainBorder(+0.4 * m_currentSensorInfo->getLength()) << endl
              << " --> GateDepth:    " << m_currentSensorInfo->getGateDepth() << endl
              << " --> DoublePixel:  " << m_currentSensorInfo->getDoublePixel() << endl);

    }
    B2DEBUG(10,
            "Processing hit " << i << " in Sensor " << sensorID << ", related to MCParticle " << m_currentParticle);
    processHit();
  }

  addNoiseDigits();
  saveDigits();
  //Return number of created digits
  setReturnValue(storeDigits.getEntries());
}

void PXDDigitizerModule::processHit()
{
  if (m_applyWindow) {
    //Ignore a hit which is outside of the active frame time of the hit gate
    float currentHitTime = m_currentHit->getGlobalTime();

    // First we have to now the current hit gate
    const TVector3 startPoint = m_currentHit->getPosIn();
    const TVector3 stopPoint = m_currentHit->getPosOut();
    auto row = m_currentSensorInfo->getVCellID(0.5 * (stopPoint.y() + startPoint.y()));

    if (m_currentSensorInfo->getID().getLayerNumber() == 1)
      row  = 767 - row;
    int gate = row / 4;

    // Compute the distance in unit of gates to the trigger gate. Do it in
    // direction of PXD rolling shutter.
    int distanceToTriggerGate = 0;
    if (gate >= m_triggerGate) distanceToTriggerGate = gate - m_triggerGate;
    else distanceToTriggerGate = 192 - m_triggerGate + gate;

    // The triggergate is switched ON at t0=0ns. Compute the integration time window for the current hit gate.
    // Time intervals of subsequent readout gates are shifted by timePerGate.
    float gateMinTime = -m_pxdIntegrationTime + m_timePerGate + m_hwdelay + distanceToTriggerGate * m_timePerGate ;
    float gateMaxTime = gateMinTime + m_pxdIntegrationTime;

    B2DEBUG(30,
            "Checking if hit is in timeframe " << gateMinTime << " <= " << currentHitTime <<
            " <= " << gateMaxTime);

    if (currentHitTime
        <  gateMinTime
        || currentHitTime
        > gateMaxTime)
      return;

    if (m_gated) {
      //Ignore simHits when the PXD is gated, i.e. PXD does not collect any charge.
      for (auto gatingStartTime : m_gatingStartTimes) {
        B2DEBUG(30,
                "Checking if hit is in gated timeframe " << gatingStartTime << " <= " << currentHitTime <<
                " <= " << gatingStartTime + m_gatingTime);

        if (currentHitTime
            > gatingStartTime
            && currentHitTime
            < gatingStartTime + m_gatingTime)
          return;
      }
    }
  }

  //Get step length and direction
  const TVector3 startPoint = m_currentHit->getPosIn();
  TVector3 stopPoint = m_currentHit->getPosOut();
  double dx = stopPoint.x() - startPoint.x();
  double dy = stopPoint.y() - startPoint.y();
  double dz = stopPoint.z() - startPoint.z();
  double trackLength2 = dx * dx + dy * dy + dz * dz;

  // Set magnetic field to save calls to getBField()
  m_currentBField = m_currentSensorInfo->getBField(0.5 * (startPoint + stopPoint));

  if (m_currentHit->getPDGcode() == 22 || trackLength2 <= 0.01 * Unit::um * Unit::um) {
    //Photons deposit the energy at the end of their step
    driftCharge(stopPoint, m_currentHit->getElectrons());
  } else {
    //Otherwise, split into segments of (default) max. 5µm and
    //drift the charges from the center of each segment
    auto segments = m_currentHit->getElectronsConstantDistance(m_segmentLength);
    double lastFraction {0};
    double lastElectrons {0};

    for (auto& segment : segments) {
      //Simhit returns step fraction and cumulative electrons. We want the
      //center of these steps and electrons in this step
      const double f = (segment.first + lastFraction) * 0.5;
      const double e = segment.second - lastElectrons;
      //Update last values
      std::tie(lastFraction, lastElectrons) = segment;

      //And drift charge from that position
      stopPoint.SetXYZ(startPoint.x() + f * dx, startPoint.y() + f * dy, startPoint.z() + f * dz);
      driftCharge(stopPoint, e);
    }
  }
}

inline double pol3(double x, const double* c)
{
  return c[0] + x * (c[1] + x * (c[2] + x * c[3]));
};

void PXDDigitizerModule::driftCharge(const TVector3& r, double electrons)
{
  //Get references to current sensor/info for ease of use
  const SensorInfo& info = *m_currentSensorInfo;
  Sensor& sensor = *m_currentSensor;

  static const double cx[] = {0, 2.611995e-01, -1.948316e+01, 9.167064e+02};
  static const double cu[] = {4.223817e-04, -1.041434e-03, 5.139596e-02, -1.180229e+00};
  static const double cv[] = {4.085681e-04, -8.615459e-04, 5.706439e-02, -1.774319e+00};
  static const double ct[] = {2.823743e-04, -1.138627e-06, 4.310888e-09, -1.559542e-11}; // temp range [250, 350] degrees with m_elStepTime = 1 ns

  double dz = 0.5 * info.getThickness() - info.getGateDepth() - r.Z(), adz = fabs(dz);

  double dx = fabs(m_currentBField.y()) > Unit::T ? copysign(pol3(adz, cx),
                                                             dz) : 0; // two configurations: with default B field (B~1.5T) and B=0T
  double sigmaDrift_u = pol3(adz, cu);
  double sigmaDrift_v = pol3(adz, cv);
  double sigmaDiffus = pol3(info.getTemperature() - 300.0,
                            ct); // sqrt(2 * Const::uTherm * info.getElectronMobility(0) * m_elStepTime);

  int nGroups = (int)(electrons / m_elGroupSize) + 1;
  double groupCharge = electrons / nGroups;
  while (nGroups--) {
    double du = gRandom->Gaus(0.0, sigmaDrift_u), dv = gRandom->Gaus(0.0, sigmaDrift_v);
    double uPos = r.x() + du + dx;
    double vPos = r.y() + dv;
    int step = 0;
    for (; step < m_elMaxSteps; ++step) {
      int id = info.getTrappedID(uPos, vPos);
      //Check if cloud inside of IG region
      if (id >= 0) {
        // Trapped in the internal gate region
        sensor[Digit(id % 250, id / 250)].add(groupCharge, m_currentParticle, m_currentTrueHit);
        break;
      }
      //Random walk with drift
      dv = gRandom->Gaus(0.0, sigmaDiffus), du = gRandom->Gaus(0.0, sigmaDiffus);
      uPos += du;
      vPos += dv;
    }
    if (step == m_elMaxSteps) {
      // cloud is not trapped but save it anyway into the closest pixel
      int iu = info.getUCellID(uPos, vPos, 1), iv = info.getVCellID(vPos, 1);
      sensor[Digit(iu, iv)].add(groupCharge, m_currentParticle, m_currentTrueHit);
    }
  }
}


double PXDDigitizerModule::addNoise(double charge)
{
  if (charge <= 0) {
    //Noise Pixel, add chargeThreshold electrons;
    charge = 1.1 * m_chargeThresholdElectrons;
  } else {
    if (m_applyPoisson) {
      // For big charge assume Gaussian distr.
      if (charge > (1000. * Unit::e))
        charge = gRandom->Gaus(charge, sqrt(charge));
      else
        // Otherwise Poisson distr.
        charge = gRandom->Poisson(charge);
    }
    if (m_applyNoise) {
      charge += gRandom->Gaus(0., m_elNoise);
    }
  }
  return charge;
}

void PXDDigitizerModule::addNoiseDigits()
{
  if (!m_applyNoise)
    return;

  for (Sensors::value_type& sensor : m_sensors) {
    Sensor& s = sensor.second;

    //Calculate the number of pixels on an empty sensor which will exceed the noise cut
    const SensorInfo& info =
      dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensor.first));
    int nU = info.getUCells();
    int nV = info.getVCells();
    int nPixels = gRandom->Poisson(info.getNoiseFraction() * nU * nV);

    // Existing digits will get noise in PXDDIgitizer::AddNoise.
    // Here we add zero charge to nPixels digits.
    // In part, this will create some new zero-charge digits.
    // If we happen to select an existing (fired) digit, it remains unchanged.
    for (int i = 0; i < nPixels; ++i) {
      Digit d(gRandom->Integer(nU), gRandom->Integer(nV));
      s[d].add(0.0);
    }
  }
}

bool PXDDigitizerModule::checkIfGated(int gate)
{
  for (auto interval : m_gatedChannelIntervals) {
    if (gate >= interval.first && gate < interval.second) return true;
  }
  return false;
}

void PXDDigitizerModule::saveDigits()
{
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<PXDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  RelationArray relDigitMCParticle(storeDigits, storeMCParticles,
                                   m_relDigitMCParticleName);
  RelationArray relDigitTrueHit(storeDigits, storeTrueHits,
                                m_relDigitTrueHitName);


  for (Sensors::value_type& sensor : m_sensors) {
    auto sensorID = sensor.first;
    const SensorInfo& info =
      dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));
    m_chargeThreshold = info.getChargeThreshold();
    m_chargeThresholdElectrons = m_chargeThreshold * m_eToADU;
    for (Sensor::value_type& digitAndValue : sensor.second) {
      const Digit& d = digitAndValue.first;
      const DigitValue& v = digitAndValue.second;

      //Add Noise where applicable
      double charge = addNoise(v.charge());

      // Draw a pedestal value
      double pedestal = std::max(gRandom->Gaus(m_pedestalMean, m_pedestalRMS), 0.0);

      // Add pedestal to charge
      charge = round(charge / m_eToADU + pedestal);

      // Clipping of ADC codes at 255
      charge = std::min(charge, 255.0);

      // Subtraction of integer pedestal in DHP
      charge = charge - round(pedestal);

      // Zero Suppression in DHP
      if (charge < m_chargeThreshold)
        continue;

      // Check if the readout digits is coming from a gated row
      if (m_gatingWithoutReadout) {
        int row = d.v();
        if (sensorID.getLayerNumber() == 1)
          row  = 767 - row;
        int gate = row / 4;
        if (checkIfGated(gate)) continue;
      }

      //Add the digit to datastore
      int digIndex = storeDigits.getEntries();
      storeDigits.appendNew(
        PXDDigit(sensorID, d.u(), d.v(), charge));

      //If the digit has any relations to MCParticles, add the Relation
      if (v.particles().size() > 0) {
        relDigitMCParticle.add(digIndex, v.particles().begin(),
                               v.particles().end());
      }
      //If the digit has any truehits to TrueHit, add the Relation
      if (v.truehits().size() > 0) {
        relDigitTrueHit.add(digIndex, v.truehits().begin(),
                            v.truehits().end());
      }
    }
  }
}


