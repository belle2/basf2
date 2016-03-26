/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <boost/lexical_cast.hpp>
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

EKLM::FiberAndElectronics::FiberAndElectronics(
  std::multimap<int, EKLMSimHit*>::iterator& it,
  std::multimap<int, EKLMSimHit*>::iterator& end,
  struct EKLM::DigitizationParams* digPar,
  FPGAFitter* fitter)
{
  int i;
  double time, attenuationTime;
  m_DigPar = digPar;
  m_fitter = fitter;
  m_hit = it;
  m_hitEnd = end;
  m_npe = 0;
  m_stripName = "Strip" + boost::lexical_cast<std::string>(it->first);
  m_histRange = m_DigPar->nDigitizations * m_DigPar->ADCSamplingTime;
  m_FPGAParams = {0, 0, 0};
  /* Amplitude arrays. */
  m_amplitudeDirect = (float*)calloc(m_DigPar->nDigitizations, sizeof(float));
  if (m_amplitudeDirect == NULL)
    B2FATAL(MemErr);
  m_amplitudeReflected = (float*)calloc(m_DigPar->nDigitizations,
                                        sizeof(float));
  if (m_amplitudeReflected == NULL)
    B2FATAL(MemErr);
  m_amplitude = (float*)calloc(m_DigPar->nDigitizations, sizeof(float));
  if (m_amplitude == NULL)
    B2FATAL(MemErr);
  m_ADCAmplitude = (int*)calloc(m_DigPar->nDigitizations, sizeof(int));
  if (m_ADCAmplitude == NULL)
    B2FATAL(MemErr);
  try {
    m_SignalTimeDependence = new double[m_DigPar->nDigitizations + 1];
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    m_SignalTimeDependenceDiff = new double[m_DigPar->nDigitizations];
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
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
}


EKLM::FiberAndElectronics::~FiberAndElectronics()
{
  free(m_amplitudeDirect);
  free(m_amplitudeReflected);
  free(m_amplitude);
  free(m_ADCAmplitude);
  delete[] m_SignalTimeDependence;
  delete[] m_SignalTimeDependenceDiff;
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
  m_FPGAParams.startTime = m_FPGAParams.startTime * m_DigPar->ADCSamplingTime;
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

/**
 * Comparison function for photoelectrons.
 */
static bool comparePhotoelectrons(
  struct EKLM::FiberAndElectronics::Photoelectron& pe1,
  struct EKLM::FiberAndElectronics::Photoelectron& pe2)
{
  return pe1.bin < pe2.bin;
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
  int i, maxBin;
  double hitTime, deExcitationTime, cosTheta, hitDist;
  double attenuationTime, sig, expSum;
  double inverseLightSpeed, inverseAttenuationLength;
  std::vector<struct Photoelectron> photoelectrons;
  std::vector<struct Photoelectron>::iterator it, it2, it3;
  struct Photoelectron pe;
  photoelectrons.reserve(nPhotons);
  *gnpe = 0;
  attenuationTime = 1.0 / m_DigPar->PEAttenuationFreq;
  inverseLightSpeed = 1.0 / m_DigPar->fiberLightSpeed;
  inverseAttenuationLength = 1.0 / m_DigPar->attenuationLength;
  /* Generation of photoelectrons. */
  for (i = 0; i < nPhotons; i++) {
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
              timeShift;
    if (hitTime >= maxHitTime)
      continue;
    if (hitTime >= 0)
      pe.bin = floor(hitTime / m_DigPar->ADCSamplingTime);
    else
      pe.bin = -1;
    pe.expTime = exp(m_DigPar->PEAttenuationFreq * hitTime);
    photoelectrons.push_back(pe);
    *gnpe = *gnpe + 1;
  }
  if (photoelectrons.size() == 0)
    return;
  /* Generation of ADC output. */
  sort(photoelectrons.begin(), photoelectrons.end(), comparePhotoelectrons);
  it = photoelectrons.begin();
  expSum = 0;
  while (1) {
    it2 = it;
    while (1) {
      ++it2;
      if (it2 == photoelectrons.end())
        break;
      if (it2->bin != it->bin)
        break;
    }
    /* Now it .. it2 - photoelectrons in current bin. */
    for (it3 = it; it3 != it2; ++it3) {
      if (it->bin >= 0) {
        sig = attenuationTime - it3->expTime *
              m_SignalTimeDependence[it->bin + 1];
        hist[it->bin] = hist[it->bin] + sig;
      }
      expSum = expSum + it3->expTime;
    }
    if (it2 == photoelectrons.end())
      maxBin = m_DigPar->nDigitizations - 1;
    else
      maxBin = it2->bin;
    for (i = it->bin + 1; i <= maxBin; i++) {
      sig = m_SignalTimeDependenceDiff[i] * expSum;
      hist[i] = hist[i] + sig;
    }
    if (it2 == photoelectrons.end())
      break;
    it = it2;
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
                         boost::lexical_cast<std::string>(gRandom->Integer(10000000)) + ".root";
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

