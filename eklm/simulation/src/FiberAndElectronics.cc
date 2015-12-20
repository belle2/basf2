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
  m_digPar = digPar;
  m_fitter = fitter;
  m_hit = it;
  m_hitEnd = end;
  m_npe = 0;
  m_stripName = "Strip" + boost::lexical_cast<std::string>(it->first);
  m_histRange = m_digPar->nDigitizations * m_digPar->ADCSamplingTime;
  m_FPGAParams = {0, 0, 0};
  /* Amplitude arrays. */
  m_amplitudeDirect = (float*)calloc(m_digPar->nDigitizations, sizeof(float));
  if (m_amplitudeDirect == NULL)
    B2FATAL(MemErr);
  m_amplitudeReflected = (float*)calloc(m_digPar->nDigitizations,
                                        sizeof(float));
  if (m_amplitudeReflected == NULL)
    B2FATAL(MemErr);
  m_amplitude = (float*)calloc(m_digPar->nDigitizations, sizeof(float));
  if (m_amplitude == NULL)
    B2FATAL(MemErr);
  m_ADCAmplitude = (int*)calloc(m_digPar->nDigitizations, sizeof(int));
  if (m_ADCAmplitude == NULL)
    B2FATAL(MemErr);
}


EKLM::FiberAndElectronics::~FiberAndElectronics()
{
  free(m_amplitudeDirect);
  free(m_amplitudeReflected);
  free(m_amplitude);
  free(m_ADCAmplitude);
}

void EKLM::FiberAndElectronics::processEntry()
{
  int i, gnpe;
  double l, d, t;
  double npe;
  std::multimap<int, EKLMSimHit*>::iterator it;
  EKLMSimHit* hit;
  m_MCTime = -1;
  for (it = m_hit; it != m_hitEnd; ++it) {
    hit = it->second;
    /* Poisson mean for number of photoelectrons. */
    npe = hit->getEDep() * m_digPar->nPEperMeV;
    /* Fill histograms. */
    l = GeometryData::Instance().getStripLength(hit->getStrip()) / CLHEP::mm *
        Unit::mm;
    d = 0.5 * l - hit->getLocalPosition().x();
    t = hit->getTime() + d / m_digPar->fiberLightSpeed;
    if (m_MCTime < 0)
      m_MCTime = t;
    else
      m_MCTime = t < m_MCTime ? t : m_MCTime;
    fillSiPMOutput(l, d, gRandom->Poisson(npe), hit->getTime(), false,
                   m_digPar, m_amplitudeDirect, &gnpe);
    m_npe = gnpe;
    if (m_digPar->mirrorReflectiveIndex > 0) {
      fillSiPMOutput(l, d, gRandom->Poisson(npe), hit->getTime(), true,
                     m_digPar, m_amplitudeReflected, &gnpe);
      m_npe = m_npe + gnpe;
    }
  }
  /* Sum up histograms. */
  for (i = 0; i < m_digPar->nDigitizations; i++) {
    m_amplitude[i] = m_amplitudeDirect[i];
    if (m_digPar->mirrorReflectiveIndex > 0)
      m_amplitude[i] = m_amplitude[i] + m_amplitudeReflected[i];
  }
  /* SiPM noise and ADC. */
  if (m_digPar->meanSiPMNoise > 0)
    addRandomSiPMNoise();
  simulateADC();
  /* Fit. */
  m_FPGAParams.bgAmplitude = m_digPar->ADCPedestal;
  m_FPGAStat = m_fitter->fit(m_ADCAmplitude, &m_FPGAParams);
  if (m_FPGAStat != c_FPGASuccessfulFit)
    return;
  /**
   * TODO: Change units.
   * FPGA fitter now uses units:
   * time = ADC conversion time,
   * amplitude = amplitude * 0.5 * m_digPar->ADCRange.
   */
  m_FPGAParams.startTime = m_FPGAParams.startTime * m_digPar->ADCSamplingTime;
  if (m_digPar->debug)
    if (m_npe >= 10)
      debugOutput();
}

void EKLM::FiberAndElectronics::addRandomSiPMNoise()
{
  int i;
  for (i = 0; i < m_digPar->nDigitizations; i++)
    m_amplitude[i] = m_amplitude[i] + gRandom->Poisson(m_digPar->meanSiPMNoise);
}

void EKLM::fillSiPMOutput(double stripLen, double distSiPM,
                          int nPE, double timeShift,
                          bool isReflected,
                          struct DigitizationParams* digPar,
                          float* hist, int* gnpe)
{
  int i;
  int j;
  double hitTime;
  double digTime;
  double deExcitationTime;
  double cosTheta;
  double hitDist;
  double sig, dt;
  *gnpe = 0;
  dt = 0.5 * digPar->ADCSamplingTime;
  for (j = 0; j < digPar->nDigitizations; j++)
    hist[j] = 0;
  for (i = 0; i < nPE; i++) {
    cosTheta = gRandom->Uniform(digPar->minCosTheta, 1);
    if (!isReflected)
      hitDist = distSiPM / cosTheta;
    else
      hitDist = (2.0 * stripLen - distSiPM) / cosTheta;
    /* Drop lightflashes which were captured by fiber. */
    if (gRandom->Uniform() > exp(-hitDist / digPar->attenuationLength))
      continue;
    /* Account for mirror reflective index. */
    if (isReflected)
      if (gRandom->Uniform() > digPar->mirrorReflectiveIndex)
        continue;
    *gnpe = *gnpe + 1;
    deExcitationTime = gRandom->Exp(digPar->scintillatorDeExcitationTime) +
                       gRandom->Exp(digPar->fiberDeExcitationTime);
    hitTime = hitDist / digPar->fiberLightSpeed + deExcitationTime +
              timeShift;
    for (j = 0; j < digPar->nDigitizations; j++) {
      digTime = j * digPar->ADCSamplingTime + 0.5;
      if (digTime > hitTime + dt)
        sig = (exp(-digPar->PEAttenuationFreq * (digTime - hitTime - dt)) -
               exp(-digPar->PEAttenuationFreq * (digTime - hitTime + dt))
              ) / digPar->PEAttenuationFreq;
      else if (digTime > hitTime - dt)
        sig = (1.0 - exp(-digPar->PEAttenuationFreq * (digTime - hitTime + dt))
              ) / digPar->PEAttenuationFreq;
      else
        sig = 0;
      hist[j] = hist[j] + sig;
    }
  }
}

void EKLM::FiberAndElectronics::simulateADC()
{
  int i;
  double amp;
  for (i = 0; i < m_digPar->nDigitizations; i++) {
    amp = m_digPar->ADCPedestal + m_digPar->ADCPEAmplitude * m_amplitude[i];
    if (amp > m_digPar->ADCSaturation)
      amp = m_digPar->ADCSaturation;
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
  return intg * m_digPar->PEAttenuationFreq / m_digPar->ADCPEAmplitude;
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
               m_digPar->nDigitizations, 0, m_histRange);
    histAmplitudeReflected =
      new TH1D("histAmplitudeReflected", m_stripName.c_str(),
               m_digPar->nDigitizations, 0, m_histRange);
    histAmplitude =
      new TH1D("histAmplitude", m_stripName.c_str(),
               m_digPar->nDigitizations, 0, m_histRange);
    histADCAmplitude =
      new TH1D("histADCAmplitude", m_stripName.c_str(),
               m_digPar->nDigitizations, 0, m_histRange);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (i = 0; i < m_digPar->nDigitizations; i++) {
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

