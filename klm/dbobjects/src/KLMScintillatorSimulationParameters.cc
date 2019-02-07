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
#include <klm/dbobjects/KLMScintillatorSimulationParameters.h>

using namespace Belle2;

KLMScintillatorSimulationParameters::KLMScintillatorSimulationParameters()
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

KLMScintillatorSimulationParameters::~KLMScintillatorSimulationParameters()
{
}

int KLMScintillatorSimulationParameters::getADCRange() const
{
  return m_ADCRange;
}

void KLMScintillatorSimulationParameters::setADCRange(int range)
{
  m_ADCRange = range;
}

float KLMScintillatorSimulationParameters::getADCSamplingFrequency() const
{
  return 1.0 / m_ADCSamplingTime;
}

void KLMScintillatorSimulationParameters::setADCSamplingFrequency(float frequency)
{
  m_ADCSamplingTime = 1.0 / frequency;
}

float KLMScintillatorSimulationParameters::getADCSamplingTime() const
{
  return m_ADCSamplingTime;
}

void KLMScintillatorSimulationParameters::setADCSamplingTime(float time)
{
  m_ADCSamplingTime = time;
}

int KLMScintillatorSimulationParameters::getNDigitizations() const
{
  return m_NDigitizations;
}

void KLMScintillatorSimulationParameters::setNDigitizations(int digitizations)
{
  m_NDigitizations = digitizations;
}

float KLMScintillatorSimulationParameters::getADCPedestal() const
{
  return m_ADCPedestal;
}

void KLMScintillatorSimulationParameters::setADCPedestal(float pedestal)
{
  m_ADCPedestal = pedestal;
}

float KLMScintillatorSimulationParameters::getADCPEAmplitude() const
{
  return m_ADCPEAmplitude;
}

void KLMScintillatorSimulationParameters::setADCPEAmplitude(float amplitude)
{
  m_ADCPEAmplitude = amplitude;
}

int KLMScintillatorSimulationParameters::getADCThreshold() const
{
  return m_ADCThreshold;
}

void KLMScintillatorSimulationParameters::setADCThreshold(int threshold)
{
  m_ADCThreshold = threshold;
}

int KLMScintillatorSimulationParameters::getADCSaturation() const
{
  return m_ADCSaturation;
}

void KLMScintillatorSimulationParameters::setADCSaturation(int saturation)
{
  m_ADCSaturation = saturation;
}

float KLMScintillatorSimulationParameters::getNPEperMeV() const
{
  return m_NPEperMeV;
}

void KLMScintillatorSimulationParameters::setNPEperMeV(float npe)
{
  m_NPEperMeV = npe;
}

float KLMScintillatorSimulationParameters::getMinCosTheta() const
{
  return m_MinCosTheta;
}

void KLMScintillatorSimulationParameters::setMinCosTheta(float minCosTheta)
{
  m_MinCosTheta = minCosTheta;
}

float KLMScintillatorSimulationParameters::getMirrorReflectiveIndex() const
{
  return m_MirrorReflectiveIndex;
}

void KLMScintillatorSimulationParameters::
setMirrorReflectiveIndex(float reflectiveIndex)
{
  m_MirrorReflectiveIndex = reflectiveIndex;
}

float KLMScintillatorSimulationParameters::getScintillatorDeExcitationTime() const
{
  return m_ScintillatorDeExcitationTime;
}

void KLMScintillatorSimulationParameters::setScintillatorDeExcitationTime(float time)
{
  m_ScintillatorDeExcitationTime = time;
}

float KLMScintillatorSimulationParameters::getFiberDeExcitationTime() const
{
  return m_FiberDeExcitationTime;
}

void KLMScintillatorSimulationParameters::setFiberDeExcitationTime(float time)
{
  m_FiberDeExcitationTime = time;
}

float KLMScintillatorSimulationParameters::getFiberLightSpeed() const
{
  return m_FiberLightSpeed;
}

void KLMScintillatorSimulationParameters::setFiberLightSpeed(float lightSpeed)
{
  m_FiberLightSpeed = lightSpeed;
}

float KLMScintillatorSimulationParameters::getAttenuationLength() const
{
  return m_AttenuationLength;
}

void KLMScintillatorSimulationParameters::setAttenuationLength(float length)
{
  m_AttenuationLength = length;
}

float KLMScintillatorSimulationParameters::getPEAttenuationFrequency() const
{
  return m_PEAttenuationFrequency;
}

void KLMScintillatorSimulationParameters::setPEAttenuationFrequency(float frequency)
{
  m_PEAttenuationFrequency = frequency;
}

float KLMScintillatorSimulationParameters::getMeanSiPMNoise() const
{
  return m_MeanSiPMNoise;
}

void KLMScintillatorSimulationParameters::setMeanSiPMNoise(int noise)
{
  m_MeanSiPMNoise = noise;
}

bool KLMScintillatorSimulationParameters::getEnableConstBkg() const
{
  return m_EnableConstBkg;
}

void KLMScintillatorSimulationParameters::setEnableConstBkg(bool enable)
{
  m_EnableConstBkg = enable;
}

