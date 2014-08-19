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

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <bklm/geometry/GeometryPar.h>
#include <bklm/dataobjects/BKLMSimHit.h>
#include <bklm/dataobjects/BKLMDigit.h>
#include <eklm/simulation/FPGAFitter.h>

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
  setPropertyFlags(c_ParallelProcessingCertified);
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
  // This will be assigned in initialize()
  m_fitter = NULL;

}

BKLMDigitizerModule::~BKLMDigitizerModule()
{
}

void BKLMDigitizerModule::initialize()
{
  StoreArray<BKLMSimHit> bklmSimHit;
  bklmSimHit.required();

  // Force creation and persistence of BKLM output datastores
  StoreArray<BKLMDigit> bklmDigit;
  bklmDigit.registerInDataStore();
  bklmSimHit.registerRelationTo(bklmDigit);
  try {
    m_fitter = new EKLM::FPGAFitter(m_nDigitizations);
  } catch (std::bad_alloc& ba) {
    B2FATAL("Memory allocation error of EKLM::FPGAFitter")
  }
}

void BKLMDigitizerModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  B2INFO("BKLMDigitizer: Experiment " << evtMetaData->getExperiment() << "  run " << evtMetaData->getRun())
}

void BKLMDigitizerModule::event()
{
  //---------------------------------------------
  // Get BKLM hits collection from the data store
  //---------------------------------------------
  StoreArray<BKLMSimHit> simHits;
  if (simHits.getEntries() == 0) return;

  StoreArray<BKLMDigit> digits;

  unsigned int nDigit = 0;
  unsigned int d = 0;

  std::map<int, std::vector<std::pair<int, BKLMSimHit*> > > volIDToSimHits;
  for (int h = 0; h < simHits.getEntries(); ++h) {
    BKLMSimHit* simHit = simHits[h];
    if (simHit->inRPC()) {
      if (simHit->getStripMin() > 0) {
        for (int s = simHit->getStripMin(); s <= simHit->getStripMax(); ++s) {
          /* DIVOT - replaced by new code
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
          simHit->addRelationTo(digits[d]);  // 1 RPC hit to many digits
          */
          int moduleID = (simHit->getModuleID() & ~BKLM_STRIP_MASK) | ((s - 1) << BKLM_STRIP_BIT);
          for (d = 0; d < nDigit; ++d) {
            if (((digits[d]->getModuleID() ^ moduleID) & BKLM_MODULESTRIPID_MASK) == 0) break;
          }
          if (d == nDigit) {
            digits.appendNew(simHit, s);
            nDigit++;
          } else {
            // DIVOT need pileup of RPC hits here
          }
          simHit->addRelationTo(digits[d]);  // 1 RPC hit to many digits
        }
      }
    } else {
      int volID = simHit->getModuleID() & BKLM_MODULESTRIPID_MASK;
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

void BKLMDigitizerModule::digitize(const std::map<int, std::vector<std::pair<int, BKLMSimHit*> > >& volIDToSimHits, StoreArray<BKLMDigit>& digits)
{

  StoreArray<BKLMSimHit> simHits;

  // Digitize the pulse(s) in each scintillator strip (identified by volumeID)
  for (std::map<int, std::vector<std::pair<int, BKLMSimHit*> > >::const_iterator iVolMap = volIDToSimHits.begin(); iVolMap != volIDToSimHits.end(); ++iVolMap) {

    // Make one BKLMDigit for each scintillator strip then add relations to this BKLMDigit from each BKLMSimHit
    BKLMDigit* digit = digits.appendNew((iVolMap->second.front()).second);
    for (std::vector<std::pair<int, BKLMSimHit*> >::const_iterator iSimHit = iVolMap->second.begin(); iSimHit != iVolMap->second.end(); ++iSimHit) {
      simHits[iSimHit->first]->addRelationTo(digit);
    }

    struct EKLM::FPGAFitParams fitParams;
    fitParams.startTime = 0.0;
    fitParams.peakTime = 0.0;
    fitParams.attenuationFreq = 0.0;
    fitParams.amplitude = 0.0;
    fitParams.bgAmplitude = (double)m_enableConstBkg;
    int nPE = 0;
    enum EKLM::FPGAFitStatus status = processEntry(iVolMap->second, fitParams, nPE);
    digit->setFitStatus(status);
    // digit->setTime(fitParams.startTime); // DIVOT - this is producing garbage times
    digit->setEDep(fitParams.amplitude);
    digit->setSimNPixel(nPE);
    digit->setNPixel(m_PEAttenuationFreq * fitParams.amplitude
                     * (0.5 * fitParams.peakTime + 1.0 / fitParams.attenuationFreq));
    digit->isAboveThreshold(nPE > m_discriminatorThreshold);

  }

}

enum EKLM::FPGAFitStatus BKLMDigitizerModule::processEntry(std::vector<std::pair<int, BKLMSimHit*> > vHits, EKLM::FPGAFitParams& fitParams, int& nPE)
{

  float amplitude[m_nDigitizations];
  int   adcPulse[m_nDigitizations];
  float adcFit[m_nDigitizations];
  for (unsigned int i = 0; i < m_nDigitizations; ++i) {
    amplitude[i] = 0.0;
    adcPulse[i] = 0;
    adcFit[i] = 0.0;
  }

  GeometryPar* geoPar = GeometryPar::instance();
  BKLMSimHit* simHit = vHits.front().second;
  const bklm::Module* module = geoPar->findModule(simHit->isForward(), simHit->getSector(), simHit->getLayer());
  double scintLength2 = 4.0 * (simHit->isPhiReadout() ? module->getPhiScintHalfLength(simHit->getStrip())
                               : module->getZScintHalfLength(simHit->getStrip()));

  for (std::vector<std::pair<int, BKLMSimHit*> >::iterator iHit = vHits.begin(); iHit != vHits.end(); ++iHit) {
    simHit = iHit->second;
    double hitDist = simHit->getPropagationTime() * m_firstPhotonlightSpeed;
    double nPEmean = simHit->getEDep() * m_nPEperMeV; // Poisson mean for # of SiPM pixels
    nPE += fillAmplitude(gRandom->Poisson(nPEmean), simHit->getTime(), false, hitDist, amplitude);
    if (m_mirrorReflectiveIndex > 0)
      nPE += fillAmplitude(gRandom->Poisson(nPEmean), simHit->getTime(), true, scintLength2 - hitDist, amplitude);
  }

  // incorporate SiPM noise
  if (m_meanSiPMNoise > 0) {
    for (unsigned int i = 0; i < m_nDigitizations; ++i) {
      amplitude[i] += gRandom->Poisson(m_meanSiPMNoise);
    }
  }

  // digitize the pulse(s)
  simulateADC(adcPulse, amplitude);

  // fit the digitized pulse(s)
  EKLM::FPGAFitStatus status = m_fitter->fit(adcPulse, adcFit, &fitParams);
  if (status == EKLM::c_FPGASuccessfulFit) {
    /**
     * TODO: Change units.
     * FPGA fitter now uses units: time = ADC conversion time,
     *                             amplitude = amplitude * 0.5 * ADCRange.
     */
    fitParams.startTime *= m_ADCSamplingTime;
    fitParams.peakTime *= fitParams.peakTime;
    fitParams.attenuationFreq /= m_ADCSamplingTime;
    fitParams.amplitude *= 2.0 / EKLM::ADCRange;
    fitParams.bgAmplitude *= 2.0 / EKLM::ADCRange;
  }
  return status;
}

double BKLMDigitizerModule::signalShape(double t)
{
  return (t > 0.0 ? exp(-m_PEAttenuationFreq * t) : 0.0);
}

int BKLMDigitizerModule::fillAmplitude(int nPEsample, double timeShift,
                                       bool isReflected, double dist, float* hist)
{
  int nPE = 0;
  for (int j = 0; j < nPEsample; ++j) {
    // Attenuation in the WLS fiber
    if (gRandom->Uniform() > exp(-dist / m_attenuationLength)) continue;
    // Absorption at the mirrored end
    if (isReflected)
      if (gRandom->Uniform() > m_mirrorReflectiveIndex) continue;
    nPE++;
    double deExcitationTime = gRandom->Exp(m_scintillatorDeExcitationTime) +
                              gRandom->Exp(m_fiberDeExcitationTime);
    double hitTime = dist / (gRandom->Uniform(m_minCosTheta, 1) * m_firstPhotonlightSpeed)
                     + deExcitationTime + timeShift;
    for (unsigned int i = 0; i < m_nDigitizations; ++i)
      hist[i] += signalShape(i * m_ADCSamplingTime - hitTime);
  }
  return nPE;
}

void BKLMDigitizerModule::simulateADC(int adcPulse[], float amplitude[])
{
  for (unsigned int i = 0; i < m_nDigitizations; ++i)
    adcPulse[i] = (int)(0.5 * EKLM::ADCRange * amplitude[i]);
}

