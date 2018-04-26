/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMChannelData.h>

using namespace Belle2;

EKLMChannelData::EKLMChannelData()
{
  m_Active = false;
  m_Pedestal = 0;
  m_PhotoelectronAmplitude = 0;
  m_Threshold = 0;
  m_AdjustmentVoltage = 0;
  m_LookbackWindow = 0;
}

EKLMChannelData::~EKLMChannelData()
{
}

bool EKLMChannelData::getActive() const
{
  return m_Active;
}

void EKLMChannelData::setActive(bool active)
{
  m_Active = active;
}

float EKLMChannelData::getPedestal() const
{
  return m_Pedestal;
}

void EKLMChannelData::setPedestal(float pedestal)
{
  m_Pedestal = pedestal;
}

float EKLMChannelData::getPhotoelectronAmplitude() const
{
  return m_PhotoelectronAmplitude;
}

void EKLMChannelData::setPhotoelectronAmplitude(float photoelectronAmplitude)
{
  m_PhotoelectronAmplitude = photoelectronAmplitude;
}

int EKLMChannelData::getThreshold() const
{
  return m_Threshold;
}

void EKLMChannelData::setThreshold(int threshold)
{
  m_Threshold = threshold;
}

int EKLMChannelData::getAdjustmentVoltage() const
{
  return m_AdjustmentVoltage;
}

void EKLMChannelData::setAdjustmentVoltage(int adjustmentVoltage)
{
  m_AdjustmentVoltage = adjustmentVoltage;
}

int EKLMChannelData::getLookbackWindow() const
{
  return m_LookbackWindow;
}

void EKLMChannelData::setLookbackWindow(int lookbackWindow)
{
  m_LookbackWindow = lookbackWindow;
}

