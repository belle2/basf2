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
#include <eklm/dataobjects/EKLMFPGAFit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMFPGAFit::EKLMFPGAFit()
{
  m_StartTime = -1;
  m_Amplitude = -1;
  m_BackgroundAmplitude = -1;
  m_MaximalAmplitude = -1;
}

EKLMFPGAFit::~EKLMFPGAFit()
{
}

float EKLMFPGAFit::getStartTime() const
{
  return m_StartTime;
}

void EKLMFPGAFit::setStartTime(float startTime)
{
  m_StartTime = startTime;
}

float EKLMFPGAFit::getAmplitude() const
{
  return m_Amplitude;
}

void EKLMFPGAFit::setAmplitude(float amplitude)
{
  m_Amplitude = amplitude;
}

float EKLMFPGAFit::getBackgroundAmplitude() const
{
  return m_BackgroundAmplitude;
}

void EKLMFPGAFit::setBackgroundAmplitude(float amplitude)
{
  m_BackgroundAmplitude = amplitude;
}

int EKLMFPGAFit::getMaximalAmplitude() const
{
  return m_MaximalAmplitude;
}

void EKLMFPGAFit::setMaximalAmplitude(int amplitude)
{
  m_MaximalAmplitude = amplitude;
}

