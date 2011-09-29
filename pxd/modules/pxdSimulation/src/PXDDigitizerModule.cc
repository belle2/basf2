/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#include <pxd/modules/pxdSimulation/PXDDigitizerModule.h>
#include <pxd/vxd/GeoCache.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>

#include <generators/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <boost/foreach.hpp>
#include <cmath>

#include <TRandom3.h>
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

PXDDigitizerModule::PXDDigitizerModule() : Module(), m_random(0)
{
  //Set module properties
  setDescription("Digitize PXDSimHits");
  addParam("ElectronicEffects", m_applyNoise,
           "Apply electronic effects?", true);
  addParam("ElectronicNoise", m_elNoise,
           "Noise added by the electronics, set in ENC", 200.0);
  addParam("NoiseSN", m_SNAdjacent,
           "SN for digits to be considered for clustering", 4.0);

  addParam("MCParticles", m_mcColName,
           "MCParticle collection name", string(""));
  addParam("Digits", m_digitColName,
           "Digits collection name", string(""));
  addParam("SimHits", m_simhitColName,
           "SimHit collection name", string(""));
  addParam("MCSimHitRel", m_relSimName,
           "Relation between MCParticles and SimHits", string(""));
  addParam("DigitMCRel", m_relDigitName,
           "Relation between MCParticles and Digits", string(""));

  addParam("PoissonSmearing", m_applyPoisson,
           "Apply Poisson smearing of electrons collected on pixels?", true);
  addParam("IntegrationWindow", m_applyWindow,
           "Use integration window?", true);

  addParam("SegmentLength", m_segmentLength,
           "Maximum segment length (in mm)", 0.005);
  addParam("ElectronGroupSize", m_elGroupSize,
           "Split Signalpoints in smaller groups of N electrons (in e)", 100);
  addParam("ElectronStepTime", m_elStepTime,
           "Time step for tracking electron groups in readout plane (in ns)", 1.0);
  addParam("ElectronMaxSteps", m_elMaxSteps,
           "Maximum number of steps when propagating electrons", 200);
  addParam("TanLorentz", m_tanLorentz,
           "Tangent of Lorentz angle", 0.25);

  addParam("ADC", m_applyADC,
           "Simulate ADC?", false);
  addParam("ADCRange", m_rangeADC,
           "Set analog-to-digital converter range 0 - ? (in e)", 31e3);
  addParam("ADCBits", m_bitsADC,
           "Set how many bits the ADC uses", 5.0);

  addParam("SimpleDriftModel", m_useSimpleDrift,
           "Use deprecated drift model?", true);
  addParam("Diffusion", m_diffusionCoefficient,
           "Diffusion coefficient (in mm)", double(0.00008617));
  addParam("widthOfDiffusCloud", m_widthOfDiffusCloud,
           "Integration range the e- cloud in sigmas", double(5.0));

  addParam("statisticsFilename", m_rootFilename,
           "ROOT Filename for statistics generation. If filename is empty, no statistics will be produced", string(""));
}

void PXDDigitizerModule::initialize()
{
  //Check for existing collections
  StoreArray<MCParticle> storeMC(m_mcColName);
  StoreArray<PXDSimHit>  storeSimHits(m_simhitColName);
  RelationArray relSimHits(storeMC, storeSimHits, m_relSimName);
  //Set name in case default was used
  m_relSimName = relSimHits.getName();

  //Register Output collections
  StoreArray<PXDDigit>   storeDigits(m_digitColName);
  RelationArray relDigits(storeDigits, storeMC, m_relDigitName);
  //Set name in case default was used
  m_relDigitName = relDigits.getName();

  //Convert parameters to correct units
  m_elNoise *= Unit::e;
  m_elStepTime *= Unit::ns;
  m_segmentLength *= Unit::mm;
  m_rangeADC = m_rangeADC * Unit::e;
  m_unitADC = m_rangeADC / (pow(2.0, m_bitsADC) - 1);
  m_diffusionCoefficient *= Unit::mm;
  m_noiseFraction = TMath::Freq(m_SNAdjacent);

  B2INFO("PXDDigitizer Parameters (in default system units, *=cannot be set directly):");
  B2INFO(" -->  ElectronicEffects:  " << (m_applyNoise ? "true" : "false"));
  B2INFO(" -->  ElectronicNoise:    " << m_elNoise);
  B2INFO(" -->  NoiseSN:            " << m_SNAdjacent);
  B2INFO(" --> *NoiseFraction:      " << m_noiseFraction);
  B2INFO(" -->  MCParticles:        " << storeMC.getName());
  B2INFO(" -->  Digits:             " << storeDigits.getName());
  B2INFO(" -->  SimHits:            " << storeSimHits.getName());
  B2INFO(" -->  MCSimHitRel:        " << m_relSimName);
  B2INFO(" -->  DigitMCRel:         " << m_relDigitName);
  B2INFO(" -->  PoissonSmearing:    " << (m_applyPoisson ? "true" : "false"));
  B2INFO(" -->  IntegrationWindow:  " << (m_applyWindow ? "true" : "false"));
  B2INFO(" -->  SegmentLength:      " << m_segmentLength);
  B2INFO(" -->  ElectronGroupSize:  " << m_elGroupSize);
  B2INFO(" -->  ElectronStepTime:   " << m_elStepTime);
  B2INFO(" -->  ElectronMaxSteps:   " << m_elMaxSteps);
  B2INFO(" -->  TanLorentz:         " << m_tanLorentz);
  B2INFO(" -->  ADC:                " << (m_applyADC ? "true" : "false"));
  B2INFO(" -->  ADCRange:           " << m_rangeADC);
  B2INFO(" -->  ADCBits:            " << m_bitsADC);
  B2INFO(" --> *ADCUnit:            " << m_unitADC);
  B2INFO(" -->  SimpleDriftModel:   " << (m_useSimpleDrift ? "true" : "false"));
  B2INFO(" -->  Diffusion:          " << m_diffusionCoefficient);
  B2INFO(" -->  widthOfDiffusCloud: " << m_widthOfDiffusCloud);
  B2INFO(" -->  statisticsFilename: " << m_rootFilename);

  if (!m_rootFilename.empty()) {
    m_rootFile = new TFile(m_rootFilename.c_str(), "RECREATE");
    m_rootFile->cd();
    m_histSteps = new TH1D("steps", "Diffusion steps;number of steps", m_elMaxSteps + 1, 0, m_elMaxSteps + 1);
    m_histDiffusion = new TH2D("diffusion", "Diffusion distance;u [um];v [um];", 200, -100, 100, 200, -100, 100);
  }

  m_random = new TRandom3(0);
}

void PXDDigitizerModule::beginRun()
{
  //Fill map with all possible sensors This is to slow to be done every event so
  //we fill it once and only clear the content of the sensors per event, not
  //the whole map
  m_sensors.clear();
  VXD::GeoCache &geo = VXD::GeoCache::getInstance();
  BOOST_FOREACH(VxdID layer, geo.getLayers()) {
    BOOST_FOREACH(VxdID ladder, geo.getLadders(layer)) {
      BOOST_FOREACH(VxdID sensor, geo.getSensors(ladder)) {
        m_sensors[sensor] = Sensor();
      }
    }
  }
}


void PXDDigitizerModule::event()
{
  //Clear sensors and process SimHits
  BOOST_FOREACH(Sensors::value_type &sensor, m_sensors) {
    sensor.second.clear();
  }
  m_currentSensor = 0;
  m_currentSensorInfo = 0;

  StoreArray<MCParticle> storeMC(m_mcColName);
  StoreArray<PXDSimHit> storeSimHits(m_simhitColName);
  RelationIndex<MCParticle, PXDSimHit> simRel(storeMC, storeSimHits, m_relSimName);
  unsigned int nSimHits = storeSimHits->GetEntries();
  if (nSimHits == 0) return;

  //Check sensor info and set pointers to current sensor
  for (unsigned int i = 0; i < nSimHits; ++i) {
    m_currentHit = storeSimHits[i];
    const RelationIndex<MCParticle, PXDSimHit>::Element* rel = simRel.getFirstFrom(m_currentHit);
    if (rel) {
      m_currentParticle = rel->indexFrom;
    } else {
      B2ERROR("Could not find MCParticle which produced PXDSimhit " << i);
      m_currentParticle = -1;
    }

    VxdID sensorID = m_currentHit->getSensorID();
    if (!m_currentSensorInfo || sensorID != m_currentSensorInfo->getID()) {
      m_currentSensorInfo = dynamic_cast<const SensorInfo*>(&VXD::GeoCache::get(sensorID));
      if (!m_currentSensorInfo)
        B2FATAL("SensorInformation for Sensor " << sensorID << " not found, make sure that the geometry is set up correctly");
      m_currentSensor = &m_sensors[sensorID];
      B2DEBUG(20, "Sensor Parameters for Sensor " << sensorID << ": " << endl
              << " --> Width:        " << m_currentSensorInfo->getWidth() << endl
              << " --> Length:       " << m_currentSensorInfo->getLength() << endl
              << " --> uPitch:       " << m_currentSensorInfo->getUPitch() << endl
              << " --> vPitch:       " << m_currentSensorInfo->getVPitch(-m_currentSensorInfo->getLength() / 2.0)
              << ", " << m_currentSensorInfo->getVPitch(m_currentSensorInfo->getLength() / 2.0) << endl
              << " --> Thickness:    " << m_currentSensorInfo->getThickness() << endl
              << " --> BulkDoping:   " << m_currentSensorInfo->getBulkDoping() << endl
              << " --> BackVoltage:  " << m_currentSensorInfo->getBackVoltage() << endl
              << " --> TopVoltage:   " << m_currentSensorInfo->getTopVoltage() << endl
              << " --> SourceBorder: " << m_currentSensorInfo->getSourceBorder() << endl
              << " --> ClearBorder:  " << m_currentSensorInfo->getClearBorder() << endl
              << " --> DrainBorder:  " << m_currentSensorInfo->getDrainBorder() << endl
              << " --> GateDepth:    " << m_currentSensorInfo->getGateDepth() << endl
              << " --> DoublePixel:  " << m_currentSensorInfo->getDoublePixel() << endl
             );

    }
    B2DEBUG(10, "Processing hit " << i << " in Sensor " << sensorID << ", related to MCParticle " << m_currentParticle);
    processHit();
  }

  addNoiseDigits();
  saveDigits();
}

void PXDDigitizerModule::processHit()
{
  if (m_applyWindow) {
    //Ignore hits which are outside of the PXD active time frame
    B2DEBUG(30, "Checking if hit is in timeframe "
            << m_currentSensorInfo->getIntegrationStart() << " <= "
            << m_currentHit->getGlobalTime()
            << " <= " << m_currentSensorInfo->getIntegrationEnd()
           );

    if (m_currentHit->getGlobalTime() < m_currentSensorInfo->getIntegrationStart() ||
        m_currentHit->getGlobalTime() > m_currentSensorInfo->getIntegrationEnd()) return;
  }

  //Get Steplength and direction
  const TVector3 &startPoint = m_currentHit->getPosIn();
  const TVector3 &stopPoint = m_currentHit->getPosOut();
  TVector3 direction = stopPoint - startPoint;
  double trackLength = direction.Mag();
  //Calculate the number of electrons
  double electrons = m_currentHit->getEnergyDep() * Unit::GeV / Unit::ehEnergy;

  if (m_currentHit->getPDGcode() == 22 || trackLength == 0) {
    //Photons deposit the energy at the end of their step
    driftCharge(stopPoint, electrons);
  } else {
    //Otherwise, split into segments of (default) max. 5µm and
    //drift the charges from the center of each segment
    int numberOfSegments = (trackLength / m_segmentLength) + 1;
    double segmentLength = trackLength / numberOfSegments;
    electrons /= numberOfSegments;
    direction.SetMag(1.0);

    for (int segment = 0; segment < numberOfSegments; ++segment) {
      TVector3 position = startPoint + direction * segmentLength * (segment + 0.5);
      driftCharge(position, electrons);
    }
  }
}

void PXDDigitizerModule::driftCharge(const TVector3 &position, double electrons)
{
  B2DEBUG(30, "Drifting " << electrons << " electrons at position ("
          << position.x() << ", "
          << position.y() << ", "
          << position.z() << ") with "
          << (m_useSimpleDrift ? "old" : "new") << " drift model"
         );

  if (m_useSimpleDrift) {
    driftChargeSimple(position, electrons);
    return;
  }
  //Get references to current sensor/info for ease of use
  const SensorInfo &info = *m_currentSensorInfo;
  Sensor &sensor = *m_currentSensor;

  double sensorThickness = info.getThickness();
  double distanceToPlane = 0.5 * sensorThickness - info.getGateDepth() - position.Z();

  //Lorentz shift correction FIXME: get from Bfield
  double uCenter = position.X() +  m_tanLorentz * distanceToPlane;
  double vCenter = position.Y();

  //Calculate diffusion during drift
  double ktemp = Unit::e * info.getBulkDoping() / Unit::permSi;
  double ytemp = ktemp * abs(distanceToPlane) / (0.5 * ktemp * sensorThickness + (info.getBackVoltage() - info.getTopVoltage()) / sensorThickness);
  if (ytemp > 0)  B2WARNING("Sensor is not fully depleted under this conditions, please check PXD parameters");
  if (ytemp >= 1) B2FATAL("Diffusioncoefficient becomes infinite, ytemp=" << ytemp << " sigma=" << sqrt(2* Unit::uTherm / ktemp*log(1 - ytemp)));

  double sigmaDrift  = sqrt(2 * Unit::uTherm / ktemp * log(1 - ytemp));
  double sigmaDiffus = sqrt(2 * Unit::uTherm  * Unit::eMobilitySi * m_elStepTime);
  //Divide into groups of m_elGroupSize electrons and simulate lateral diffusion of each group by doing a
  //random walk with free electron mobility in uv-plane
  int    nGroups     = (electrons / m_elGroupSize) + 1;
  double groupCharge = electrons / nGroups;

  B2DEBUG(30, "Sigma of drift diffusion is " << sigmaDrift);
  B2DEBUG(30, "Sigma of lateral diffusion is " << sigmaDiffus << " per step");
  B2DEBUG(40, "Splitting charge in " << nGroups << " groups of " << groupCharge << " electrons");
  for (int group = 0; group < nGroups; ++group) {
    //Apply initial drift diffusion
    double uPos = uCenter + m_random->Gaus(0.0, sigmaDrift);
    double vPos = vCenter + m_random->Gaus(0.0, sigmaDrift);
    int uID(0), vID(0);

    //double collectionTime(0);
    bool insideIG(false);
    for (int step = 0; step < m_elMaxSteps; ++step) {
      //Get Pixel ID and center coordinates
      uID                  = info.getUCellID(uPos, vPos, true);
      vID                  = info.getVCellID(vPos, true);
      const double uPixel  = info.getUCellPosition(uID);
      const double vPixel  = info.getVCellPosition(vID);
      //Check if electrons are close enough to internal gate
      const double uPitch  = info.getUPitch();
      const double vPitch  = info.getVPitch(vPixel);
      double lowerBorder   = 0.5 * vPitch - info.getDrainBorder();
      double upperBorder   = 0.5 * vPitch - info.getSourceBorder();
      const double uBorder = 0.5 * uPitch - info.getClearBorder();

      if (info.getDoublePixel() && vID % 2 == 0) {
        swap(lowerBorder, upperBorder);
      }
      B2DEBUG(90, "Check if position (" << uPos << "," << vPos << ") is inside IG");
      //Check if cloud inside of IG region
      if ((fabs(uPos - uPixel) < uBorder) && (vPos > vPixel - lowerBorder) && (vPos < vPixel + upperBorder)) {
        B2DEBUG(50, "Group " << group << " trapped in IG (" << uID << "," << vID << ") after " << step << " steps");
        insideIG = true;
        if (m_histSteps) m_histSteps->Fill(step);
        break;
      }

      //Random walk
      //collectionTime += m_elStepTime;
      uPos += m_random->Gaus(0.0, sigmaDiffus);
      vPos += m_random->Gaus(0.0, sigmaDiffus);
    }
    if (m_histDiffusion) m_histDiffusion->Fill((uPos - uCenter) / Unit::um, (vPos - vCenter) / Unit::um, groupCharge);

    if (!insideIG) {
      B2DEBUG(50, "Group " << group << " not inside IG after " << m_elMaxSteps << " steps");
      B2DEBUG(50, " --> charge position: u=" << uPos << ", v=" << vPos
              << ", uID=" << info.getUCellID(uPos) << ", vID=" << info.getVCellID(vPos));
      B2DEBUG(50, " --> neares pixel:    u=" << info.getUCellPosition(uID)
              << ", v=" << info.getVCellPosition(vID)
              << ", uID=" << uID << ", vID=" << vID);
      if (m_histSteps) m_histSteps->Fill(m_elMaxSteps);
    }
    sensor[Digit(uID, vID)].add(groupCharge, m_currentParticle);
  }
}

void PXDDigitizerModule::driftChargeSimple(const TVector3 &position, double electrons)
{
  //Get references to current sensor/info for ease of use
  const SensorInfo &info = *m_currentSensorInfo;
  Sensor &sensor = *m_currentSensor;

  double sensorThickness = info.getThickness();
  //Drift to module surface
  double distanceToPlane = 0.5 * sensorThickness - position.Z();
  TVector3 final = position + TVector3(m_tanLorentz * distanceToPlane, 0, distanceToPlane);
  double driftLength = (final - position).Mag();

  //Calculate diffusion of electron cloud
  double invCosLorentzAngle = sqrt(1.0 + m_tanLorentz * m_tanLorentz);
  double sigmaDiffus = sqrt(driftLength * m_diffusionCoefficient);

  //Calculate the the size of the diffusion cloud and all pixel IDs this will hit
  double uCenter = final.X();
  double vCenter = final.Y();
  double uSigma  = sigmaDiffus * invCosLorentzAngle;
  double vSigma  = sigmaDiffus;
  double uLow    = uCenter - m_widthOfDiffusCloud * uSigma;
  double uHigh   = uCenter + m_widthOfDiffusCloud * uSigma;
  double vLow    = vCenter - m_widthOfDiffusCloud * vSigma;
  double vHigh   = vCenter + m_widthOfDiffusCloud * vSigma;
  int    uIDLow  = info.getUCellID(uLow, 0, true);
  int    uIDHigh = info.getUCellID(uHigh, 0, true);
  int    vIDLow  = info.getVCellID(vLow, true);
  int    vIDHigh = info.getVCellID(vHigh, true);
  B2DEBUG(30, "Size of diffusion cloud: " << uSigma << ", " << vSigma);
  B2DEBUG(30, "uID from " << uIDLow << " to " << uIDHigh
          << ", vID from " << vIDLow << " to " << vIDHigh);

#define NORMAL_CDF(mean,sigma,x) TMath::Freq(((x)-(mean))/(sigma))

  //Now loop over all pixels and calculate the integral of the 2D gaussian charge distribution
  //Deposit the charge corresponding to the per pixel integral in each pixel
  double fraction(0);
  double vLowerTail = NORMAL_CDF(vCenter, vSigma, info.getVCellPosition(vIDLow) - 0.5 * info.getVPitch(vLow));
  for (int vID = vIDLow; vID <= vIDHigh; ++vID) {
    double vPos       = info.getVCellPosition(vID);
    double vUpperTail = NORMAL_CDF(vCenter, vSigma, vPos + 0.5 * info.getVPitch(vPos));
    double vIntegral  = vUpperTail - vLowerTail;
    vLowerTail = vUpperTail;

    double uLowerTail = NORMAL_CDF(uCenter, uSigma, info.getUCellPosition(uIDLow) - 0.5 * info.getUPitch());
    for (int uID = uIDLow; uID <= uIDHigh; ++uID) {
      double uPos       = info.getUCellPosition(uID);
      double uUpperTail = NORMAL_CDF(uCenter, uSigma, uPos + 0.5 * info.getUPitch());
      double uIntegral  = uUpperTail - uLowerTail;
      uLowerTail = uUpperTail;

      double charge = electrons * uIntegral * vIntegral;
      sensor[Digit(uID, vID)].add(charge, m_currentParticle);
      B2DEBUG(80, "Relative charge for pixel (" << uID << ", " << vID << "): " << uIntegral*vIntegral);
      fraction += uIntegral * vIntegral;
      if (m_histDiffusion && charge >= 1.0) m_histDiffusion->Fill((uPos - uCenter) / Unit::um, (vPos - vCenter) / Unit::um, charge);
    }
    B2DEBUG(30, "Fraction of charge: " << fraction);
  }

#undef NORMAL_CDF

}

double PXDDigitizerModule::addNoise(double charge)
{
  if (charge == 0) {
    //Noise Pixel, add noise to exceed Noise Threshold;
    double p = m_random->Uniform(m_noiseFraction, 1.0);
    charge = TMath::NormQuantile(p) * m_elNoise;
  } else {
    if (m_applyPoisson) {
      // For big charge assume Gaussian distr.
      if (charge > (1000. * Unit::e))
        charge = m_random->Gaus(charge, sqrt(charge));
      else  // Otherwise Poisson distr.
        charge = m_random->Poisson(charge);
    }
    if (m_applyNoise) {
      charge += m_random->Gaus(0., m_elNoise);
    }
  }
  return charge;
}

void PXDDigitizerModule::addNoiseDigits()
{
  if (!m_applyNoise) return;

  double fraction = 1 - m_noiseFraction;
  BOOST_FOREACH(Sensors::value_type &sensor, m_sensors) {
    Sensor &s = sensor.second;
    //FIXME: Backwards compatible
    if (s.size() == 0) continue;

    //Calculate the number of pixels on an empty sensor which will exceed the noise cut
    const SensorInfo &info = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensor.first));
    int nU = info.getUCells();
    int nV = info.getVCells();
    int nPixels = m_random->Poisson(fraction * nU * nV);

    //With an empty Sensor, nPixels would exceed the noise cut. If pixels are lit, these will have their own
    //noise fluctuation in addNoise. So if we find that a pixel we chose randomly to be lit already carry charge,
    //we ignore it.
    for (int i = 0; i < nPixels; ++i) {
      Digit d(m_random->Integer(nU), m_random->Integer(nV));
      //Add 0 electrons, will not modify existing digits but will add empty ones which
      //will be filled with noise in PXDDigitizer::addNoise
      s[d].add(0.0);
    }
  }
}

void PXDDigitizerModule::saveDigits()
{
  StoreArray<MCParticle> storeMC(m_mcColName);
  StoreArray<PXDDigit> storeDigits(m_digitColName);
  RelationArray storeRel(storeDigits, storeMC, m_relDigitName);

  //Clear out old digits
  storeDigits->Clear();
  storeRel.clear();

  //Zero supression cut in electrons
  double charge_threshold = m_SNAdjacent * m_elNoise;

  BOOST_FOREACH(Sensors::value_type &sensor, m_sensors) {
    int sensorID = sensor.first;
    const SensorInfo &info = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));
    BOOST_FOREACH(Sensor::value_type &digit, sensor.second) {
      const Digit &d = digit.first;
      const DigitValue &v = digit.second;

      //Add Noise where applicable
      double charge = addNoise(v.charge());

      //Limit electrons to ADC steps
      if (m_applyADC)
        charge = round(min(m_rangeADC, max(0.0, charge)) / m_unitADC) * m_unitADC;

      //Zero suppresion cut
      if (charge < charge_threshold) continue;

      //Add the digit to datastore
      int digIndex = storeDigits->GetLast() + 1;
      new(storeDigits->AddrAt(digIndex)) PXDDigit(
        sensorID, d.u(), d.v(),
        info.getUCellPosition(d.u()), info.getVCellPosition(d.v()), charge
      );

      //If the digit has any relations to MCParticles, add the Relation
      if (v.relations().size() > 0) {
        storeRel.add(digIndex, v.relations().begin(), v.relations().end());
      }
    }
  }
}


void PXDDigitizerModule::terminate()
{
  if (m_rootFile) {
    m_rootFile->Write();
    m_rootFile->Close();
  }
  if (m_random) delete m_random;
}
