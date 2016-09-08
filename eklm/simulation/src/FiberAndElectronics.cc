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
#include <framework/gearbox/GearDir.h>
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
  struct EKLM::DigitizationParams* digPar,
  FPGAFitter* fitter)
{
  int i;
  double time, attenuationTime;
  m_DigPar = digPar;
  m_fitter = fitter;
  m_npe = 0;
  m_histRange = m_DigPar->nDigitizations * m_DigPar->ADCSamplingTime;
  m_FPGAParams = {0, 0, 0};
  /* Amplitude arrays. */
  m_amplitudeDirect = (float*)malloc(m_DigPar->nDigitizations * sizeof(float));
  if (m_amplitudeDirect == NULL)
    B2FATAL(MemErr);
  m_amplitudeReflected = (float*)malloc(m_DigPar->nDigitizations *
                                        sizeof(float));
  if (m_amplitudeReflected == NULL)
    B2FATAL(MemErr);
  m_amplitude = (float*)malloc(m_DigPar->nDigitizations * sizeof(float));
  if (m_amplitude == NULL)
    B2FATAL(MemErr);
  m_ADCAmplitude = (int*)malloc(m_DigPar->nDigitizations * sizeof(int));
  if (m_ADCAmplitude == NULL)
    B2FATAL(MemErr);
  m_SignalTimeDependence = (double*)malloc((m_DigPar->nDigitizations + 1) *
                                           sizeof(double));
  if (m_SignalTimeDependence == NULL)
    B2FATAL(MemErr);
  m_SignalTimeDependenceDiff = (double*)malloc(m_DigPar->nDigitizations *
                                               sizeof(double));
  if (m_SignalTimeDependenceDiff == NULL)
    B2FATAL(MemErr);
  attenuationTime = 1.0 / m_DigPar->PEAttenuationFreq;
  for (i = 0; i <= m_DigPar->nDigitizations; i++) {
    time = digPar->ADCSamplingTime * i;
    m_SignalTimeDependence[i] = exp(-digPar->PEAttenuationFreq * time) *
                                attenuationTime;
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
  reallocPhotoElectronBuffers(0);
}

void EKLM::FiberAndElectronics::setHitRange(
  std::multimap<int, EKLMSimHit*>::iterator& it,
  std::multimap<int, EKLMSimHit*>::iterator& end)
{
  m_hit = it;
  m_hitEnd = end;
  m_stripName = "Strip" + std::to_string(it->first);
}

void EKLM::FiberAndElectronics::processEntry()
{
  int i, gnpe;
  double l, d, t;
  double nPhotons;
  std::multimap<int, EKLMSimHit*>::iterator it;
  EKLMSimHit* hit;
  m_MCTime = -1;
  m_npe = 0;
  for (i = 0; i < m_DigPar->nDigitizations; i++) {
    m_amplitudeDirect[i] = 0;
    m_amplitudeReflected[i] = 0;
  }
  for (it = m_hit; it != m_hitEnd; ++it) {
    hit = it->second;
    /* Poisson mean for number of photons. */
    nPhotons = hit->getEDep() * m_DigPar->nPEperMeV;
    /* Fill histograms. */
    l = GeometryData::Instance().getStripLength(hit->getStrip()) / CLHEP::mm *
        Unit::mm;
    d = 0.5 * l - hit->getLocalPosition().x();
    t = hit->getTime() + d / m_DigPar->fiberLightSpeed;
    if (m_MCTime < 0)
      m_MCTime = t;
    else
      m_MCTime = t < m_MCTime ? t : m_MCTime;
    fillSiPMOutput(l, d, gRandom->Poisson(nPhotons), hit->getTime(), false,
                   m_amplitudeDirect, &gnpe);
    m_npe = m_npe + gnpe;
    if (m_DigPar->mirrorReflectiveIndex > 0) {
      fillSiPMOutput(l, d, gRandom->Poisson(nPhotons), hit->getTime(), true,
                     m_amplitudeReflected, &gnpe);
      m_npe = m_npe + gnpe;
    }
  }
  /* Sum up histograms. */
  for (i = 0; i < m_DigPar->nDigitizations; i++) {
    m_amplitude[i] = m_amplitudeDirect[i];
    if (m_DigPar->mirrorReflectiveIndex > 0)
      m_amplitude[i] = m_amplitude[i] + m_amplitudeReflected[i];
  }
  /* SiPM noise and ADC. */
  if (m_DigPar->meanSiPMNoise > 0)
    addRandomSiPMNoise();
  simulateADC();
  /* Fit. */
  m_FPGAParams.bgAmplitude = m_DigPar->ADCPedestal;
  m_FPGAStat = m_fitter->fit(m_ADCAmplitude, &m_FPGAParams);
  if (m_FPGAStat != c_FPGASuccessfulFit)
    return;
  /**
   * TODO: Change units.
   * FPGA fitter now uses units:
   * time = ADC conversion time,
   * amplitude = amplitude * 0.5 * m_DigPar->ADCRange.
   */
  m_FPGAParams.startTime = m_FPGAParams.startTime * m_DigPar->ADCSamplingTime +
                           m_DigPar->digitizationInitialTime;
  if (m_DigPar->debug)
    if (m_npe >= 10)
      debugOutput();
}

void EKLM::FiberAndElectronics::addRandomSiPMNoise()
{
  int i;
  for (i = 0; i < m_DigPar->nDigitizations; i++)
    m_amplitude[i] = m_amplitude[i] + gRandom->Poisson(m_DigPar->meanSiPMNoise);
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
void EKLM::FiberAndElectronics::fillSiPMOutput(
  double stripLen, double distSiPM, int nPhotons, double timeShift,
  bool isReflected, float* hist, int* gnpe)
{
  const double maxHitTime = m_DigPar->nDigitizations *
                            m_DigPar->ADCSamplingTime;
  int i, bin, nPhotoelectrons, maxBin;
  double hitTime, deExcitationTime, cosTheta, hitDist;
  double attenuationTime, sig, expSum;
  double inverseLightSpeed, inverseAttenuationLength;
  int ind1, ind2, ind3;
  int* indexArray;
  nPhotoelectrons = 0;
  attenuationTime = 1.0 / m_DigPar->PEAttenuationFreq;
  inverseLightSpeed = 1.0 / m_DigPar->fiberLightSpeed;
  inverseAttenuationLength = 1.0 / m_DigPar->attenuationLength;
  /* Generation of photoelectrons. */
  for (i = 0; i < nPhotons; i++) {
    if (nPhotoelectrons >= m_PhotoelectronBufferSize)
      reallocPhotoElectronBuffers(m_PhotoelectronBufferSize + 100);
    cosTheta = gRandom->Uniform(m_DigPar->minCosTheta, 1);
    if (!isReflected)
      hitDist = distSiPM / cosTheta;
    else
      hitDist = (2.0 * stripLen - distSiPM) / cosTheta;
    /* Fiber absorption. */
    if (gRandom->Uniform() > exp(-hitDist * inverseAttenuationLength))
      continue;
    /* Account for mirror reflective index. */
    if (isReflected)
      if (gRandom->Uniform() > m_DigPar->mirrorReflectiveIndex)
        continue;
    deExcitationTime = gRandom->Exp(m_DigPar->scintillatorDeExcitationTime) +
                       gRandom->Exp(m_DigPar->fiberDeExcitationTime);
    hitTime = hitDist * inverseLightSpeed + deExcitationTime +
              timeShift - m_DigPar->digitizationInitialTime;
    if (hitTime >= maxHitTime)
      continue;
    if (hitTime >= 0)
      m_Photoelectrons[nPhotoelectrons].bin =
        floor(hitTime / m_DigPar->ADCSamplingTime);
    else
      m_Photoelectrons[nPhotoelectrons].bin = -1;
    m_Photoelectrons[nPhotoelectrons].expTime =
      exp(m_DigPar->PEAttenuationFreq * hitTime);
    m_PhotoelectronIndex[nPhotoelectrons] = nPhotoelectrons;
    nPhotoelectrons++;
  }
  *gnpe = nPhotoelectrons;
  if (nPhotoelectrons == 0)
    return;
  /* Generation of ADC output. */
  indexArray = sortPhotoelectrons(nPhotoelectrons);
  ind1 = 0;
  expSum = 0;
  while (1) {
    ind2 = ind1;
    bin = m_Photoelectrons[indexArray[ind1]].bin;
    while (1) {
      ind2++;
      if (ind2 == nPhotoelectrons)
        break;
      if (bin != m_Photoelectrons[indexArray[ind2]].bin)
        break;
    }
    /* Now ind1 .. ind2 - photoelectrons in current bin. */
    for (ind3 = ind1; ind3 != ind2; ind3++) {
      if (bin >= 0) {
        sig = attenuationTime - m_Photoelectrons[indexArray[ind3]].expTime *
              m_SignalTimeDependence[bin + 1];
        hist[bin] = hist[bin] + sig;
      }
      expSum = expSum + m_Photoelectrons[indexArray[ind3]].expTime;
    }
    if (ind2 == nPhotoelectrons)
      maxBin = m_DigPar->nDigitizations - 1;
    else
      maxBin = m_Photoelectrons[indexArray[ind2]].bin;
    for (i = bin + 1; i <= maxBin; i++) {
      sig = m_SignalTimeDependenceDiff[i] * expSum;
      hist[i] = hist[i] + sig;
    }
    if (ind2 == nPhotoelectrons)
      break;
    ind1 = ind2;
  }
}

void EKLM::FiberAndElectronics::simulateADC()
{
  int i;
  double amp;
  for (i = 0; i < m_DigPar->nDigitizations; i++) {
    amp = m_DigPar->ADCPedestal + m_DigPar->ADCPEAmplitude * m_amplitude[i];
    if (amp > m_DigPar->ADCSaturation)
      amp = m_DigPar->ADCSaturation;
    m_ADCAmplitude[i] = amp;
  }
}

struct EKLM::FPGAFitParams* EKLM::FiberAndElectronics::getFitResults()
{
  return &m_FPGAParams;
}

enum EKLM::FPGAFitStatus EKLM::FiberAndElectronics::getFitStatus() const
{
  return m_FPGAStat;
}

double EKLM::FiberAndElectronics::getNPE()
{
  double intg;
  intg = m_FPGAParams.amplitude;
  return intg * m_DigPar->PEAttenuationFreq / m_DigPar->ADCPEAmplitude;
}

int EKLM::FiberAndElectronics::getGeneratedNPE()
{
  return m_npe;
}

void EKLM::FiberAndElectronics::debugOutput()
{
  int i;
  TFile* hfile = NULL;
  TH1D* histAmplitudeDirect = NULL;
  TH1D* histAmplitudeReflected = NULL;
  TH1D* histAmplitude = NULL;
  TH1D* histADCAmplitude = NULL;
  try {
    histAmplitudeDirect =
      new TH1D("histAmplitudeDirect", m_stripName.c_str(),
               m_DigPar->nDigitizations, 0, m_histRange);
    histAmplitudeReflected =
      new TH1D("histAmplitudeReflected", m_stripName.c_str(),
               m_DigPar->nDigitizations, 0, m_histRange);
    histAmplitude =
      new TH1D("histAmplitude", m_stripName.c_str(),
               m_DigPar->nDigitizations, 0, m_histRange);
    histADCAmplitude =
      new TH1D("histADCAmplitude", m_stripName.c_str(),
               m_DigPar->nDigitizations, 0, m_histRange);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (i = 0; i < m_DigPar->nDigitizations; i++) {
    histAmplitudeDirect->SetBinContent(i + 1, m_amplitudeDirect[i]);
    histAmplitudeReflected->SetBinContent(i + 1, m_amplitudeReflected[i]);
    histAmplitude->SetBinContent(i + 1, m_amplitude[i]);
    histADCAmplitude->SetBinContent(i + 1, m_ADCAmplitude[i]);
  }
  std::string filename = m_stripName +
                         std::to_string(gRandom->Integer(10000000)) + ".root";
  try {
    hfile = new TFile(filename.c_str(), "NEW");
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

