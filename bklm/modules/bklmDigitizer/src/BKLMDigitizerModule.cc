/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/modules/bklmDigitizer/BKLMDigitizerModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <bklm/geometry/GeometryPar.h>
#include <bklm/dataobjects/BKLMSimHit.h>
#include <bklm/dataobjects/BKLMDigit.h>

#include <TRandom.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::bklm;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BKLMDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BKLMDigitizerModule::BKLMDigitizerModule() : Module()
{

  setDescription("Creates BKLMDigits from BKLMSimHits");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
  addParam("DiscriminatorThreshold", m_discriminatorThreshold,
           "Strip hits with NPE lower this value will be marked as bad",
           double(7.0));

  // DIVOT these are hard-coded for now but should be user-definable
  m_ADCSamplingTime = 1.0;
  m_nDigitizations = 200;
  m_nPEperMeV = 22;
  m_minCosTheta = cos(26.7 / 180.0 * M_PI);
  m_mirrorReflectiveIndex = 0.95;
  m_scintillatorDeExcitationTime = 3.0;
  m_fiberDeExcitationTime = 10.0;
  m_firstPhotonlightSpeed = 17.0;
  m_attenuationLength = 300.0;
  m_PEAttenuationFreq = 3.0;
  m_meanSiPMNoise = -1;
  m_enableConstBkg = false;
  // These are calculated dynamically but initialize here to satisfy valgrind
  m_npe = 0;
  m_hitDistDirect = 0.0;
  m_hitDistReflected = 0.0;

}

BKLMDigitizerModule::~BKLMDigitizerModule()
{
}

void BKLMDigitizerModule::initialize()
{
  StoreArray<BKLMSimHit>::required();

  // Force creation and persistence of BKLM output datastores
  StoreArray<BKLMDigit>::registerPersistent();
  RelationArray::registerPersistent<BKLMSimHit, BKLMDigit>();
  try {
    m_fitter = new EKLM::FPGAFitter(m_nDigitizations);
  } catch (std::bad_alloc& ba) {
    B2FATAL("BKLMDigitizer:: Memory allocation error of FPGAFitter")
  }
}

void BKLMDigitizerModule::beginRun()
{
}

void BKLMDigitizerModule::event()
{
  //---------------------------------------------
  // Get BKLM hits collection from the data store
  //---------------------------------------------
  StoreArray<BKLMSimHit> simHits;
  if (simHits.getEntries() == 0) return;

  StoreArray<BKLMDigit> digits;
  RelationArray simHitToDigit(simHits, digits);

  unsigned int nDigit = 0;
  unsigned int d = 0;

  std::vector<unsigned int> indices;
  std::map<int, std::vector<std::pair<int, BKLMSimHit*> > > volIDToSimHits;
  for (int h = 0; h < simHits.getEntries(); ++h) {
    BKLMSimHit* simHit = simHits[h];
    if (simHit->isInRPC()) {
      indices.clear();
      if (simHit->getStripMin() >= 0) {
        for (int s = simHit->getStripMin(); s <= simHit->getStripMax(); ++s) {
          BKLMDigit digit(simHit, s);
          for (d = 0; d < nDigit; ++d) {
            if (digit.match(digits[d])) break;
          }
          if (d == nDigit) {
            digits.appendNew(digit);
            nDigit++;
          } else {
            // DIVOT need pileup of RPC hits here
          }
          indices.push_back(d);
        }
      }
      simHitToDigit.add(h, indices);  // 1 RPC hit to many digits
    } else {
      int volID = simHit->getModuleID() + simHit->getStrip();
      std::map<int, std::vector<std::pair<int, BKLMSimHit*> > >::iterator it = volIDToSimHits.find(volID);
      if (it == volIDToSimHits.end()) {
        std::vector<std::pair<int, BKLMSimHit*> > firstHit(1, std::pair<int, BKLMSimHit*>(h, simHit));
        volIDToSimHits.insert(std::pair<int, std::vector<std::pair<int, BKLMSimHit*> > >(volID, firstHit));
      } else {
        it->second.push_back(std::pair<int, BKLMSimHit*>(h, simHit));
      }
    }
  }

  // Digitize the scintillator-strip hits
  digitize(volIDToSimHits, digits);

}

void BKLMDigitizerModule::endRun()
{
}

void BKLMDigitizerModule::terminate()
{
  delete m_fitter;
}

void BKLMDigitizerModule::digitize(std::map<int, std::vector<std::pair<int, BKLMSimHit*> > > volIDToSimHits, StoreArray<BKLMDigit>& digits)
{

  StoreArray<BKLMSimHit> simHits;
  RelationArray simHitToDigit(simHits, digits);

  // Make one BKLMDigit for each scintillator strip (identified by volumeID)
  for (std::map<int, std::vector<std::pair<int, BKLMSimHit*> > >::iterator iVolMap = volIDToSimHits.begin(); iVolMap != volIDToSimHits.end(); ++iVolMap) {
    BKLMDigit* digit = new(digits.nextFreeAddress()) BKLMDigit((iVolMap->second.front()).second);
    int d = digits.getEntries() - 1;
    // Collect all SimHits in this scint associated with a given trackID by taking earliest time and sum of energy deposits
    std::map<int, std::vector<BKLMSimHit*> > trackIDToSimHit;
    for (std::vector<std::pair<int, BKLMSimHit*> >::iterator iSimHit = iVolMap->second.begin(); iSimHit != iVolMap->second.end(); ++iSimHit) {
      simHitToDigit.add(iSimHit->first, d);
      int trackID = (iSimHit->second)->getTrackID();
      std::map <int, std::vector<BKLMSimHit*> >::iterator iTrackMap = trackIDToSimHit.find(trackID);
      if (iTrackMap == trackIDToSimHit.end()) {
        trackIDToSimHit.insert(std::pair<int, std::vector<BKLMSimHit*> >(trackID, std::vector<BKLMSimHit*>(1, iSimHit->second)));
      } else {
        iTrackMap->second.push_back(iSimHit->second);
        // DIVOT Do not modify the StoreArray simhits!
        //if ((iSimHit->second)->getTime() > iTrackMap->second->getTime()) { // this hit is later
        //  iTrackMap->second->increaseEDep((iSimHit->second)->getEDep());
        //} else { // this hit is earlier
        //  (iSimHit->second)->increaseEDep(iTrackMap->second->getEDep());
        //  iTrackMap->second = iSimHit->second;
        //}
      }
    }

// DIVOT Do not merge trackID SimHit with parentID SimHit

    enum EKLM::FPGAFitStatus status = processEntry(iVolMap->second);
    digit->setFitStatus(status);
    digit->setTime(m_FPGAParams.startTime);
    digit->setEDep(m_FPGAParams.amplitude);
    digit->setSimNPixel(m_npe);
    digit->setNPixel(m_PEAttenuationFreq * m_FPGAParams.amplitude
                     * (0.5 * m_FPGAParams.peakTime + 1.0 / m_FPGAParams.attenuationFreq));
    digit->isGood(m_npe > m_discriminatorThreshold);

  } // end of loop over VolIDToSimHit map

}

enum EKLM::FPGAFitStatus BKLMDigitizerModule::processEntry(std::vector<std::pair<int, BKLMSimHit*> > vHits)
{

  m_npe = 0;
  float m_amplitudeDirect[m_nDigitizations];
  float m_amplitudeReflected[m_nDigitizations];
  float m_amplitude[m_nDigitizations];
  int   m_ADCAmplitude[m_nDigitizations];
  float m_ADCFit[m_nDigitizations];
  // initialize variable-size arrays at run-time
  for (unsigned int i = 0; i < m_nDigitizations; ++i) {
    m_amplitudeDirect[i] = 0.0;
    m_amplitudeReflected[i] = 0.0;
    m_amplitude[i] = 0.0;
    m_ADCAmplitude[i] = 0;
    m_ADCFit[i] = 0.0;
  }

  GeometryPar* geoPar = GeometryPar::instance();
  BKLMSimHit* simHit = vHits.front().second;
  const bklm::Module* module = geoPar->findModule(simHit->isForward(), simHit->getSector(), simHit->getLayer());
  double half_len = (simHit->isPhiReadout() ? module->getPhiScintHalfLength(simHit->getStrip())
                     : module->getZScintHalfLength(simHit->getStrip()));

  for (std::vector<std::pair<int, BKLMSimHit*> >::iterator iHit = vHits.begin(); iHit != vHits.end(); ++iHit) {

    simHit = iHit->second;
    // calculate distance
    double local_pos = simHit->getLocalPosition().x();
    m_hitDistDirect = half_len - local_pos;
    m_hitDistReflected = 3.0 * half_len + local_pos;

    // Poisson mean for # of p.e.
    double nPEmean = simHit->getEDep() * m_nPEperMeV;

    // fill histograms
    fillAmplitude(gRandom->Poisson(nPEmean), simHit->getTime(), false, m_amplitudeDirect);
    if (m_mirrorReflectiveIndex > 0)
      fillAmplitude(gRandom->Poisson(nPEmean), simHit->getTime(), true, m_amplitudeReflected);
  }

  // sum up histograms
  for (unsigned int i = 0; i < m_nDigitizations; ++i) {
    m_amplitude[i] = m_amplitudeDirect[i];
    if (m_mirrorReflectiveIndex > 0)
      m_amplitude[i] = m_amplitude[i] + m_amplitudeReflected[i];
  }

  /* SiPM noise and ADC. */
  if (m_meanSiPMNoise > 0)
    for (unsigned int i = 0; i < m_nDigitizations; ++i)
      m_amplitude[i] = m_amplitude[i] + gRandom->Poisson(m_meanSiPMNoise);
  simulateADC(m_ADCAmplitude, m_amplitude);

  /* Fit. */
  m_FPGAParams.bgAmplitude = (double)m_enableConstBkg;
  m_FPGAStat = m_fitter->fit(m_ADCAmplitude, m_ADCFit, &m_FPGAParams);
  if (m_FPGAStat != EKLM::c_FPGASuccessfulFit)
    m_FPGAParams.startTime = 0.0;
  m_FPGAParams.peakTime = 0.0;
  m_FPGAParams.attenuationFreq = 0.0;
  m_FPGAParams.amplitude = 0.0;
  return m_FPGAStat;
  /**
   * TODO: Change units.
   * FPGA fitter now uses units: time = ADC conversion time,
   *                             amplitude = amplitude * 0.5 * ADCRange.
   */
  m_FPGAParams.startTime = m_FPGAParams.startTime * m_ADCSamplingTime;
  m_FPGAParams.peakTime = m_FPGAParams.peakTime * m_ADCSamplingTime;
  m_FPGAParams.attenuationFreq = m_FPGAParams.attenuationFreq /
                                 m_ADCSamplingTime;
  m_FPGAParams.amplitude = m_FPGAParams.amplitude * 2 / EKLM::ADCRange;
  m_FPGAParams.bgAmplitude = m_FPGAParams.bgAmplitude * 2 / EKLM::ADCRange;
  return m_FPGAStat;
}

double BKLMDigitizerModule::signalShape(double t)
{
  return (t > 0.0 ? exp(-m_PEAttenuationFreq * t) : 0.0);
}

double  BKLMDigitizerModule::distanceAttenuation(double dist)
{
  return exp(-dist / m_attenuationLength);
}

void BKLMDigitizerModule::fillAmplitude(int nPE, double timeShift,
                                        bool isReflected, float* hist)
{
  for (int j = 0; j < nPE; ++j) {
    double cosTheta = gRandom->Uniform(m_minCosTheta, 1);
    double hitDist = (isReflected ? m_hitDistReflected : m_hitDistDirect) / cosTheta;
    /* Drop lightflashes which were captured by fiber */
    if (gRandom->Uniform() > distanceAttenuation(hitDist))
      continue;
    /* Account for mirror reflective index. */
    if (isReflected)
      if (gRandom->Uniform() > m_mirrorReflectiveIndex)
        continue;
    m_npe++;
    double deExcitationTime = gRandom->Exp(m_scintillatorDeExcitationTime) +
                              gRandom->Exp(m_fiberDeExcitationTime);
    double hitTime = hitDist / m_firstPhotonlightSpeed + deExcitationTime + timeShift;
    for (unsigned int i = 0; i < m_nDigitizations; ++i)
      hist[i] = hist[i] + signalShape(i * m_ADCSamplingTime - hitTime);
  }
}

void BKLMDigitizerModule::simulateADC(int m_ADCAmplitude[], float m_amplitude[])
{
  for (unsigned int i = 0; i < m_nDigitizations; ++i)
    m_ADCAmplitude[i] = (int)(0.5 * EKLM::ADCRange * m_amplitude[i]);
}

