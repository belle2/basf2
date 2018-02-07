/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#include <testbeam/vxd/modules/telSimulation/TelDigitizerModule.h>
#include <vxd/geometry/GeoCache.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>

#include <mdst/dataobjects/MCParticle.h>
#include <testbeam/vxd/dataobjects/TelTrueHit.h>
#include <testbeam/vxd/dataobjects/TelDigit.h>
#include <cmath>

#include <root/TRandom.h>
#include <root/TMath.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::TEL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TelDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TelDigitizerModule::TelDigitizerModule() : Module(),
  m_elNoise(300.0), m_SNThreshold(5), m_noiseFraction(0), m_currentHit(0),
  m_currentParticle(0), m_currentTrueHit(0), m_currentSensor(0),
  m_currentSensorInfo(0)
{
  //Set module properties
  setDescription("Digitize TelSimHits");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("ElectronicNoise", m_elNoise,
           "Noise added by the electronics, set in ENC", 300.0);
  addParam("NoiseSN", m_SNThreshold,
           "SN for digits to be considered for clustering", 5.0);
  addParam("SegmentLength", m_segmentLength, "Maximum segment length (in mm)",
           0.01);
  addParam("Diffusion", m_diffusionCoefficient,
           "Diffusion coefficient (in mm)", double(0.00008617));
  addParam("widthOfDiffusCloud", m_widthOfDiffusCloud,
           "Integration range of the e- cloud in sigmas", double(5.0));
  addParam("tanLorentz", m_tanLorentz, "Tangent of the Lorentz angle",
           double(0.25));
}

void TelDigitizerModule::initialize()
{
  //Register collections
  StoreArray<TelDigit> storeDigits("");
  StoreArray<MCParticle> storeMCParticles("");
  StoreArray<TelSimHit> storeSimHits("");
  StoreArray<TelTrueHit> storeTrueHits("");

  storeDigits.registerInDataStore();
  /* This always makes problems.
  storeMCParticles.isRequired();
  storeSimHits.isRequired();
  storeTrueHits.isRequired();
  */

  RelationArray relDigitsToMCParticles(storeDigits, storeMCParticles);
  RelationArray relDigitsToTrueHits(storeDigits, storeTrueHits);
  RelationArray relTrueHitsToSimHits(storeTrueHits, storeSimHits);
  RelationArray relMCParticlesToSimHits(storeMCParticles, storeSimHits);

  relDigitsToMCParticles.registerInDataStore();
  relDigitsToTrueHits.registerInDataStore();

  // Save names for easy retrieval
  m_storeMCParticlesName = storeMCParticles.getName();
  m_storeSimHitsName = storeSimHits.getName();
  m_storeTrueHitsName = storeTrueHits.getName();
  m_storeDigitsName = storeDigits.getName();

  m_relTrueHitSimHitName = relTrueHitsToSimHits.getName();
  m_relDigitMCParticleName = relDigitsToMCParticles.getName();
  m_relDigitTrueHitName = relDigitsToTrueHits.getName();
  m_relMCParticleSimHitName = relMCParticlesToSimHits.getName();

  //Convert parameters to correct units
  m_elNoise *= Unit::e;
  m_segmentLength *= Unit::mm;
  m_diffusionCoefficient *= Unit::mm;
  m_noiseFraction = TMath::Freq(m_SNThreshold);

  B2INFO(
    "TelDigitizer Parameters (in default system units, *=cannot be set directly):");
  B2INFO(" -->  ElectronicNoise:    " << m_elNoise);
  B2INFO(" -->  NoiseSN:            " << m_SNThreshold);
  B2INFO(" --> *NoiseFraction:      " << m_noiseFraction);
  B2INFO(
    " -->  MCParticles:        " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2INFO(
    " -->  SimHits:            " << DataStore::arrayName<TelSimHit>(m_storeSimHitsName));
  B2INFO(
    " -->  Digits:             " << DataStore::arrayName<TelDigit>(m_storeDigitsName));
  B2INFO(
    " -->  TrueHits:           " << DataStore::arrayName<TelTrueHit>(m_storeTrueHitsName));
  B2INFO(" -->  MCSimHitRel:        " << m_relMCParticleSimHitName);
  B2INFO(" -->  DigitMCRel:         " << m_relDigitMCParticleName);
  B2INFO(" -->  TrueSimRel:         " << m_relTrueHitSimHitName);
  B2INFO(" -->  DigitTrueRel:       " << m_relDigitTrueHitName);
  B2INFO(" -->  SegmentLength:      " << m_segmentLength);
  B2INFO(" -->  Diffusion:          " << m_diffusionCoefficient);
  B2INFO(" -->  widthOfDiffusCloud: " << m_widthOfDiffusCloud);
}

void TelDigitizerModule::beginRun()
{
  //Fill map with all possible sensors This is too slow to be done every event so
  //we fill it once and only clear the content of the sensors per event, not
  //the whole map
  m_sensors.clear();
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (VxdID layer : geo.getLayers(SensorInfo::TEL)) {
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        m_sensors[sensor] = Sensor();
      }
    }
  }
}

void TelDigitizerModule::event()
{
  //Clear sensors and process SimHits
  for (Sensors::value_type& sensor : m_sensors) {
    sensor.second.clear();
  }
  m_currentSensor = 0;
  m_currentSensorInfo = 0;

  const StoreArray<TelSimHit> storeSimHits(m_storeSimHitsName);
  unsigned int nSimHits = storeSimHits.getEntries();
  if (nSimHits == 0)
    return;

  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<TelTrueHit> storeTrueHits(m_storeTrueHitsName);

  StoreArray<TelDigit> storeDigits(m_storeDigitsName);
  storeDigits.clear();

  RelationArray relDigitMCParticle(storeDigits, storeMCParticles,
                                   m_relDigitMCParticleName);
  if (relDigitMCParticle) relDigitMCParticle.clear();

  RelationArray relDigitTrueHit(storeDigits, storeTrueHits,
                                m_relDigitTrueHitName);
  if (relDigitTrueHit) relDigitTrueHit.clear();

  RelationIndex<MCParticle, TelSimHit> relMCParticleSimHit(storeMCParticles,
                                                           storeSimHits, m_relMCParticleSimHitName);
  RelationIndex<TelTrueHit, TelSimHit> relTrueHitSimHit(storeTrueHits,
                                                        storeSimHits, m_relTrueHitSimHitName);

  //Check sensor info and set pointers to current sensor
  for (unsigned int i = 0; i < nSimHits; ++i) {
    m_currentHit = storeSimHits[i];
    const RelationIndex<MCParticle, TelSimHit>::Element* mcRel =
      relMCParticleSimHit.getFirstElementTo(m_currentHit);
    if (mcRel) {
      m_currentParticle = mcRel->indexFrom;
    } else {
      B2WARNING(
        "Could not find MCParticle which produced TelSimhit " << i);
      m_currentParticle = -1;
    }
    const RelationIndex<TelTrueHit, TelSimHit>::Element* trueRel =
      relTrueHitSimHit.getFirstElementTo(m_currentHit);
    if (trueRel) {
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
          "SensorInformation for Sensor " << sensorID << " not found, make sure that the geometry is set up correctly");
      m_currentSensor = &m_sensors[sensorID];
      B2DEBUG(20,
              "Sensor Parameters for Sensor " << sensorID << ": " << endl
              << " --> Width:        " << m_currentSensorInfo->getWidth() << endl
              << " --> Length:       " << m_currentSensorInfo->getLength() << endl
              << " --> uPitch:       " << m_currentSensorInfo->getUPitch() << endl
              << " --> vPitch:       " << m_currentSensorInfo->getVPitch() << endl
              << " --> Thickness:    " << m_currentSensorInfo->getThickness() << endl
             );
    }
    B2DEBUG(10,
            "Processing hit " << i << " in Sensor " << sensorID << ", related to MCParticle " << m_currentParticle);
    processHit();
  }

  addNoiseDigits();
  saveDigits();
}

void TelDigitizerModule::processHit()
{
  //Get Steplength and direction
  const TVector3& startPoint = m_currentHit->getPosIn();
  const TVector3& stopPoint = m_currentHit->getPosOut();
  TVector3 direction = stopPoint - startPoint;
  double trackLength = direction.Mag();

  // Set magnetic field to save calls to getBField() NOT USED currently
  m_currentBField = m_currentSensorInfo->getBField(0.5 * (startPoint + stopPoint));

  if (m_currentHit->getPDGcode() == 22 || trackLength <= 0.1 * Unit::um) {
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


void TelDigitizerModule::driftCharge(const TVector3& position,
                                     double electrons)
{
  //Get references to current sensor/info for ease of use
  const SensorInfo& info = *m_currentSensorInfo;
  Sensor& sensor = *m_currentSensor;

  double sensorThickness = info.getThickness();
  //Drift to module surface
  double distanceToPlane = 0.5 * sensorThickness - position.Z();
  TVector3 final = position
                   + TVector3(m_tanLorentz * distanceToPlane, 0, distanceToPlane);
  double driftLength = (final - position).Mag();

  //Calculate diffusion of electron cloud
  double invCosLorentzAngle = sqrt(1.0 + m_tanLorentz * m_tanLorentz);
  double sigmaDiffus = sqrt(driftLength * m_diffusionCoefficient);

  //Calculate the size of the diffusion cloud and all pixel IDs this will hit
  double uCenter = final.X();
  double vCenter = final.Y();
  double uSigma = sigmaDiffus * invCosLorentzAngle;
  double vSigma = sigmaDiffus;
  double uLow = uCenter - m_widthOfDiffusCloud * uSigma;
  double uHigh = uCenter + m_widthOfDiffusCloud * uSigma;
  double vLow = vCenter - m_widthOfDiffusCloud * vSigma;
  double vHigh = vCenter + m_widthOfDiffusCloud * vSigma;
  int uIDLow = info.getUCellID(uLow, 0, true);
  int uIDHigh = info.getUCellID(uHigh, 0, true);
  int vIDLow = info.getVCellID(vLow, true);
  int vIDHigh = info.getVCellID(vHigh, true);
  B2DEBUG(30, "Size of diffusion cloud: " << uSigma << ", " << vSigma);
  B2DEBUG(30,
          "uID from " << uIDLow << " to " << uIDHigh << ", vID from " << vIDLow << " to " << vIDHigh);

#define NORMAL_CDF(mean,sigma,x) TMath::Freq(((x)-(mean))/(sigma))

  //Now loop over all pixels and calculate the integral of the 2D gaussian charge distribution
  //Deposit the charge corresponding to the per pixel integral in each pixel
  double fraction(0);
  double vLowerTail =
    NORMAL_CDF(vCenter, vSigma, info.getVCellPosition(vIDLow) - 0.5 * info.getVPitch(vLow));
  for (int vID = vIDLow; vID <= vIDHigh; ++vID) {
    double vPos = info.getVCellPosition(vID);
    double vUpperTail =
      NORMAL_CDF(vCenter, vSigma, vPos + 0.5 * info.getVPitch(vPos));
    double vIntegral = vUpperTail - vLowerTail;
    vLowerTail = vUpperTail;

    double uLowerTail =
      NORMAL_CDF(uCenter, uSigma, info.getUCellPosition(uIDLow) - 0.5 * info.getUPitch());
    for (int uID = uIDLow; uID <= uIDHigh; ++uID) {
      double uPos = info.getUCellPosition(uID);
      double uUpperTail =
        NORMAL_CDF(uCenter, uSigma, uPos + 0.5 * info.getUPitch());
      double uIntegral = uUpperTail - uLowerTail;
      uLowerTail = uUpperTail;

      double charge = electrons * uIntegral * vIntegral;
      sensor[Digit(uID, vID)].add(charge, m_currentParticle,
                                  m_currentTrueHit);
      B2DEBUG(80,
              "Relative charge for pixel (" << uID << ", " << vID << "): " << uIntegral * vIntegral);
      fraction += uIntegral * vIntegral;
    }
    B2DEBUG(30, "Fraction of charge: " << fraction);
  }

#undef NORMAL_CDF

}

double TelDigitizerModule::addNoise(double charge)
{
  if (charge <= 0) {
    //Noise Pixel, add noise to exceed Noise Threshold;
    double p = gRandom->Uniform(m_noiseFraction, 1.0);
    charge = TMath::NormQuantile(p) * m_elNoise;
  } else { // Poisson
    charge = gRandom->Gaus(charge, sqrt(charge));
  }
  charge += gRandom->Gaus(0., m_elNoise);
  return charge;
}

void TelDigitizerModule::addNoiseDigits()
{
  double fraction = 1 - m_noiseFraction;
  for (Sensors::value_type& sensor : m_sensors) {
    Sensor& s = sensor.second;

    //Calculate the number of pixels on an empty sensor which will exceed the noise cut
    const SensorInfo& info =
      dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensor.first));
    int nU = info.getUCells();
    int nV = info.getVCells();
    int nPixels = gRandom->Poisson(fraction * nU * nV);

    //With an empty Sensor, nPixels would exceed the noise cut. If pixels are lit, these will have their own
    //noise fluctuation in addNoise. So if we find that a pixel we chose randomly to be lit already carry charge,
    //we ignore it.
    for (int i = 0; i < nPixels; ++i) {
      Digit d(gRandom->Integer(nU), gRandom->Integer(nV));
      //Add 0 electrons, will not modify existing digits but will add empty ones which
      //will be filled with noise in TelDigitizer::addNoise
      s[d].add(0.0);
    }
  }
}

void TelDigitizerModule::saveDigits()
{
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<TelTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<TelDigit> storeDigits(m_storeDigitsName);
  RelationArray relDigitMCParticle(storeDigits, storeMCParticles,
                                   m_relDigitMCParticleName);
  RelationArray relDigitTrueHit(storeDigits, storeTrueHits,
                                m_relDigitTrueHitName);

  //Zero supression cut in electrons
  double charge_threshold = m_SNThreshold * m_elNoise;

  for (Sensors::value_type& sensor : m_sensors) {
    int sensorID = sensor.first;
    for (Sensor::value_type& digit : sensor.second) {
      const Digit& d = digit.first;
      const DigitValue& v = digit.second;

      //Add Noise where applicable
      double charge = addNoise(v.charge());
      //Binary: forget about charge from now on
      if (charge < charge_threshold)
        continue;

      //Add the digit to datastore
      int digIndex = storeDigits.getEntries();
      storeDigits.appendNew(
        TelDigit(sensorID, d.u(), d.v()));

      //If the digit has any relations to MCParticles, add the Relation
      if (v.particles().size() > 0) {
        relDigitMCParticle.add(digIndex, v.particles().begin(),
                               v.particles().end());
      }
      //If the digit has any relations to TrueHits, add the Relation
      if (v.truehits().size() > 0) {
        relDigitTrueHit.add(digIndex, v.truehits().begin(),
                            v.truehits().end());
      }
    }
  }
}

void TelDigitizerModule::terminate()
{
}
