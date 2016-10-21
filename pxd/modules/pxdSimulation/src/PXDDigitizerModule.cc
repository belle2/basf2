/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#include <pxd/modules/pxdSimulation/PXDDigitizerModule.h>
#include <vxd/geometry/GeoCache.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
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
  Module(), m_rootFile(0), m_histSteps(0), m_histDiffusion(0), m_histLorentz_u(
    0), m_histLorentz_v(0)
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

  addParam("ADC", m_applyADC, "Simulate ADC?", true);
  addParam("Gq", m_gq, "Gq of a pixel in nA/electron", 0.5);
  addParam("ADCFineMode", m_ADCFineMode, "Fine mode has slope of ADC curve of 70 nA/ADU, coarse mode has 130", false);

  addParam("statisticsFilename", m_rootFilename,
           "ROOT Filename for statistics generation. If filename is empty, no statistics will be produced",
           string(""));
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
  m_eToADU = (m_ADCFineMode) ? 70.0 / m_gq : 130.0 / m_gq;
  m_elStepTime *= Unit::ns;
  m_segmentLength *= Unit::mm;

  B2INFO(
    "PXDDigitizer Parameters (in system units, *=calculated +=set in xml):");
  B2INFO(" -->  ElectronicEffects:  " << (m_applyNoise ? "true" : "false"));
  B2INFO(" -->  ElectronicNoise:    " << m_elNoise << " e-");
  B2INFO(" --> +ChargeThreshold:    " << "set in xml by sensor, nominal 4 ADU");
  B2INFO(" --> *NoiseFraction:      " << "set in xml by sensor, nominal 1.0e-5");
  B2INFO(
    " -->  MCParticles:        " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2INFO(
    " -->  SimHits:            " << DataStore::arrayName<PXDSimHit>(m_storeSimHitsName));
  B2INFO(
    " -->  Digits:             " << DataStore::arrayName<PXDDigit>(m_storeDigitsName));
  B2INFO(
    " -->  TrueHits:           " << DataStore::arrayName<PXDTrueHit>(m_storeTrueHitsName));
  B2INFO(" -->  MCSimHitRel:        " << m_relMCParticleSimHitName);
  B2INFO(" -->  DigitMCRel:         " << m_relDigitMCParticleName);
  B2INFO(" -->  TrueSimRel:         " << m_relTrueHitSimHitName);
  B2INFO(" -->  DigitTrueRel:       " << m_relDigitTrueHitName);
  B2INFO(" -->  PoissonSmearing:    " << (m_applyPoisson ? "true" : "false"));
  B2INFO(" --> +IntegrationWindow:  " << (m_applyWindow ? "true" : "false") << ", size defined in xml");
  B2INFO(" -->  SegmentLength:      " << m_segmentLength << " cm");
  B2INFO(" -->  ElectronGroupSize:  " << m_elGroupSize << " e-");
  B2INFO(" -->  ElectronStepTime:   " << m_elStepTime << " ns");
  B2INFO(" -->  ElectronMaxSteps:   " << m_elMaxSteps);
  B2INFO(" -->  ADC:                " << (m_applyADC ? "true" : "false"));
  B2INFO(" -->  ADU unit:           " << m_eToADU << " e-/ADU");
  B2INFO(" -->  statisticsFilename: " << m_rootFilename);

  if (!m_rootFilename.empty()) {
    m_rootFile = new TFile(m_rootFilename.c_str(), "RECREATE");
    m_rootFile->cd();
    m_histSteps = new TH1D("steps", "Diffusion steps;number of steps",
                           m_elMaxSteps + 1, 0, m_elMaxSteps + 1);
    m_histDiffusion = new TH2D("diffusion",
                               "Diffusion distance;u [um];v [um];", 200, -100, 100, 200, -100,
                               100);
    m_histLorentz_u = new TH1D("h_LorentzAngle_u", "Lorentz angle, u", 200,
                               -0.3, 0.3);
    m_histLorentz_u->GetXaxis()->SetTitle("Lorentz angle");
    m_histLorentz_v = new TH1D("h_LorentzAngle_v", "Lorentz angle, v", 100,
                               -0.1, 0.1);
    m_histLorentz_v->GetXaxis()->SetTitle("Lorentz angle");
  }

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
    //Ignore hits which are outside of the PXD active time frame
    B2DEBUG(30,
            "Checking if hit is in timeframe " << m_currentSensorInfo->getIntegrationStart() << " <= " << m_currentHit->getGlobalTime() <<
            " <= " << m_currentSensorInfo->getIntegrationEnd());

    if (m_currentHit->getGlobalTime()
        < m_currentSensorInfo->getIntegrationStart()
        || m_currentHit->getGlobalTime()
        > m_currentSensorInfo->getIntegrationEnd())
      return;
  }

  //Get step length and direction
  const TVector3& startPoint = m_currentHit->getPosIn();
  const TVector3& stopPoint = m_currentHit->getPosOut();
  TVector3 direction = stopPoint - startPoint;
  double trackLength = direction.Mag();

  // Set magnetic field to save calls to getBField()
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


void PXDDigitizerModule::driftCharge(const TVector3& position,
                                     double electrons)
{
  // Constants for the 5-point Gauss quadrature formula
  static double alphaGL = 1.0 / 3.0 * sqrt(5.0 + 2.0 * sqrt(10.0 / 7.0));
  static double betaGL = 1.0 / 3.0 * sqrt(5.0 - 2.0 * sqrt(10.0 / 7.0));
  static double walpha = (322 - 13.0 * sqrt(70)) / 900;
  static double wbeta = (322 + 13.0 * sqrt(70)) / 900;
  static double weightGL[5] = { walpha, wbeta, 128.0 / 225.0, wbeta, walpha };

  B2DEBUG(30,
          "Drifting " << electrons << " electrons at position (" << position.x() << ", " << position.y() << ", " << position.z() << ")");

  //Get references to current sensor/info for ease of use
  const SensorInfo& info = *m_currentSensorInfo;
  Sensor& sensor = *m_currentSensor;

  double sensorThickness = info.getThickness();
  double distanceToPlane = 0.5 * sensorThickness - info.getGateDepth()
                           - position.Z();

  // We have to calculate final position, and width of the diffusion cloud.
  TVector3 currentEField = info.getEField(position);
  TVector3 v = info.getDriftVelocity(currentEField, m_currentBField);
  TVector3 positionInPlane(position);
  double sigmaDriftSqr = 0;
  // If we are close to the target plane, don't integrate
  if (fabs(distanceToPlane) > 1.0 * Unit::um) {
    // set integration region and integration points
    double h = distanceToPlane / 2.0;
    double midpoint = position.Z() + 0.5 * distanceToPlane;
    std::vector<double> zKnots(5);
    zKnots[0] = midpoint - alphaGL * h;
    zKnots[1] = midpoint - betaGL * h;
    zKnots[2] = midpoint;
    zKnots[3] = midpoint + betaGL * h;
    zKnots[4] = midpoint + alphaGL * h;
    // Calculate the quadrature.
    // We integrate
    // - v/v_z for position,
    // - 2 * kT/e * mobility(E(position)) / v_z for sigmaDrift.
    for (int iz = 0; iz < 5; ++iz) {
      // This is OK as long as E only depends on z.
      currentEField = info.getEField(TVector3(0, 0, zKnots[iz]));
      v = info.getDriftVelocity(currentEField, m_currentBField);
      double weightByT = h * weightGL[iz] / v.Z();
      positionInPlane += weightByT * v;
      sigmaDriftSqr += fabs(weightByT) * 2 * Const::uTherm
                       * info.getElectronMobility(currentEField.Mag());
    } // for knots
  } // Integration
  // Adjust sigmaDrift to _current_ Lorentz angle
  double currentTanLA_u = v.X() / v.Z();
  double currentTanLA_v = v.Y() / v.Z();
  double sigmaDrift_u = sqrt(
                          sigmaDriftSqr * (1 + currentTanLA_u * currentTanLA_u));
  double sigmaDrift_v = sqrt(
                          sigmaDriftSqr * (1 + currentTanLA_v * currentTanLA_v));
  if (fabs(distanceToPlane) > 1.0 * Unit::um) {
    double tanLorentz_u = (positionInPlane.X() - position.X())
                          / distanceToPlane;
    double tanLorentz_v = (positionInPlane.Y() - position.Y())
                          / distanceToPlane;
    if (m_histLorentz_u)
      m_histLorentz_u->Fill(tanLorentz_u);
    if (m_histLorentz_v)
      m_histLorentz_v->Fill(tanLorentz_v);
  };
  // Diffusion: Free diffusion + drift due to magnetic force. We use asymptotic mobility
  // here, as the drift is perpendicular to the "macro" E field.
  TVector3 E0(0, 0, 0);
  double sigmaDiffus = sqrt(
                         2 * Const::uTherm * info.getElectronMobility(0) * m_elStepTime);
  //Divide into groups of m_elGroupSize electrons and simulate lateral diffusion of each group by doing a
  //random walk with free electron mobility in uv-plane
  int nGroups = (int)(electrons / m_elGroupSize) + 1;
  double groupCharge = electrons / nGroups;

  B2DEBUG(30,
          "Sigma of drift diffusion is " << sigmaDriftSqr << ", i.p. " << sigmaDrift_u << " in u, and " << sigmaDrift_v << " in v");
  B2DEBUG(30, "Sigma of lateral diffusion is " << sigmaDiffus << " per step");
  B2DEBUG(40,
          "Splitting charge in " << nGroups << " groups of " << groupCharge << " electrons");
  for (int group = 0; group < nGroups; ++group) {
    //Distribute according to sigmaDrift in u and v
    TVector3 step3(gRandom->Gaus(0.0, sigmaDrift_u),
                   gRandom->Gaus(0.0, sigmaDrift_v), 0);
    double uPos = positionInPlane.X() + step3.X();
    double vPos = positionInPlane.Y() + step3.Y();
    int uID(0), vID(0);

    //double collectionTime(0);
    bool insideIG(false);
    for (int step = 0; step < m_elMaxSteps; ++step) {
      //Get Pixel ID and center coordinates
      uID = info.getUCellID(uPos, vPos, true);
      vID = info.getVCellID(vPos, true);
      const double uPixel = info.getUCellPosition(uID);
      const double vPixel = info.getVCellPosition(vID);
      //Check if electrons are close enough to internal gate
      const double uPitch = info.getUPitch();
      const double vPitch = info.getVPitch(vPixel);
      double lowerBorder = 0.5 * vPitch - info.getDrainBorder(vPixel);
      double upperBorder = 0.5 * vPitch - info.getSourceBorder(vPixel);
      const double uBorder = 0.5 * uPitch - info.getClearBorder(vPixel);

      if (info.getDoublePixel() && vID % 2 == 0) {
        swap(lowerBorder, upperBorder);
      }
      B2DEBUG(90,
              "Check if position (" << uPos << "," << vPos << ") is inside IG");
      //Check if cloud inside of IG region
      if ((fabs(uPos - uPixel) < uBorder) && (vPos > vPixel - lowerBorder)
          && (vPos < vPixel + upperBorder)) {
        B2DEBUG(50,
                "Group " << group << " trapped in IG (" << uID << "," << vID << ") after " << step << " steps");
        insideIG = true;
        if (m_histSteps)
          m_histSteps->Fill(step);
        break;
      }
      //Random walk with drift
      //collectionTime += m_elStepTime;
      step3.SetXYZ(gRandom->Gaus(0.0, sigmaDiffus),
                   gRandom->Gaus(0.0, sigmaDiffus), 0);
      step3 += info.getHallFactor() * Const::eMobilitySi
               * step3.Cross(m_currentBField);
      uPos += step3.X();
      vPos += step3.Y();
    }
    if (m_histDiffusion)
      m_histDiffusion->Fill((uPos - positionInPlane.X()) / Unit::um,
                            (vPos - positionInPlane.Y()) / Unit::um, groupCharge);

    if (!insideIG) {
      B2DEBUG(50,
              "Group " << group << " not inside IG after " << m_elMaxSteps << " steps");
      B2DEBUG(50,
              " --> charge position: u=" << uPos << ", v=" << vPos << ", uID=" << info.getUCellID(uPos) << ", vID=" << info.getVCellID(vPos));
      B2DEBUG(50,
              " --> nearest pixel:    u=" << info.getUCellPosition(uID) << ", v=" << info.getVCellPosition(
                vID) << ", uID=" << uID << ", vID=" << vID);
      if (m_histSteps)
        m_histSteps->Fill(m_elMaxSteps);
    }
    B2DEBUG(40,
            "Adding charge " << groupCharge << " to digit (" << uID << "," << vID << ")");
    sensor[Digit(uID, vID)].add(groupCharge, m_currentParticle,
                                m_currentTrueHit);
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
    int sensorID = sensor.first;
    const SensorInfo& info =
      dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));
    m_chargeThreshold = info.getChargeThreshold();
    m_chargeThresholdElectrons = m_chargeThreshold * m_eToADU;
    for (Sensor::value_type& digit : sensor.second) {
      const Digit& d = digit.first;
      const DigitValue& v = digit.second;

      //Add Noise where applicable
      double charge = addNoise(v.charge());

      //Zero suppresion cut
      if (charge < m_chargeThresholdElectrons)
        continue;

      //Limit electrons to ADC steps
      if (m_applyADC) charge = round(charge / m_eToADU);

      //Add the digit to datastore
      int digIndex = storeDigits.getEntries();
      storeDigits.appendNew(
        PXDDigit(sensorID, d.u(), d.v(),
                 info.getUCellPosition(d.u()),
                 info.getVCellPosition(d.v()), charge));

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

void PXDDigitizerModule::terminate()
{
  if (m_rootFile) {
    m_histSteps->GetListOfFunctions()->Add(new TNamed("Description", "Validation: Diffusion steps;number of steps."));
    m_histSteps->GetListOfFunctions()->Add(new TNamed("Check", "Validation: Check shape, should be mostly zero in about 40 steps."));
    m_histSteps->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
    m_histDiffusion->GetListOfFunctions()->Add(new TNamed("Description", "Validation: Diffusion distance;u [um];v [um];."));
    m_histDiffusion->GetListOfFunctions()->Add(new TNamed("Check",
                                                          "Validation: Check spot shape, should be homogeniouse arround and sharp peak in middle."));
    m_histDiffusion->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
    m_histLorentz_u->GetListOfFunctions()->Add(new TNamed("Description", "Validation: Lorentz angle, u."));
    m_histLorentz_u->GetListOfFunctions()->Add(new TNamed("Check",
                                                          "Validation: Check peak position, should be on range 0.1 .. 0.3, because magnetic field."));
    m_histLorentz_u->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
    m_histLorentz_v->GetListOfFunctions()->Add(new TNamed("Description", "Validation: Lorentz angle, v."));
    m_histLorentz_v->GetListOfFunctions()->Add(new TNamed("Check",
                                                          "Validation: Check peak position, should be on middle, because no magnet field on this direction."));
    m_histLorentz_v->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
    m_rootFile->Write();
    m_rootFile->Close();
  }
}
