/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vtx/modules/vtxSimulation/VTXDigitizerModule.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/GeoTools.h>
#include <pxd/reconstruction/PXDGainCalibrator.h>
#include <pxd/reconstruction/PXDPixelMasker.h>
#include <vtx/geometry/SensorInfo.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>

#include <mdst/dataobjects/MCParticle.h>
#include <vtx/dataobjects/VTXTrueHit.h>
#include <vtx/dataobjects/VTXDigit.h>
#include <cmath>

#include <TRandom.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::VTX;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(VTXDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

VTXDigitizerModule::VTXDigitizerModule() :
  Module()
{
  //Set module properties
  setDescription("Digitize VTXSimHits");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("MCParticles", m_storeMCParticlesName, "MCParticle collection name", string(""));
  addParam("Digits", m_storeDigitsName, "Digits collection name", string(""));
  addParam("SimHits", m_storeSimHitsName, "SimHit collection name", string(""));
  addParam("TrueHits", m_storeTrueHitsName, "TrueHit collection name", string(""));

  m_currentSensor = nullptr;
}

void VTXDigitizerModule::initialize()
{
  //Register output collections
  StoreArray<VTXDigit> storeDigits(m_storeDigitsName);
  storeDigits.registerInDataStore();
  StoreArray<MCParticle> storeParticles(m_storeMCParticlesName);
  storeDigits.registerRelationTo(storeParticles);
  StoreArray<VTXTrueHit> storeTrueHits(m_storeTrueHitsName);
  storeDigits.registerRelationTo(storeTrueHits);


  //Set default names for the relations
  m_relMCParticleSimHitName = DataStore::relationName(
                                DataStore::arrayName<MCParticle>(m_storeMCParticlesName),
                                DataStore::arrayName<VTXSimHit>(m_storeSimHitsName));
  m_relTrueHitSimHitName = DataStore::relationName(
                             DataStore::arrayName<VTXTrueHit>(m_storeTrueHitsName),
                             DataStore::arrayName<VTXSimHit>(m_storeSimHitsName));
  m_relDigitMCParticleName = DataStore::relationName(
                               DataStore::arrayName<VTXDigit>(m_storeDigitsName),
                               DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  m_relDigitTrueHitName = DataStore::relationName(
                            DataStore::arrayName<VTXDigit>(m_storeDigitsName),
                            DataStore::arrayName<VTXTrueHit>(m_storeTrueHitsName));

  B2DEBUG(20, "VTXDigitizer Parameters (in system units, *=calculated +=set in xml):");
  B2DEBUG(20,
          " -->  MCParticles:        " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2DEBUG(20,
          " -->  SimHits:            " << DataStore::arrayName<VTXSimHit>(m_storeSimHitsName));
  B2DEBUG(20,
          " -->  Digits:             " << DataStore::arrayName<VTXDigit>(m_storeDigitsName));
  B2DEBUG(20,
          " -->  TrueHits:           " << DataStore::arrayName<VTXTrueHit>(m_storeTrueHitsName));
  B2DEBUG(20, " -->  MCSimHitRel:        " << m_relMCParticleSimHitName);
  B2DEBUG(20, " -->  DigitMCRel:         " << m_relDigitMCParticleName);
  B2DEBUG(20, " -->  TrueSimRel:         " << m_relTrueHitSimHitName);
  B2DEBUG(20, " -->  DigitTrueRel:       " << m_relDigitTrueHitName);
}

void VTXDigitizerModule::beginRun()
{
  //Fill map with all possible sensors This is too slow to be done every event so
  //we fill it once and only clear the content of the sensors per event, not
  //the whole map.
  // NOTE: Some VXDIDs will be added to the map on the way if we have multiple
  // frames, but they will stay and will be cleared appropriately, so this is not
  // a problem, and after a few events the performance will stabilize.
  m_sensors.clear();
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (VxdID layer : geo.getLayers(SensorInfo::VTX)) {
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        m_sensors[sensor] = Sensor();
      }
    }
  }
}

void VTXDigitizerModule::event()
{

  //Clear sensors and process SimHits
  for (Sensors::value_type& sensor : m_sensors) {
    sensor.second.clear();
  }
  m_currentSensor = 0;
  m_currentSensorInfo = 0;
  //Clear return value
  setReturnValue(0);

  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<VTXSimHit> storeSimHits(m_storeSimHitsName);
  StoreArray<VTXTrueHit> storeTrueHits(m_storeTrueHitsName);

  RelationArray mcParticlesToSimHits(storeMCParticles, storeSimHits,
                                     m_relMCParticleSimHitName);
  RelationArray mcParticlesToTrueHits(storeMCParticles, storeTrueHits); // not used here
  RelationArray trueHitsToSimHits(storeTrueHits, storeSimHits,
                                  m_relTrueHitSimHitName);

  StoreArray<VTXDigit> storeDigits(m_storeDigitsName);
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

  RelationIndex<MCParticle, VTXSimHit> relMCParticleSimHit(storeMCParticles,
                                                           storeSimHits, m_relMCParticleSimHitName);
  RelationIndex<VTXTrueHit, VTXSimHit> relTrueHitSimHit(storeTrueHits,
                                                        storeSimHits, m_relTrueHitSimHitName);

  //Check sensor info and set pointers to current sensor
  for (unsigned int i = 0; i < nSimHits; ++i) {
    m_currentHit = storeSimHits[i];
    const RelationIndex<MCParticle, VTXSimHit>::Element* mcRel =
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
      if (m_currentHit->getBackgroundTag() == BackgroundMetaData::bg_none)
        B2WARNING(
          "Could not find MCParticle which produced VTXSimhit " << i);
      m_currentParticle = -1;
    }
    const RelationIndex<VTXTrueHit, VTXSimHit>::Element* trueRel =
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
              << " --> ApplyElectronicEffects: " << (m_currentSensorInfo->getApplyElectronicEffects() ? "true" : "false") << endl
              << " --> ElectronicNoise:        " << m_currentSensorInfo->getElectronicNoise() << endl
              << " --> ApplyBinaryReadout:     " << (m_currentSensorInfo->getApplyBinaryReadout() ? "true" : "false") << endl
              << " --> BinaryHitThreshold:     " << m_currentSensorInfo->getBinaryHitThreshold()  << endl
              << " --> ElectronToADU:          " << m_currentSensorInfo->getElectronToADU() << endl
              << " --> MaxADUCode:             " << m_currentSensorInfo->getMaxADUCode()  << endl
              << " --> ApplyPoissonSmearing:   " << (m_currentSensorInfo->getApplyPoissonSmearing() ? "true" : "false") << endl
              << " --> ApplyIntegrationWindow: " << (m_currentSensorInfo->getApplyIntegrationWindow() ? "true" : "false") << endl
              << " --> SegmentLength:          " << m_currentSensorInfo->getSegmentLength() << endl
              << " --> ElectronGroupSize:      " << m_currentSensorInfo->getElectronGroupSize() << endl
              << " --> ElectronStepTime:       " << m_currentSensorInfo->getElectronStepTime()  << endl
              << " --> ElectronMaxSteps:       " << m_currentSensorInfo->getElectronMaxSteps()  << endl
              << " --> HardwareDelay:          " << m_currentSensorInfo->getHardwareDelay() << endl
              << " --> ADCunit:                " << m_currentSensorInfo->getADCunit() << endl
              << " --> CloudSize:              " << m_currentSensorInfo->getCloudSize() << endl
              << " --> Gq:                     " << m_currentSensorInfo->getGq() << endl
              << " --> X0average:              " << m_currentSensorInfo->getX0average() << endl
              << " --> TanLorentzAngle:        " << m_currentSensorInfo->getTanLorentzAngle() << endl
              << " --> Integration time:       " << m_currentSensorInfo->getIntegrationEnd() - m_currentSensorInfo->getIntegrationStart() <<
              endl);
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

void VTXDigitizerModule::processHit()
{
  const SensorInfo& info = *m_currentSensorInfo;
  if (info.getApplyIntegrationWindow()) {
    //Ignore a hit which is outside of the active time
    float currentHitTime = m_currentHit->getGlobalTime();
    double m_vtxIntegrationTime = info.getIntegrationEnd() - info.getIntegrationStart();
    if (currentHitTime
        <  -0.5 * m_vtxIntegrationTime
        || currentHitTime
        > 0.5 * m_vtxIntegrationTime)
      return;
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
    auto segments = m_currentHit->getElectronsConstantDistance(info.getSegmentLength());
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

void VTXDigitizerModule::driftCharge(const TVector3& r, double electrons)
{

  //Get references to current sensor/info for ease of use
  const SensorInfo& info = *m_currentSensorInfo;
  Sensor& sensor = *m_currentSensor;

  double distAlongEfield = r.z() + (info.getThickness() / 2);
  // Two configurations: with default B field (B~1.5T) and B=0T
  double lorentzShift = fabs(m_currentBField.y()) > Unit::T ? info.getTanLorentzAngle() * distAlongEfield : 0;
  double sigmaDiffus = sqrt(info.getCloudSize() *  distAlongEfield);

  int nGroups = (int)(electrons / info.getElectronGroupSize()) + 1;
  double groupCharge = electrons / nGroups;
  while (nGroups--) {
    double du = gRandom->Gaus(0.0, sigmaDiffus), dv = gRandom->Gaus(0.0, sigmaDiffus);
    double uPos = r.x() + du + lorentzShift;
    double vPos = r.y() + dv;

    // Cloud is not trapped, save it anyway into the closest pixel
    int iu = info.getUCellID(uPos, vPos, 1), iv = info.getVCellID(vPos, 1);
    sensor[Digit(iu, iv)].add(groupCharge, m_currentParticle, m_currentTrueHit);
  }

}


double VTXDigitizerModule::addNoise(const SensorInfo& info, double charge)
{
  if (charge <= 0) {
    //This is a noise Pixel,
    if (info.getApplyBinaryReadout()) {
      // Add a charge above binary threshold
      charge = 1.1 * info.getBinaryHitThreshold();
    } else {
      // Add charge above threshold such that
      // after analog to digital conversion we are above the digital threshold
      charge = 1.1 * info.getChargeThreshold() * info.getElectronToADU();
    }
  } else {
    if (info.getApplyPoissonSmearing()) {
      // For big charge assume Gaussian distr.
      if (charge > (1000. * Unit::e))
        charge = gRandom->Gaus(charge, sqrt(charge));
      else
        // Otherwise Poisson distr.
        charge = gRandom->Poisson(charge);
    }
    if (info.getApplyElectronicEffects()) {
      charge += gRandom->Gaus(0., info.getElectronicNoise());
    }
  }
  return charge;
}

void VTXDigitizerModule::addNoiseDigits()
{
  for (Sensors::value_type& sensor : m_sensors) {
    Sensor& s = sensor.second;

    //Calculate the number of pixels on an empty sensor which will exceed the noise cut
    const SensorInfo& info =
      dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensor.first));

    if (!info.getApplyElectronicEffects()) {return;}

    int nU = info.getUCells();
    int nV = info.getVCells();
    int nPixels = gRandom->Poisson(info.getNoiseFraction() * nU * nV);

    // Existing digits will get noise in VTXDIgitizer::AddNoise.
    // Here we add zero charge to nPixels digits.
    // In part, this will create some new zero-charge digits.
    // If we happen to select an existing (fired) digit, it remains unchanged.
    for (int i = 0; i < nPixels; ++i) {
      Digit d(gRandom->Integer(nU), gRandom->Integer(nV));
      s[d].add(0.0);
    }
  }
}



void VTXDigitizerModule::saveDigits()
{
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<VTXTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<VTXDigit> storeDigits(m_storeDigitsName);
  RelationArray relDigitMCParticle(storeDigits, storeMCParticles,
                                   m_relDigitMCParticleName);
  RelationArray relDigitTrueHit(storeDigits, storeTrueHits,
                                m_relDigitTrueHitName);


  for (Sensors::value_type& sensor : m_sensors) {
    auto sensorID = sensor.first;
    const SensorInfo& info =
      dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));
    double m_chargeThreshold = info.getChargeThreshold();
    for (Sensor::value_type& digitAndValue : sensor.second) {
      const Digit& d = digitAndValue.first;
      const DigitValue& v = digitAndValue.second;

      // Check if the readout digit is coming from a masked or dead area
      // if (PXD::PXDPixelMasker::getInstance().pixelDead(sensorID, d.u(), d.v())
      //     || !PXD::PXDPixelMasker::getInstance().pixelOK(sensorID, d.u(), d.v())) {
      //   continue;
      // }

      //Add Noise where applicable
      double charge = addNoise(info, v.charge());

      //Signal processing
      if (info.getApplyBinaryReadout()) {
        if (charge < info.getBinaryHitThreshold()) continue;
        else charge = 1.0;
      } else {
        // Remove threshold for value of TOT
        charge -= m_chargeThreshold * info.getElectronToADU();

        // Check if under threshold
        if (charge < 0) continue;

        // Amplifiy and digitize charge
        charge = round(charge / info.getElectronToADU());

        // Clipping of ADC codes
        charge = (int) charge % (int) info.getMaxADUCode();
      }

      //Add the digit to datastore
      int digIndex = storeDigits.getEntries();
      storeDigits.appendNew(
        VTXDigit(sensorID, d.u(), d.v(), charge));

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


