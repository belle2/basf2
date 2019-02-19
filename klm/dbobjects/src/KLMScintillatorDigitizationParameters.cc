/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                 *
 * Copyright(C) 2016  Belle II Collaboration                          *
 *                                                                    *
 * Author: The Belle II Collaboration                                 *
 * Contributors: Kirill Chilikin                                      *
 *                                                                    *
 * This software is provided "as is" without any warranty.            *
 **************************************************************************/

/* Belle2 headers. */
#include <klm/dbobjects/KLMScintillatorDigitizationParameters.h>

using namespace Belle2;

KLMScintillatorDigitizationParameters::KLMScintillatorDigitizationParameters()
{
  m_ADCRange = 0;
  m_ADCSamplingTime = 0;
  m_NDigitizations = 0;
  m_ADCPedestal = 0;
  m_ADCPEAmplitude = 0;
  m_ADCThreshold = 0;
  m_ADCSaturation = 0;
  m_NPEperMeV = 0;
  m_MinCosTheta = 0;
  m_MirrorReflectiveIndex = 0;
  m_ScintillatorDeExcitationTime = 0;
  m_FiberDeExcitationTime = 0;
  m_FiberLightSpeed = 0;
  m_AttenuationLength = 0;
  m_PEAttenuationFrequency = 0;
  m_MeanSiPMNoise = 0;
  m_EnableConstBkg = false;
}

KLMScintillatorDigitizationParameters::~KLMScintillatorDigitizationParameters()
{
}

int KLMScintillatorDigitizationParameters::getADCRange() const
{
  return m_ADCRange;
}

void KLMScintillatorDigitizationParameters::setADCRange(int range)
{
  m_ADCRange = range;
}

float KLMScintillatorDigitizationParameters::getADCSamplingFrequency() const
{
  return 1.0 / m_ADCSamplingTime;
}

void KLMScintillatorDigitizationParameters::setADCSamplingFrequency(float frequency)
{
  m_ADCSamplingTime = 1.0 / frequency;
}

float KLMScintillatorDigitizationParameters::getADCSamplingTime() const
{
  return m_ADCSamplingTime;
}

void KLMScintillatorDigitizationParameters::setADCSamplingTime(float time)
{
  m_ADCSamplingTime = time;
}

int KLMScintillatorDigitizationParameters::getNDigitizations() const
{
  return m_NDigitizations;
}

void KLMScintillatorDigitizationParameters::setNDigitizations(int digitizations)
{
  m_NDigitizations = digitizations;
}

float KLMScintillatorDigitizationParameters::getADCPedestal() const
{
  return m_ADCPedestal;
}

void KLMScintillatorDigitizationParameters::setADCPedestal(float pedestal)
{
  m_ADCPedestal = pedestal;
}

float KLMScintillatorDigitizationParameters::getADCPEAmplitude() const
{
  return m_ADCPEAmplitude;
}

void KLMScintillatorDigitizationParameters::setADCPEAmplitude(float amplitude)
{
  m_ADCPEAmplitude = amplitude;
}

int KLMScintillatorDigitizationParameters::getADCThreshold() const
{
  return m_ADCThreshold;
}

void KLMScintillatorDigitizationParameters::setADCThreshold(int threshold)
{
  m_ADCThreshold = threshold;
}

int KLMScintillatorDigitizationParameters::getADCSaturation() const
{
  return m_ADCSaturation;
}

void KLMScintillatorDigitizationParameters::setADCSaturation(int saturation)
{
  m_ADCSaturation = saturation;
}

float KLMScintillatorDigitizationParameters::getNPEperMeV() const
{
  return m_NPEperMeV;
}

void KLMScintillatorDigitizationParameters::setNPEperMeV(float npe)
{
  m_NPEperMeV = npe;
}

float KLMScintillatorDigitizationParameters::getMinCosTheta() const
{
  return m_MinCosTheta;
}

void KLMScintillatorDigitizationParameters::setMinCosTheta(float minCosTheta)
{
  m_MinCosTheta = minCosTheta;
}

float KLMScintillatorDigitizationParameters::getMirrorReflectiveIndex() const
{
  return m_MirrorReflectiveIndex;
}

void KLMScintillatorDigitizationParameters::
setMirrorReflectiveIndex(float reflectiveIndex)
{
  m_MirrorReflectiveIndex = reflectiveIndex;
}

float KLMScintillatorDigitizationParameters::getScintillatorDeExcitationTime() const
{
  return m_ScintillatorDeExcitationTime;
}

void KLMScintillatorDigitizationParameters::setScintillatorDeExcitationTime(float time)
{
  m_ScintillatorDeExcitationTime = time;
}

float KLMScintillatorDigitizationParameters::getFiberDeExcitationTime() const
{
  return m_FiberDeExcitationTime;
}

void KLMScintillatorDigitizationParameters::setFiberDeExcitationTime(float time)
{
  m_FiberDeExcitationTime = time;
}

float KLMScintillatorDigitizationParameters::getFiberLightSpeed() const
{
  return m_FiberLightSpeed;
}

void KLMScintillatorDigitizationParameters::setFiberLightSpeed(float lightSpeed)
{
  m_FiberLightSpeed = lightSpeed;
}

float KLMScintillatorDigitizationParameters::getAttenuationLength() const
{
  return m_AttenuationLength;
}

void KLMScintillatorDigitizationParameters::setAttenuationLength(float length)
{
  m_AttenuationLength = length;
}

float KLMScintillatorDigitizationParameters::getPEAttenuationFrequency() const
{
  return m_PEAttenuationFrequency;
}

void KLMScintillatorDigitizationParameters::setPEAttenuationFrequency(float frequency)
{
  m_PEAttenuationFrequency = frequency;
}

float KLMScintillatorDigitizationParameters::getMeanSiPMNoise() const
{
  return m_MeanSiPMNoise;
}

void KLMScintillatorDigitizationParameters::setMeanSiPMNoise(int noise)
{
  m_MeanSiPMNoise = noise;
}

bool KLMScintillatorDigitizationParameters::getEnableConstBkg() const
{
  return m_EnableConstBkg;
}

void KLMScintillatorDigitizationParameters::setEnableConstBkg(bool enable)
{
  m_EnableConstBkg = enable;
}

