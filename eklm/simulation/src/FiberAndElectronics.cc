/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* C++ headers. */
#include <string>

/* External headers. */
#include <TH1D.h>
#include <TFile.h>

/* Belle2 headers. */
#include <eklm/geometry/GeometryData.h>
#include <eklm/simulation/FiberAndElectronics.h>
#include <framework/core/RandomNumbers.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

static const char MemErr[] = "Memory allocation error.";

void EKLM::FiberAndElectronics::reallocPhotoElectronBuffers(int size)
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
    if (m_Photoelectrons == NULL || m_PhotoelectronIndex == NULL ||
        m_PhotoelectronIndex2 == NULL)
      B2FATAL(MemErr);
  }
}

EKLM::FiberAndElectronics::FiberAndElectronics(
  const EKLMDigitizationParameters* digPar, FPGAFitter* fitter,
  double digitizationInitialTime, bool debug)
{
  int i;
  double time, attenuationTime;
  m_DigPar = digPar;
  m_fitter = fitter;
  m_DigitizationInitialTime = digitizationInitialTime;
  m_Debug = debug;
  m_npe = 0;
  m_ChannelData = NULL;
  m_histRange = m_DigPar->getNDigitizations() * m_DigPar->getADCSamplingTime();
  /* Amplitude arrays. */
  m_amplitudeDirect = (float*)malloc(m_DigPar->getNDigitizations() *
                                     sizeof(float));
  if (m_amplitudeDirect == NULL)
    B2FATAL(MemErr);
  m_amplitudeReflected = (float*)malloc(m_DigPar->getNDigitizations() *
                                        sizeof(float));
  if (m_amplitudeReflected == NULL)
    B2FATAL(MemErr);
  m_amplitude = (float*)malloc(m_DigPar->getNDigitizations() * sizeof(float));
  if (m_amplitude == NULL)
    B2FATAL(MemErr);
  m_ADCAmplitude = (int*)malloc(m_DigPar->getNDigitizations() * sizeof(int));
  if (m_ADCAmplitude == NULL)
    B2FATAL(MemErr);
  m_SignalTimeDependence = (double*)malloc((m_DigPar->getNDigitizations() + 1) *
                                           sizeof(double));
  if (m_SignalTimeDependence == NULL)
    B2FATAL(MemErr);
  m_SignalTimeDependenceDiff = (double*)malloc(m_DigPar->getNDigitizations() *
                                               sizeof(double));
  if (m_SignalTimeDependenceDiff == NULL)
    B2FATAL(MemErr);
  attenuationTime = 1.0 / m_DigPar->getPEAttenuationFrequency();
  for (i = 0; i <= m_DigPar->getNDigitizations(); i++) {
    time = digPar->getADCSamplingTime() * i;
    m_SignalTimeDependence[i] =
      exp(-digPar->getPEAttenuationFrequency() * time) * attenuationTime;
    if (i > 0) {
      m_SignalTimeDependenceDiff[i - 1] = m_SignalTimeDependence[i - 1] -
                                          m_SignalTimeDependence[i];
    }
  }
  m_Photoelectrons = NULL;
  m_PhotoelectronIndex = NULL;
  m_PhotoelectronIndex2 = NULL;
  reallocPhotoElectronBuffers(100);
}


EKLM::FiberAndElectronics::~FiberAndElectronics()
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

void EKLM::FiberAndElectronics::setHitRange(
  std::multimap<int, EKLMSimHit*>::iterator& it,
  std::multimap<int, EKLMSimHit*>::iterator& end)
{
  m_hit = it;
  m_hitEnd = end;
  m_stripName = "strip_" + std::to_string(it->first);
}

void EKLM::FiberAndElectronics::setChannelData(
  const EKLMChannelData* channelData)
{
  m_ChannelData = channelData;
}

void EKLM::FiberAndElectronics::processEntry()
{
  int i;
  double l, d, t;
  double nPhotons;
  std::multimap<int, EKLMSimHit*>::iterator it;
  EKLMSimHit* hit;
  m_MCTime = -1;
  m_npe = 0;
  for (i = 0; i < m_DigPar->getNDigitizations(); i++) {
    if (m_Debug) {
      m_amplitudeDirect[i] = 0;
      m_amplitudeReflected[i] = 0;
    } else
      m_amplitude[i] = 0;
  }
  for (it = m_hit; it != m_hitEnd; ++it) {
    hit = it->second;
    /* Poisson mean for number of photons. */
    nPhotons = hit->getEDep() * m_DigPar->getNPEperMeV();
    /* Fill histograms. */
    l = GeometryData::Instance().getStripLength(hit->getStrip()) / CLHEP::mm *
        Unit::mm;
    d = 0.5 * l - hit->getLocalPosition().x();
    t = hit->getTime() + d / m_DigPar->getFiberLightSpeed();
    if (m_MCTime < 0)
      m_MCTime = t;
    else
      m_MCTime = t < m_MCTime ? t : m_MCTime;
    generatePhotoelectrons(l, d, gRandom->Poisson(nPhotons), hit->getTime(),
                           false);
    if (m_DigPar->getMirrorReflectiveIndex() > 0) {
      generatePhotoelectrons(l, d, gRandom->Poisson(nPhotons), hit->getTime(),
                             true);
    }
  }
  if (m_Debug) {
    fillSiPMOutput(m_amplitudeDirect, true, false);
    fillSiPMOutput(m_amplitudeReflected, false, true);
    for (i = 0; i < m_DigPar->getNDigitizations(); i++)
      m_amplitude[i] = m_amplitudeDirect[i] + m_amplitudeReflected[i];
  } else
    fillSiPMOutput(m_amplitude, true, true);
  /* SiPM noise and ADC. */
  if (m_DigPar->getMeanSiPMNoise() > 0)
    addRandomSiPMNoise();
  simulateADC();
  m_FPGAStat = m_fitter->fit(m_ADCAmplitude, m_ChannelData->getThreshold(),
                             &m_FPGAFit);
  if (m_FPGAStat != c_FPGASuccessfulFit)
    return;
  if (m_Debug)
    if (m_npe >= 10)
      debugOutput();
}

void EKLM::FiberAndElectronics::addRandomSiPMNoise()
{
  int i;
  for (i = 0; i < m_DigPar->getNDigitizations(); i++)
    m_amplitude[i] = m_amplitude[i] +
                     gRandom->Poisson(m_DigPar->getMeanSiPMNoise());
}

int* EKLM::FiberAndElectronics::sortPhotoelectrons(int nPhotoelectrons)
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

void EKLM::FiberAndElectronics::generatePhotoelectrons(
  double stripLen, double distSiPM, int nPhotons, double timeShift,
  bool isReflected)
{
  const double maxHitTime = m_DigPar->getNDigitizations() *
                            m_DigPar->getADCSamplingTime();
  int i;
  double hitTime, deExcitationTime, cosTheta, hitDist;
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
    if (gRandom->Uniform() > exp(-hitDist * inverseAttenuationLength))
      continue;
    /* Account for mirror reflective index. */
    if (isReflected)
      if (gRandom->Uniform() > m_DigPar->getMirrorReflectiveIndex())
        continue;
    deExcitationTime =
      gRandom->Exp(m_DigPar->getScintillatorDeExcitationTime()) +
      gRandom->Exp(m_DigPar->getFiberDeExcitationTime());
    hitTime = hitDist * inverseLightSpeed + deExcitationTime +
              timeShift - m_DigitizationInitialTime;
    if (hitTime >= maxHitTime)
      continue;
    if (hitTime >= 0)
      m_Photoelectrons[m_npe].bin =
        floor(hitTime / m_DigPar->getADCSamplingTime());
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
 * (t_dig * (i + 1)), where t_dig = m_DigPar->ADCSamplingTime and i is the bin
 * number. The integrals are
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
 * where tau_j is the time of j-th hit. The bindependent expresions B1 and B2
 * are the same for all hits:
 *
 * I = B2 * \sum_{hits before this bin} exp(tau_j / t0) +
 *     N_i * t0 - B1 * \sum_{hits in this bin} exp(tau_j / t0).
 *
 * where N_i is the number of hits in this (i-th) bin.
 */
void EKLM::FiberAndElectronics::fillSiPMOutput(float* hist, bool useDirect,
                                               bool useReflected)
{
  int i, bin, maxBin;
  double attenuationTime, sig, expSum;
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

void EKLM::FiberAndElectronics::simulateADC()
{
  int i;
  double amp;
  for (i = 0; i < m_DigPar->getNDigitizations(); i++) {
    amp = m_ChannelData->getPedestal() -
          m_ChannelData->getPhotoelectronAmplitude() * m_amplitude[i];
    if (amp < m_DigPar->getADCSaturation())
      amp = m_DigPar->getADCSaturation();
    m_ADCAmplitude[i] = floor(amp);
  }
}

EKLMFPGAFit* EKLM::FiberAndElectronics::getFPGAFit()
{
  return &m_FPGAFit;
}

enum EKLM::FPGAFitStatus EKLM::FiberAndElectronics::getFitStatus() const
{
  return m_FPGAStat;
}

double EKLM::FiberAndElectronics::getNPE()
{
  double intg;
  intg = m_FPGAFit.getAmplitude();
  return intg * m_DigPar->getPEAttenuationFrequency() /
         m_ChannelData->getPhotoelectronAmplitude();
}

int EKLM::FiberAndElectronics::getGeneratedNPE()
{
  return m_npe;
}

void EKLM::FiberAndElectronics::debugOutput()
{
  int i;
  std::string str;
  StoreObjPtr<EventMetaData> event;
  TFile* hfile = NULL;
  TH1D* histAmplitudeDirect = NULL;
  TH1D* histAmplitudeReflected = NULL;
  TH1D* histAmplitude = NULL;
  TH1D* histADCAmplitude = NULL;
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

