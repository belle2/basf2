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
#include <TRandom.h>
#include <TH1D.h>
#include <TFile.h>

/*i Belle2 headers. */
#include <eklm/geometry/GeoEKLMBelleII.h>
#include <eklm/geometry/StripData.h>
#include <eklm/simulation/FiberAndElectronics.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

using namespace Belle2;

static const char MemErr[] = "Memory allocation error.";

EKLM::FiberAndElectronics::FiberAndElectronics(
  std::pair < int, std::vector<EKLMSimHit*> > entry,
  struct EKLM::TransformData* transf,
  struct EKLM::DigitizationParams* digPar)
{
  m_digPar = digPar;
  m_npe = 0;
  m_stripName = "Strip" + boost::lexical_cast<std::string>(entry.first);

  m_histRange = m_digPar->nDigitizations * m_digPar->ADCSamplingTime;

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
  m_ADCFit = (float*)calloc(m_digPar->nDigitizations, sizeof(float));
  if (m_ADCFit == NULL)
    B2FATAL(MemErr);

  // define vector of hits
  m_vectorHits = entry.second;
  m_transf = transf;
}


EKLM::FiberAndElectronics::~FiberAndElectronics()
{
  free(m_amplitudeDirect);
  free(m_amplitudeReflected);
  free(m_amplitude);
  free(m_ADCAmplitude);
  free(m_ADCFit);
}

void EKLM::FiberAndElectronics::processEntry()
{
  int i;
  for (std::vector<EKLMSimHit*> ::iterator iHit = m_vectorHits.begin();
       iHit != m_vectorHits.end(); iHit++) {

    // calculate distance
    lightPropagationDistance(*iHit);


    // Poisson mean for # of p.e.
    double nPEmean = (*iHit)->getEDep() * m_digPar->nPEperMeV;


    // fill histograms
    fillAmplitude(gRandom->Poisson(nPEmean), (*iHit)->getTime(),
                  false, m_amplitudeDirect);
    if (m_digPar->mirrorReflectiveIndex > 0)
      fillAmplitude(gRandom->Poisson(nPEmean), (*iHit)->getTime(),
                    true, m_amplitudeReflected);

  }

  // sum up histograms
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
  m_FPGAParams.bgAmplitude = (double)m_digPar->enableConstBkg;
  m_FPGAStat = FPGAFit(m_ADCAmplitude, m_ADCFit, m_digPar->nDigitizations,
                       &m_FPGAParams);
  if (m_FPGAStat != c_FPGASuccessfulFit)
    return;
  /**
   * TODO: Change units.
   * FPGA fitter now uses units: time = ADC conversion time,
   *                             amplitude = amplitude * 0.5 * ADCRange.
   */
  m_FPGAParams.startTime = m_FPGAParams.startTime * m_digPar->ADCSamplingTime;
  m_FPGAParams.peakTime = m_FPGAParams.peakTime * m_digPar->ADCSamplingTime;
  m_FPGAParams.attenuationFreq = m_FPGAParams.attenuationFreq /
                                 m_digPar->ADCSamplingTime;
  m_FPGAParams.amplitude = m_FPGAParams.amplitude * 2 / ADCRange;
  m_FPGAParams.bgAmplitude = m_FPGAParams.bgAmplitude * 2 / ADCRange;
  if (m_digPar->debug)
    debugOutput();
}

void EKLM::FiberAndElectronics::lightPropagationDistance(EKLMSimHit* sh)
{
  double half_len;
  double local_pos;
  /* Convert mm to cm. */
  half_len = 0.05 * EKLM::stripLen[sh->getStrip() - 1];
  local_pos = sh->getLocalPosition()->x();
  m_hitDist = std::make_pair(half_len - local_pos, 3.0 * half_len + local_pos);
}

void EKLM::FiberAndElectronics::addRandomSiPMNoise()
{
  int i;
  for (i = 0; i < m_digPar->nDigitizations; i++)
    m_amplitude[i] = m_amplitude[i] + gRandom->Poisson(m_digPar->meanSiPMNoise);
}

double EKLM::FiberAndElectronics::signalShape(double t)
{
  if (t > 0)
    return exp(-m_digPar->PEAttenuationFreq * t);
  return 0;
}

double  EKLM::FiberAndElectronics::distanceAttenuation(double dist)
{
  return exp(-dist / m_digPar->attenuationLength);
}

void EKLM::FiberAndElectronics::fillAmplitude(int nPE, double timeShift,
                                              bool isReflected, float* hist)
{
  int i;
  int j;
  double hitTime;
  double deExcitationTime;
  double cosTheta;
  double hitDist;
  for (i = 0; i < nPE; i++) {
    cosTheta = gRandom->Uniform(m_digPar->minCosTheta, 1);
    if (!isReflected)
      hitDist = m_hitDist.first / cosTheta;
    else
      hitDist = m_hitDist.second / cosTheta;
    /* Drop lightflashes which were captured by fiber */
    if (gRandom->Uniform() > distanceAttenuation(hitDist))
      continue;
    /* Account for mirror reflective index. */
    if (isReflected)
      if (gRandom->Uniform() > m_digPar->mirrorReflectiveIndex)
        continue;
    m_npe++;
    deExcitationTime = gRandom->Exp(m_digPar->scintillatorDeExcitationTime) +
                       gRandom->Exp(m_digPar->fiberDeExcitationTime);
    hitTime = lightPropagationTime(hitDist) + deExcitationTime + timeShift;
    for (j = 0; j < m_digPar->nDigitizations; j++)
      hist[j] = hist[j] + signalShape(j * m_digPar->ADCSamplingTime - hitTime);
  }
}

void EKLM::FiberAndElectronics::simulateADC()
{
  int i;
  for (i = 0; i < m_digPar->nDigitizations; i++)
    m_ADCAmplitude[i] = (int)(0.5 * ADCRange * m_amplitude[i]);
}

double EKLM::FiberAndElectronics::lightPropagationTime(double L)
{
  return L / m_digPar->firstPhotonlightSpeed;
}

struct EKLM::FPGAFitParams* EKLM::FiberAndElectronics::getFitResults() {
  return &m_FPGAParams;
}

enum EKLM::FPGAFitStatus EKLM::FiberAndElectronics::getFitStatus() const
{
  return m_FPGAStat;
}

double EKLM::FiberAndElectronics::getNPE()
{
  double intg;
  intg = m_FPGAParams.amplitude * (0.5 * m_FPGAParams.peakTime +
                                   1.0 / m_FPGAParams.attenuationFreq);
  return intg * m_digPar->PEAttenuationFreq;
}

int EKLM::FiberAndElectronics::getGeneratedNPE()
{
  return m_npe;
}

void EKLM::FiberAndElectronics::debugOutput()
{
  int i;
  TH1D* histAmplitudeDirect;
  TH1D* histAmplitudeReflected;
  TH1D* histAmplitude;
  TH1D* histADCAmplitude;
  TH1D* histADCFit;
  try {
    histAmplitudeDirect =
      new TH1D("histAmplitudeDirect", m_stripName.c_str(),
               m_digPar->nDigitizations, 0, m_histRange);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    histAmplitudeReflected =
      new TH1D("histAmplitudeReflected", m_stripName.c_str(),
               m_digPar->nDigitizations, 0, m_histRange);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    histAmplitude =
      new TH1D("histAmplitude", m_stripName.c_str(),
               m_digPar->nDigitizations, 0, m_histRange);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    histADCAmplitude =
      new TH1D("histADCAmplitude", m_stripName.c_str(),
               m_digPar->nDigitizations, 0, m_histRange);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    histADCFit =
      new TH1D("histADCFit", m_stripName.c_str(),
               m_digPar->nDigitizations, 0, m_histRange);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (i = 0; i < m_digPar->nDigitizations; i++) {
    histAmplitudeDirect->SetBinContent(i + 1, m_amplitudeDirect[i]);
    histAmplitudeReflected->SetBinContent(i + 1, m_amplitudeReflected[i]);
    histAmplitude->SetBinContent(i + 1, m_amplitude[i]);
    histADCAmplitude->SetBinContent(i + 1, m_ADCAmplitude[i]);
    histADCFit->SetBinContent(i + 1, m_ADCFit[i]);
  }
  std::string filename = m_stripName +
                         boost::lexical_cast<std::string>(gRandom->Integer(10000000)) + ".root";
  TFile* hfile;
  try {
    hfile = new TFile(filename.c_str(), "NEW");
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  hfile->Append(histAmplitudeDirect);
  hfile->Append(histAmplitudeReflected);
  hfile->Append(histAmplitude);
  hfile->Append(histADCAmplitude);
  hfile->Append(histADCFit);
  hfile->Write();
  hfile->Close();
}

