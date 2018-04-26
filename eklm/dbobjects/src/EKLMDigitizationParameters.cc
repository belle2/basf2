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
#include <eklm/dbobjects/EKLMDigitizationParameters.h>

using namespace Belle2;

EKLMDigitizationParameters::EKLMDigitizationParameters()
{
  m_ADCRange = 0;
  m_ADCSamplingTime = 0;
  m_NDigitizations = 0;
  m_ADCPedestal = 0;
  m_ADCPEAmplitude = 0;
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

EKLMDigitizationParameters::~EKLMDigitizationParameters()
{
}

int EKLMDigitizationParameters::getADCRange() const
{
  return m_ADCRange;
}

void EKLMDigitizationParameters::setADCRange(int range)
{
  m_ADCRange = range;
}

float EKLMDigitizationParameters::getADCSamplingFrequency() const
{
  return 1.0 / m_ADCSamplingTime;
}

void EKLMDigitizationParameters::setADCSamplingFrequency(float frequency)
{
  m_ADCSamplingTime = 1.0 / frequency;
}

float EKLMDigitizationParameters::getADCSamplingTime() const
{
  return m_ADCSamplingTime;
}

void EKLMDigitizationParameters::setADCSamplingTime(float time)
{
  m_ADCSamplingTime = time;
}

int EKLMDigitizationParameters::getNDigitizations() const
{
  return m_NDigitizations;
}

void EKLMDigitizationParameters::setNDigitizations(int digitizations)
{
  m_NDigitizations = digitizations;
}

void EKLMDigitizationParameters::setADCPedestal(float pedestal)
{
  m_ADCPedestal = pedestal;
}

void EKLMDigitizationParameters::setADCPEAmplitude(float amplitude)
{
  m_ADCPEAmplitude = amplitude;
}

float EKLMDigitizationParameters::getADCSaturation() const
{
  return m_ADCSaturation;
}

void EKLMDigitizationParameters::setADCSaturation(float saturation)
{
  m_ADCSaturation = saturation;
}

float EKLMDigitizationParameters::getNPEperMeV() const
{
  return m_NPEperMeV;
}

void EKLMDigitizationParameters::setNPEperMeV(float npe)
{
  m_NPEperMeV = npe;
}

float EKLMDigitizationParameters::getMinCosTheta() const
{
  return m_MinCosTheta;
}

void EKLMDigitizationParameters::setMinCosTheta(float minCosTheta)
{
  m_MinCosTheta = minCosTheta;
}

float EKLMDigitizationParameters::getMirrorReflectiveIndex() const
{
  return m_MirrorReflectiveIndex;
}

void EKLMDigitizationParameters::
setMirrorReflectiveIndex(float reflectiveIndex)
{
  m_MirrorReflectiveIndex = reflectiveIndex;
}

float EKLMDigitizationParameters::getScintillatorDeExcitationTime() const
{
  return m_ScintillatorDeExcitationTime;
}

void EKLMDigitizationParameters::setScintillatorDeExcitationTime(float time)
{
  m_ScintillatorDeExcitationTime = time;
}

float EKLMDigitizationParameters::getFiberDeExcitationTime() const
{
  return m_FiberDeExcitationTime;
}

void EKLMDigitizationParameters::setFiberDeExcitationTime(float time)
{
  m_FiberDeExcitationTime = time;
}

float EKLMDigitizationParameters::getFiberLightSpeed() const
{
  return m_FiberLightSpeed;
}

void EKLMDigitizationParameters::setFiberLightSpeed(float lightSpeed)
{
  m_FiberLightSpeed = lightSpeed;
}

float EKLMDigitizationParameters::getAttenuationLength() const
{
  return m_AttenuationLength;
}

void EKLMDigitizationParameters::setAttenuationLength(float length)
{
  m_AttenuationLength = length;
}

float EKLMDigitizationParameters::getPEAttenuationFrequency() const
{
  return m_PEAttenuationFrequency;
}

void EKLMDigitizationParameters::setPEAttenuationFrequency(float frequency)
{
  m_PEAttenuationFrequency = frequency;
}

float EKLMDigitizationParameters::getMeanSiPMNoise() const
{
  return m_MeanSiPMNoise;
}

void EKLMDigitizationParameters::setMeanSiPMNoise(int noise)
{
  m_MeanSiPMNoise = noise;
}

bool EKLMDigitizationParameters::getEnableConstBkg() const
{
  return m_EnableConstBkg;
}

void EKLMDigitizationParameters::setEnableConstBkg(bool enable)
{
  m_EnableConstBkg = enable;
}

