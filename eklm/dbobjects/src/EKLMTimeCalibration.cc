/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMTimeCalibration.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMTimeCalibration::EKLMTimeCalibration()
{
  m_EffectiveLightSpeed = 0;
  m_AmplitudeTimeConstant = 0;
}

EKLMTimeCalibration::~EKLMTimeCalibration()
{
}

void EKLMTimeCalibration::
setTimeCalibrationData(uint16_t strip, EKLMTimeCalibrationData* dat)
{
  std::map<uint16_t, EKLMTimeCalibrationData>::iterator it;
  it = m_data.find(strip);
  if (it == m_data.end())
    m_data.insert(std::pair<uint16_t, EKLMTimeCalibrationData>(strip, *dat));
  else
    B2WARNING("TimeCalibration data for the strip already exists.");
}

const EKLMTimeCalibrationData*
EKLMTimeCalibration::getTimeCalibrationData(uint16_t strip) const
{
  std::map<uint16_t, EKLMTimeCalibrationData>::const_iterator it;
  it = m_data.find(strip);
  if (it == m_data.end())
    return NULL;
  return &(it->second);
}

void EKLMTimeCalibration::cleanTimeCalibrationData()
{
  m_data.clear();
}

float EKLMTimeCalibration::getEffectiveLightSpeed() const
{
  return m_EffectiveLightSpeed;
}

void EKLMTimeCalibration::setEffectiveLightSpeed(float lightSpeed)
{
  m_EffectiveLightSpeed = lightSpeed;
}

float EKLMTimeCalibration::getAmplitudeTimeConstant() const
{
  return m_AmplitudeTimeConstant;
}

void EKLMTimeCalibration::setAmplitudeTimeConstant(float amplitudeTimeConstant)
{
  m_AmplitudeTimeConstant = amplitudeTimeConstant;
}

