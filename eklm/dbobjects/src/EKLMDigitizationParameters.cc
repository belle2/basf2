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
}

EKLMDigitizationParameters::~EKLMDigitizationParameters()
{
}

float EKLMDigitizationParameters::getADCRange() const
{
  return m_ADCRange;
}


void EKLMDigitizationParameters::setADCRange(float range)
{
  m_ADCRange = range;
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


float EKLMDigitizationParameters::getADCPedestal() const
{
  return m_ADCPedestal;
}


void EKLMDigitizationParameters::setADCPedestal(float pedestal)
{
  m_ADCPedestal = pedestal;
}


float EKLMDigitizationParameters::getADCPEAmplitude() const
{
  return m_ADCPEAmplitude;
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


float EKLMDigitizationParameters::getTimeResolution() const
{
  return m_TimeResolution;
}


void EKLMDigitizationParameters::setTimeResolution(float resolution)
{
  m_TimeResolution = resolution;
}


