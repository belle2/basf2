/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/simulation/ScintillatorSimulator.h>

/* KLM headers. */
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/eklm/geometry/GeometryData.h>

/* Belle 2 headers. */
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

/* ROOT headers. */
#include <TFile.h>
#include <TH1D.h>
#include <TRandom.h>

/* C++ headers. */
#include <algorithm>
#include <string>

using namespace Belle2;

static const char MemErr[] = "Memory allocation error.";

static bool compareBKLMSimHits(const BKLMSimHit* hit1, const BKLMSimHit* hit2)
{
  return hit1->getEnergyDeposit() < hit2->getEnergyDeposit();
}

static bool compareEKLMSimHits(const EKLMSimHit* hit1, const EKLMSimHit* hit2)
{
  return hit1->getEnergyDeposit() < hit2->getEnergyDeposit();
}

void KLM::ScintillatorSimulator::reallocPhotoElectronBuffers(int size)
{
  m_PhotoelectronBufferSize = size;
  /*
   * Here there is a memory leak in case of realloc() failure, but it does not
   * matter because a fatal error is issued in this case.
   */
  /* cppcheck-suppress memleakOnRealloc */
  m_Photoelectrons =
    (struct Photoelectron*)realloc(m_Photoelectrons,
                                   size * sizeof(struct Photoelectron));
  /* cppcheck-suppress memleakOnRealloc */
  m_PhotoelectronIndex = (int*)realloc(m_PhotoelectronIndex,
                                       size * sizeof(int));
  /* cppcheck-suppress memleakOnRealloc */
  m_PhotoelectronIndex2 = (int*)realloc(m_PhotoelectronIndex2,
                                        size * sizeof(int));
  if (size != 0) {
    if (m_Photoelectrons == nullptr || m_PhotoelectronIndex == nullptr ||
        m_PhotoelectronIndex2 == nullptr)
      B2FATAL(MemErr);
  }
}

KLM::ScintillatorSimulator::ScintillatorSimulator(
  const KLMScintillatorDigitizationParameters* digPar,
  ScintillatorFirmware* fitter,
  double digitizationInitialTime, bool debug) :
  m_Time(&(KLMTime::Instance())),
  m_DigPar(digPar),
  m_fitter(fitter),
  m_DigitizationInitialTime(digitizationInitialTime),
  m_Debug(debug),
  m_FPGAStat(c_ScintillatorFirmwareNoSignal),
  m_npe(0),
  m_Energy(0),
  m_MCTime(-1),
  m_SiPMMCTime(-1)
{
  int i;
  const double samplingTime = m_DigPar->getADCSamplingTDCPeriods() *
                              m_Time->getTDCPeriod();
  /* cppcheck-suppress variableScope */
  double time, attenuationTime;
  m_histRange = m_DigPar->getNDigitizations() * samplingTime;
  m_Pedestal = m_DigPar->getADCPedestal();
  m_PhotoelectronAmplitude = m_DigPar->getADCPEAmplitude();
  m_Threshold = m_DigPar->getADCThreshold();
  /* Amplitude arrays. */
  m_amplitudeDirect = (float*)malloc(m_DigPar->getNDigitizations() *
                                     sizeof(float));
  if (m_amplitudeDirect == nullptr)
    B2FATAL(MemErr);
  m_amplitudeReflected = (float*)malloc(m_DigPar->getNDigitizations() *
                                        sizeof(float));
  if (m_amplitudeReflected == nullptr)
    B2FATAL(MemErr);
  m_amplitude = (float*)malloc(m_DigPar->getNDigitizations() * sizeof(float));
  if (m_amplitude == nullptr)
    B2FATAL(MemErr);
  m_ADCAmplitude = (int*)malloc(m_DigPar->getNDigitizations() * sizeof(int));
  if (m_ADCAmplitude == nullptr)
    B2FATAL(MemErr);
  m_SignalTimeDependence = (double*)malloc((m_DigPar->getNDigitizations() + 1) *
                                           sizeof(double));
  if (m_SignalTimeDependence == nullptr)
    B2FATAL(MemErr);
  m_SignalTimeDependenceDiff = (double*)malloc(m_DigPar->getNDigitizations() *
                                               sizeof(double));
  if (m_SignalTimeDependenceDiff == nullptr)
    B2FATAL(MemErr);
  attenuationTime = 1.0 / m_DigPar->getPEAttenuationFrequency();
  for (i = 0; i <= m_DigPar->getNDigitizations(); i++) {
    time = samplingTime * i;
    m_SignalTimeDependence[i] =
      exp(-digPar->getPEAttenuationFrequency() * time) * attenuationTime /
      samplingTime;
    if (i > 0) {
      m_SignalTimeDependenceDiff[i - 1] = m_SignalTimeDependence[i - 1] -
                                          m_SignalTimeDependence[i];
    }
  }
  m_Photoelectrons = nullptr;
  m_PhotoelectronIndex = nullptr;
  m_PhotoelectronIndex2 = nullptr;
  reallocPhotoElectronBuffers(100);
}


KLM::ScintillatorSimulator::~ScintillatorSimulator()
{
  free(m_amplitudeDirect);
  free(m_amplitudeReflected);
  free(m_amplitude);
  free(m_ADCAmplitude);
  free(m_SignalTimeDependence);
  free(m_SignalTimeDependenceDiff);
  free(m_Photoelectrons);
  free(m_PhotoelectronIndex);
  free(m_PhotoelectronIndex2);
}

void KLM::ScintillatorSimulator::setFEEData(
  const KLMScintillatorFEEData* FEEData)
{
  m_Pedestal = FEEData->getPedestal();
  m_PhotoelectronAmplitude = FEEData->getPhotoelectronAmplitude();
  m_Threshold = FEEData->getThreshold();
}

void KLM::ScintillatorSimulator::prepareSimulation()
{
  m_MCTime = -1;
  m_SiPMMCTime = -1;
  m_npe = 0;
  m_Energy = 0;
  for (int i = 0; i < m_DigPar->getNDigitizations(); i++) {
    if (m_Debug) {
      m_amplitudeDirect[i] = 0;
      m_amplitudeReflected[i] = 0;
    } else
      m_amplitude[i] = 0;
  }
}

void KLM::ScintillatorSimulator::simulate(
  const std::multimap<uint16_t, const BKLMSimHit*>::iterator& firstHit,
  const std::multimap<uint16_t, const BKLMSimHit*>::iterator& end)
{
  m_stripName = "strip_" + std::to_string(firstHit->first);
  prepareSimulation();
  bklm::GeometryPar* geoPar = bklm::GeometryPar::instance();
  const BKLMSimHit* hit = firstHit->second;
  const bklm::Module* module =
    geoPar->findModule(hit->getSection(), hit->getSector(), hit->getLayer());
  double stripLength =
    2.0 * (hit->isPhiReadout() ?
           module->getPhiScintHalfLength(hit->getStrip()) :
           module->getZScintHalfLength(hit->getStrip()));
  std::vector<const BKLMSimHit*> hits;
  for (std::multimap<uint16_t, const BKLMSimHit*>::iterator it = firstHit;
       it != end; ++it)
    hits.push_back(it->second);
  std::sort(hits.begin(), hits.end(), compareBKLMSimHits);
  for (std::vector<const BKLMSimHit*>::iterator it = hits.begin();
       it != hits.end(); ++it) {
    hit = *it;
    m_Energy = m_Energy + hit->getEnergyDeposit();
    /* Poisson mean for number of photons. */
    double nPhotons = hit->getEnergyDeposit() * m_DigPar->getNPEperMeV();
    /* Fill histograms. */
    double sipmDistance = hit->getPropagationTime() *
                          m_DigPar->getFiberLightSpeed();
    double time = hit->getTime() + hit->getPropagationTime();
    if (m_MCTime < 0) {
      m_MCTime = hit->getTime();
      m_SiPMMCTime = time;
    } else {
      if (hit->getTime() < m_MCTime)
        m_MCTime = hit->getTime();
      if (time < m_SiPMMCTime)
        m_SiPMMCTime = time;
    }
    int generatedPhotons = gRandom->Poisson(nPhotons);
    generatePhotoelectrons(stripLength, sipmDistance, generatedPhotons,
                           hit->getTime(), false);
    if (m_DigPar->getMirrorReflectiveIndex() > 0) {
      generatedPhotons = gRandom->Poisson(nPhotons);
      generatePhotoelectrons(stripLength, sipmDistance, generatedPhotons,
                             hit->getTime(), true);
    }
  }
  performSimulation();
}

void KLM::ScintillatorSimulator::simulate(
  const std::multimap<uint16_t, const EKLMSimHit*>::iterator& firstHit,
  const std::multimap<uint16_t, const EKLMSimHit*>::iterator& end)
{
  m_stripName = "strip_" + std::to_string(firstHit->first);
  prepareSimulation();
  const EKLMSimHit* hit = firstHit->second;
  double stripLength = EKLM::GeometryData::Instance().getStripLength(
                         hit->getStrip()) / CLHEP::mm * Unit::mm;
  std::vector<const EKLMSimHit*> hits;
  for (std::multimap<uint16_t, const EKLMSimHit*>::iterator it = firstHit;
       it != end; ++it)
    hits.push_back(it->second);
  std::sort(hits.begin(), hits.end(), compareEKLMSimHits);
  for (std::vector<const EKLMSimHit*>::iterator it = hits.begin();
       it != hits.end(); ++it) {
    hit = *it;
    m_Energy = m_Energy + hit->getEnergyDeposit();
    /* Poisson mean for number of photons. */
    double nPhotons = hit->getEnergyDeposit() * m_DigPar->getNPEperMeV();
    /* Fill histograms. */
    double sipmDistance = 0.5 * stripLength - hit->getLocalPosition().x();
    double time = hit->getTime() +
                  sipmDistance / m_DigPar->getFiberLightSpeed();
    if (m_MCTime < 0)
      m_MCTime = time;
    else
      m_MCTime = time < m_MCTime ? time : m_MCTime;
    int generatedPhotons = gRandom->Poisson(nPhotons);
    generatePhotoelectrons(stripLength, sipmDistance, generatedPhotons,
                           hit->getTime(), false);
    if (m_DigPar->getMirrorReflectiveIndex() > 0) {
      generatedPhotons = gRandom->Poisson(nPhotons);
      generatePhotoelectrons(stripLength, sipmDistance, generatedPhotons,
                             hit->getTime(), true);
    }
  }
  performSimulation();
}

void KLM::ScintillatorSimulator::performSimulation()
{
  if (m_Debug) {
    fillSiPMOutput(m_amplitudeDirect, true, false);
    fillSiPMOutput(m_amplitudeReflected, false, true);
    for (int i = 0; i < m_DigPar->getNDigitizations(); i++)
      m_amplitude[i] = m_amplitudeDirect[i] + m_amplitudeReflected[i];
  } else
    fillSiPMOutput(m_amplitude, true, true);
  /* SiPM noise and ADC. */
  if (m_DigPar->getMeanSiPMNoise() > 0)
    addRandomSiPMNoise();
  simulateADC();
  m_FPGAStat = m_fitter->fit(m_ADCAmplitude, m_Threshold, &m_FPGAFit);
  if (m_FPGAStat != c_ScintillatorFirmwareSuccessfulFit)
    return;
  if (m_Debug) {
    if (m_npe >= 10)
      debugOutput();
  }
}

void KLM::ScintillatorSimulator::addRandomSiPMNoise()
{
  int i;
  for (i = 0; i < m_DigPar->getNDigitizations(); i++)
    m_amplitude[i] = m_amplitude[i] +
                     gRandom->Poisson(m_DigPar->getMeanSiPMNoise());
}

int* KLM::ScintillatorSimulator::sortPhotoelectrons(int nPhotoelectrons)
{
  int* currentIndexArray, *newIndexArray, *tmpIndexArray;
  int i, i1, i2, i1Max, i2Max, j, mergeSize;
  currentIndexArray = m_PhotoelectronIndex;
  newIndexArray = m_PhotoelectronIndex2;
  mergeSize = 1;
  while (mergeSize < nPhotoelectrons) {
    for (i = 0; i < nPhotoelectrons; i = i + 2 * mergeSize) {
      i1 = i;
      j = i;
      i2 = i + mergeSize;
      if (i2 > nPhotoelectrons)
        i2 = nPhotoelectrons;
      i1Max = i2;
      i2Max = i2 + mergeSize;
      if (i2Max > nPhotoelectrons)
        i2Max = nPhotoelectrons;
      while (i1 < i1Max || i2 < i2Max) {
        if (i1 < i1Max) {
          if (i2 < i2Max) {
            if (m_Photoelectrons[currentIndexArray[i1]].bin <
                m_Photoelectrons[currentIndexArray[i2]].bin) {
              newIndexArray[j] = currentIndexArray[i1];
              i1++;
            } else {
              newIndexArray[j] = currentIndexArray[i2];
              i2++;
            }
          } else {
            newIndexArray[j] = currentIndexArray[i1];
            i1++;
          }
        } else {
          newIndexArray[j] = currentIndexArray[i2];
          i2++;
        }
        j++;
      }
    }
    tmpIndexArray = currentIndexArray;
    currentIndexArray = newIndexArray;
    newIndexArray = tmpIndexArray;
    mergeSize = mergeSize * 2;
  }
  return currentIndexArray;
}

void KLM::ScintillatorSimulator::generatePhotoelectrons(
  double stripLen, double distSiPM, int nPhotons, double timeShift,
  bool isReflected)
{
  const double samplingTime = m_DigPar->getADCSamplingTDCPeriods() *
                              m_Time->getTDCPeriod();
  const double maxHitTime = m_DigPar->getNDigitizations() * samplingTime;
  int i;
  /* cppcheck-suppress variableScope */
  double hitTime, deExcitationTime, cosTheta, hitDist, selection;
  double inverseLightSpeed, inverseAttenuationLength;
  inverseLightSpeed = 1.0 / m_DigPar->getFiberLightSpeed();
  inverseAttenuationLength = 1.0 / m_DigPar->getAttenuationLength();
  /* Generation of photoelectrons. */
  for (i = 0; i < nPhotons; i++) {
    if (m_npe >= m_PhotoelectronBufferSize)
      reallocPhotoElectronBuffers(m_npe + 100);
    cosTheta = gRandom->Uniform(m_DigPar->getMinCosTheta(), 1);
    if (!isReflected)
      hitDist = distSiPM / cosTheta;
    else
      hitDist = (2.0 * stripLen - distSiPM) / cosTheta;
    /* Fiber absorption. */
    selection = gRandom->Uniform();
    if (selection > exp(-hitDist * inverseAttenuationLength))
      continue;
    /* Account for mirror reflective index. */
    if (isReflected) {
      selection = gRandom->Uniform();
      if (selection > m_DigPar->getMirrorReflectiveIndex())
        continue;
    }
    deExcitationTime =
      gRandom->Exp(m_DigPar->getScintillatorDeExcitationTime()) +
      gRandom->Exp(m_DigPar->getFiberDeExcitationTime());
    hitTime = hitDist * inverseLightSpeed + deExcitationTime +
              timeShift - m_DigitizationInitialTime;
    if (hitTime >= maxHitTime)
      continue;
    if (hitTime >= 0)
      m_Photoelectrons[m_npe].bin = floor(hitTime / samplingTime);
    else
      m_Photoelectrons[m_npe].bin = -1;
    m_Photoelectrons[m_npe].expTime =
      exp(m_DigPar->getPEAttenuationFrequency() * hitTime);
    m_Photoelectrons[m_npe].isReflected = isReflected;
    m_PhotoelectronIndex[m_npe] = m_npe;
    m_npe++;
  }
}

/*
 * ADC signal corresponding to a photoelectron is
 *
 * exp(-(t - tau) / t0),
 *
 * where tau is the hit time and t0 = 1.0 / m_DigPar->PEAttenuationFreq. Its
 * integral from t1 to t2 is
 *
 * t0 * exp(-(t1 - tau) / t0) - t0 * exp(-(t2 - tau) / t0).
 *
 * The integration is performed over digitization bins from (t_dig * i) to
 * (t_dig * (i + 1)), where t_dig = m_DigPar->ADCSamplingTDCPeriods() *
 * m_Time->getTDCPeriod() and i is the bin number.
 * The integrals are
 *
 * I1 = t0 - t0 * exp(-(t_dig * (i + 1) - tau) / t0)
 *
 * for the first signal bin (t_dig * i <= tau < t_dig * (i + 1)) and
 *
 * I2 = t0 * exp(-(t_dig * i - tau) / t0) -
 *      t0 * exp(-(t_dig * (i + 1) - tau) / t0)
 *
 * for the following bins. The integrals contain expressions that do not depend
 * on the hit time and may be calculated preliminary:
 *
 * B1 = t0 * exp(-(t_dig * (i + 1) / t0)
 *
 * and
 *
 * B2 = t0 * exp(-(t_dig * i) / t0) - t0 * exp(-(t_dig * (i + 1)) / t0).
 *
 * In terms of B1 and B2 the integrals are equal to
 *
 * I1 = t0 - B1 * exp(tau / t0)
 *
 * and
 *
 * I2 = B2 * exp(tau / t0).
 *
 * The sum of integrals for all photoelectrons is
 *
 * I = \sum_{hits before this bin} B2 * exp(tau_j / t0) +
 *     \sum_{hits in this bin} t0 - B1 * exp(tau_j / t0),
 *
 * where tau_j is the time of j-th hit. The bin-dependent expresions B1 and B2
 * are the same for all hits:
 *
 * I = B2 * \sum_{hits before this bin} exp(tau_j / t0) +
 *     N_i * t0 - B1 * \sum_{hits in this bin} exp(tau_j / t0).
 *
 * where N_i is the number of hits in this (i-th) bin.
 *
 * In order to get the approximation of the function value, the integrals
 * over bins are normalized:
 *
 * I -> I / t_dig.
 *
 * The normalization (1 / t_dig) is included into the definitions of B1 and B2.
 */
void KLM::ScintillatorSimulator::fillSiPMOutput(float* hist, bool useDirect,
                                                bool useReflected)
{
  /* cppcheck-suppress variableScope */
  int i, bin, maxBin;
  double attenuationTime, sig, expSum;
  /* cppcheck-suppress variableScope */
  int ind1, ind2, ind3;
  int* indexArray;
  if (m_npe == 0)
    return;
  attenuationTime = 1.0 / m_DigPar->getPEAttenuationFrequency();
  indexArray = sortPhotoelectrons(m_npe);
  ind1 = 0;
  expSum = 0;
  while (1) {
    ind2 = ind1;
    bin = m_Photoelectrons[indexArray[ind1]].bin;
    while (1) {
      ind2++;
      if (ind2 == m_npe)
        break;
      if (bin != m_Photoelectrons[indexArray[ind2]].bin)
        break;
    }
    /* Now ind1 .. ind2 - photoelectrons in current bin. */
    for (ind3 = ind1; ind3 != ind2; ind3++) {
      if (m_Photoelectrons[indexArray[ind3]].isReflected && !useReflected)
        continue;
      if (!m_Photoelectrons[indexArray[ind3]].isReflected && !useDirect)
        continue;
      if (bin >= 0) {
        sig = attenuationTime - m_Photoelectrons[indexArray[ind3]].expTime *
              m_SignalTimeDependence[bin + 1];
        hist[bin] = hist[bin] + sig;
      }
      expSum = expSum + m_Photoelectrons[indexArray[ind3]].expTime;
    }
    if (ind2 == m_npe)
      maxBin = m_DigPar->getNDigitizations() - 1;
    else
      maxBin = m_Photoelectrons[indexArray[ind2]].bin;
    for (i = bin + 1; i <= maxBin; i++) {
      sig = m_SignalTimeDependenceDiff[i] * expSum;
      hist[i] = hist[i] + sig;
    }
    if (ind2 == m_npe)
      break;
    ind1 = ind2;
  }
}

void KLM::ScintillatorSimulator::simulateADC()
{
  int i;
  /* cppcheck-suppress variableScope */
  double amp;
  if (m_Pedestal == 0 || m_PhotoelectronAmplitude == 0)
    B2FATAL("Incorrect EKLM ADC simulation parameters.");
  for (i = 0; i < m_DigPar->getNDigitizations(); i++) {
    amp = m_Pedestal - m_PhotoelectronAmplitude * m_amplitude[i];
    if (amp < m_DigPar->getADCSaturation())
      amp = m_DigPar->getADCSaturation();
    m_ADCAmplitude[i] = floor(amp);
  }
}

KLMScintillatorFirmwareFitResult* KLM::ScintillatorSimulator::getFPGAFit()
{
  return &m_FPGAFit;
}

enum KLM::ScintillatorFirmwareFitStatus KLM::ScintillatorSimulator::getFitStatus() const
{
  return m_FPGAStat;
}

double KLM::ScintillatorSimulator::getNPhotoelectrons()
{
  double intg;
  intg = m_FPGAFit.getAmplitude();
  return intg * m_DigPar->getPEAttenuationFrequency() /
         m_PhotoelectronAmplitude;
}

int KLM::ScintillatorSimulator::getNGeneratedPhotoelectrons()
{
  return m_npe;
}

double KLM::ScintillatorSimulator::getEnergy()
{
  return m_Energy;
}

void KLM::ScintillatorSimulator::debugOutput()
{
  int i;
  std::string str;
  StoreObjPtr<EventMetaData> event;
  TFile* hfile = nullptr;
  TH1D* histAmplitudeDirect = nullptr;
  TH1D* histAmplitudeReflected = nullptr;
  TH1D* histAmplitude = nullptr;
  TH1D* histADCAmplitude = nullptr;
  try {
    histAmplitudeDirect =
      new TH1D("histAmplitudeDirect", m_stripName.c_str(),
               m_DigPar->getNDigitizations(), 0, m_histRange);
    histAmplitudeReflected =
      new TH1D("histAmplitudeReflected", m_stripName.c_str(),
               m_DigPar->getNDigitizations(), 0, m_histRange);
    histAmplitude =
      new TH1D("histAmplitude", m_stripName.c_str(),
               m_DigPar->getNDigitizations(), 0, m_histRange);
    histADCAmplitude =
      new TH1D("histADCAmplitude", m_stripName.c_str(),
               m_DigPar->getNDigitizations(), 0, m_histRange);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (i = 0; i < m_DigPar->getNDigitizations(); i++) {
    histAmplitudeDirect->SetBinContent(i + 1, m_amplitudeDirect[i]);
    histAmplitudeReflected->SetBinContent(i + 1, m_amplitudeReflected[i]);
    histAmplitude->SetBinContent(i + 1, m_amplitude[i]);
    histADCAmplitude->SetBinContent(i + 1, m_ADCAmplitude[i]);
  }
  str = std::string("experiment_") + std::to_string(event->getExperiment()) +
        "_run_" + std::to_string(event->getRun()) + "_event_" +
        std::to_string(event->getEvent()) + "_" + m_stripName + ".root";
  try {
    hfile = new TFile(str.c_str(), "NEW");
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  hfile->Append(histAmplitudeDirect);
  hfile->Append(histAmplitudeReflected);
  hfile->Append(histAmplitude);
  hfile->Append(histADCAmplitude);
  hfile->Write();
  hfile->Close();
}

