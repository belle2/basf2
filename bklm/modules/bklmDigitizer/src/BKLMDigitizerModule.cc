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
#include <framework/gearbox/GearDir.h>

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

  // These will be (re)assigned in initialize()
  m_ADCSamplingTime = 1.0; // ns
  m_nDigitizations = 200;
  m_nPEperMeV = 22.0;
  m_minCosTheta = cos(M_PI * 26.7 / 180.0);
  m_mirrorReflectiveIndex = 0.95;
  m_scintillatorDeExcitationTime = 3.0; // ns
  m_fiberDeExcitationTime = 10.0; // ns
  m_fiberLightSpeed = 17.0; // cm/ns
  m_attenuationLength = 300.0; // cm
  m_PEAttenuationFreq = 3.0; // 1/ns
  m_meanSiPMNoise = -1;
  m_enableConstBkg = 0;
  m_timeResolution = 3.0; // ns
  m_ADCRange = 4096; // to be filled by actual value in FPGA

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
  GearDir dig("/Detector/DetectorComponent[@name=\"EKLM\"]/Content/DigitizationParams");
  m_ADCSamplingTime = dig.getDouble("ADCSamplingTime");
  m_nDigitizations = dig.getDouble("nDigitizations");
  m_nPEperMeV = dig.getDouble("nPEperMeV");
  m_minCosTheta = cos(M_PI * dig.getDouble("MaxTotalIRAngle") / 180.0);
  m_mirrorReflectiveIndex = dig.getDouble("MirrorReflectiveIndex");
  m_scintillatorDeExcitationTime = dig.getDouble("ScintDeExTime");
  m_fiberDeExcitationTime = dig.getDouble("FiberDeExTime");
  m_fiberLightSpeed = dig.getDouble("FiberLightSpeed");
  m_attenuationLength = dig.getDouble("AttenuationLength");
  m_PEAttenuationFreq = dig.getDouble("PEAttenuationFreq"); // wrong value
  m_PEAttenuationFreq = 1.0 / 8.75; // from T2K paper by F. Retiere: PoS (PD07) 017
  m_meanSiPMNoise = dig.getDouble("MeanSiPMNoise");
  m_enableConstBkg = dig.getDouble("EnableConstBkg") > 0;
  m_timeResolution = dig.getDouble("TimeResolution");
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
  int nSimHit = simHits.getEntries();
  if (nSimHit == 0) return;

  StoreArray<BKLMDigit> digits;

  unsigned int nDigit = 0;
  unsigned int d = 0;

  std::map<int, std::vector<std::pair<int, BKLMSimHit*> > > volIDToSimHits;
  for (int h = 0; h < nSimHit; ++h) {
    BKLMSimHit* simHit = simHits[h];
    if (simHit->inRPC()) {
      if (simHit->getStripMin() > 0) {
        for (int s = simHit->getStripMin(); s <= simHit->getStripMax(); ++s) {
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
}

void BKLMDigitizerModule::digitize(const std::map<int, std::vector<std::pair<int, BKLMSimHit*> > >& volIDToSimHits,
                                   StoreArray<BKLMDigit>& digits)
{

  StoreArray<BKLMSimHit> simHits;

  // Digitize the pulse(s) in each scintillator strip (identified by volumeID)
  for (std::map<int, std::vector<std::pair<int, BKLMSimHit*> > >::const_iterator iVolMap = volIDToSimHits.begin();
       iVolMap != volIDToSimHits.end(); ++iVolMap) {
    // Make one BKLMDigit for each scintillator strip then add relations to this BKLMDigit from each BKLMSimHit
    BKLMDigit* digit = digits.appendNew((iVolMap->second.front()).second);
    for (std::vector<std::pair<int, BKLMSimHit*> >::const_iterator iSimHit = iVolMap->second.begin(); iSimHit != iVolMap->second.end();
         ++iSimHit) {
      simHits[iSimHit->first]->addRelationTo(digit);
    }
    processEntry(iVolMap->second, digit);
  }

}

void BKLMDigitizerModule::processEntry(std::vector<std::pair<int, BKLMSimHit*> > vHits, BKLMDigit* digit)
{

  digit->setFitStatus(EKLM::c_FPGANoSignal);
  digit->setTime(0.0);
  digit->setEDep(0.0);
  digit->setSimNPixel(0);
  digit->setNPixel(0.0);
  digit->isAboveThreshold(false);

  int nPE = 0;
  std::vector<double> adcPulse(m_nDigitizations, 0.0);

  GeometryPar* geoPar = GeometryPar::instance();
  BKLMSimHit* simHit = vHits.front().second;
  const bklm::Module* module = geoPar->findModule(simHit->isForward(), simHit->getSector(), simHit->getLayer());
  double scintLength2 = 4.0 * (simHit->isPhiReadout() ? module->getPhiScintHalfLength(simHit->getStrip())
                               : module->getZScintHalfLength(simHit->getStrip()));

  for (std::vector<std::pair<int, BKLMSimHit*> >::iterator iHit = vHits.begin(); iHit != vHits.end(); ++iHit) {
    simHit = iHit->second;
    double hitDist = simHit->getPropagationTime() * m_fiberLightSpeed;
    double nPEmean = simHit->getEDep() * m_nPEperMeV; // Poisson mean for # of SiPM pixels
    nPE += fillAmplitude(gRandom->Poisson(nPEmean), simHit->getTime(), false, hitDist, adcPulse);
    if (m_mirrorReflectiveIndex > 0)
      nPE += fillAmplitude(gRandom->Poisson(nPEmean), simHit->getTime(), true, scintLength2 - hitDist, adcPulse);
  }

  // incorporate SiPM noise
  if (m_meanSiPMNoise > 0) {
    for (unsigned int i = 0; i < m_nDigitizations; ++i) {
      adcPulse[i] += gRandom->Poisson(m_meanSiPMNoise);
    }
  }

  // digitize the pulse(s)
  for (unsigned int i = 0; i < m_nDigitizations; ++i)
    adcPulse[i] = (int)(0.5 * m_ADCRange * adcPulse[i]);

  // "Fit" the digitized pulse(s), i.e., extract the relevant features
  // Calculate integral above threshold and simultaneously find
  // first and last point above threshold.
  double sum = 0.0;
  unsigned int first = m_nDigitizations;
  unsigned int last = 0;
  double peakAmplitude = 0.0;
  const double thr = 100.0 * m_PEAttenuationFreq;
  for (unsigned int i = 0; i < m_nDigitizations; ++i) {
    sum += adcPulse[i];
    if (adcPulse[i] > thr) {
      if (first == m_nDigitizations) first = i;
      last = i;
      if (adcPulse[i] > peakAmplitude) {
        peakAmplitude = adcPulse[i];
      }
    }
  }

  if (first == m_nDigitizations) return;

  digit->setFitStatus(EKLM::c_FPGASuccessfulFit);
  digit->setTime(first * m_ADCSamplingTime);
  digit->setSimNPixel(nPE);

  double bkgArea = 0.0;
  int bkgCount = 0;
  double cfd = -1.0;
  double cfdAmplitude = 0.05 * peakAmplitude;
  for (unsigned int i = 0; i < m_nDigitizations; ++i) {
    if ((i + 2 < first) || (i > last + 2)) {
      bkgArea += adcPulse[i];
      bkgCount++;
    }
    if ((cfd < 0.0) && (adcPulse[i] > cfdAmplitude)) {
      cfd = i + (cfdAmplitude - adcPulse[i]) / (adcPulse[i] - (i == 0 ? 0 : adcPulse[i - 1]));
    }
  }

  double bkgAmplitude = 0.0;
  if (bkgCount > 0) bkgAmplitude = bkgArea / double(bkgCount);

  digit->setNPixel((sum - (last - first + 1) * bkgAmplitude) * 2.0 / m_ADCRange);
  digit->setEDep(digit->getNPixel() / m_nPEperMeV);
  digit->isAboveThreshold(nPE > m_discriminatorThreshold);
  return;
}

int BKLMDigitizerModule::fillAmplitude(int nPEsample, double timeShift,
                                       bool isReflected, double dist, std::vector<double>& adcPulse)
{
  int nPE = 0;
  double attenuation = exp(-m_PEAttenuationFreq * m_ADCSamplingTime);
  double norm = 1.0 - attenuation;
  for (int i = 0; i < nPEsample; ++i) {
    // Attenuation in the WLS fiber
    if (gRandom->Uniform() > exp(-dist / m_attenuationLength)) continue;
    // Absorption at the mirrored end
    if (isReflected)
      if (gRandom->Uniform() > m_mirrorReflectiveIndex) continue;
    nPE++;
    double deExcitationTime = gRandom->Exp(m_scintillatorDeExcitationTime) +
                              gRandom->Exp(m_fiberDeExcitationTime);
    double hitTime = dist / (gRandom->Uniform(m_minCosTheta, 1.0) * m_fiberLightSpeed)
                     + deExcitationTime + timeShift;
    unsigned int jMin = 0;
    double f = 0.0;
    if (hitTime >= 0.0) {
      jMin = (unsigned int)(hitTime / m_ADCSamplingTime);
      if (jMin < m_nDigitizations) {
        f = exp(-m_PEAttenuationFreq * (jMin * m_ADCSamplingTime - hitTime));
        adcPulse[jMin] += 1.0 - f * attenuation; // partial bin
      }
    } else {
      f = exp(m_PEAttenuationFreq * hitTime);
      adcPulse[0] += norm * f; // full bin starting at t=0
    }
    for (unsigned int j = jMin + 1; j < m_nDigitizations; ++j) {
      f *= attenuation;
      adcPulse[j] += norm * f;
    }
  }
  return nPE;
}
