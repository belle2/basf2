/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                 *
 * Copyright(C) 2016  Belle II Collaboration                          *
 *                                                                    *
 * Author: The Belle II Collaboration                                 *
 * Contributors: Yinghui GUAN                                         *
 *                                                                    *
 * This software is provided "as is" without any warranty.            *
 **************************************************************************/

#include <bklm/dbobjects/BKLMScinDigitizationParams.h>

using namespace Belle2;

BKLMScinDigitizationParams::BKLMScinDigitizationParams()
{
  m_ADCRange = 0;
  m_ADCSamplingTime = 0;
  m_NDigitizations = 0;
  //m_ADCPedestal = 0;
  //m_ADCPEAmplitude = 0;
  //m_ADCSaturation = 0;
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
  m_timeResolution = 0;
}

BKLMScinDigitizationParams::~BKLMScinDigitizationParams()
{
}

int BKLMScinDigitizationParams::getADCRange() const
{
  return m_ADCRange;
}

void BKLMScinDigitizationParams::setADCRange(int range)
{
  m_ADCRange = range;
}

float BKLMScinDigitizationParams::getADCSamplingTime() const
{
  return m_ADCSamplingTime;
}

void BKLMScinDigitizationParams::setADCSamplingTime(float time)
{
  m_ADCSamplingTime = time;
}

int BKLMScinDigitizationParams::getNDigitizations() const
{
  return m_NDigitizations;
}

void BKLMScinDigitizationParams::setNDigitizations(int digitizations)
{
  m_NDigitizations = digitizations;
}

float BKLMScinDigitizationParams::getNPEperMeV() const
{
  return m_NPEperMeV;
}

void BKLMScinDigitizationParams::setNPEperMeV(float npe)
{
  m_NPEperMeV = npe;
}

float BKLMScinDigitizationParams::getMinCosTheta() const
{
  return m_MinCosTheta;
}

void BKLMScinDigitizationParams::setMinCosTheta(float minCosTheta)
{
  m_MinCosTheta = minCosTheta;
}

float BKLMScinDigitizationParams::getMirrorReflectiveIndex() const
{
  return m_MirrorReflectiveIndex;
}

void BKLMScinDigitizationParams::setMirrorReflectiveIndex(float reflectiveIndex)
{
  m_MirrorReflectiveIndex = reflectiveIndex;
}

float BKLMScinDigitizationParams::getScintillatorDeExcitationTime() const
{
  return m_ScintillatorDeExcitationTime;
}

void BKLMScinDigitizationParams::setScintillatorDeExcitationTime(float time)
{
  m_ScintillatorDeExcitationTime = time;
}

float BKLMScinDigitizationParams::getFiberDeExcitationTime() const
{
  return m_FiberDeExcitationTime;
}

void BKLMScinDigitizationParams::setFiberDeExcitationTime(float time)
{
  m_FiberDeExcitationTime = time;
}

float BKLMScinDigitizationParams::getFiberLightSpeed() const
{
  return m_FiberLightSpeed;
}

void BKLMScinDigitizationParams::setFiberLightSpeed(float lightSpeed)
{
  m_FiberLightSpeed = lightSpeed;
}

float BKLMScinDigitizationParams::getAttenuationLength() const
{
  return m_AttenuationLength;
}

void BKLMScinDigitizationParams::setAttenuationLength(float length)
{
  m_AttenuationLength = length;
}

float BKLMScinDigitizationParams::getPEAttenuationFrequency() const
{
  return m_PEAttenuationFrequency;
}

void BKLMScinDigitizationParams::setPEAttenuationFrequency(float frequency)
{
  m_PEAttenuationFrequency = frequency;
}

float BKLMScinDigitizationParams::getMeanSiPMNoise() const
{
  return m_MeanSiPMNoise;
}

void BKLMScinDigitizationParams::setMeanSiPMNoise(int noise)
{
  m_MeanSiPMNoise = noise;
}

bool BKLMScinDigitizationParams::getEnableConstBkg() const
{
  return m_EnableConstBkg;
}

void BKLMScinDigitizationParams::setEnableConstBkg(bool enable)
{
  m_EnableConstBkg = enable;
}

