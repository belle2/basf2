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
#include <framework/logging/Logger.h>
#include <klm/dataobjects/KLMScintillatorFirmwareFitResult.h>

using namespace Belle2;

KLMScintillatorFirmwareFitResult::KLMScintillatorFirmwareFitResult()
{
  m_StartTime = -1;
  m_Amplitude = -1;
  m_BackgroundAmplitude = -1;
  m_MinimalAmplitude = -1;
}

KLMScintillatorFirmwareFitResult::~KLMScintillatorFirmwareFitResult()
{
}

int KLMScintillatorFirmwareFitResult::getStartTime() const
{
  return m_StartTime;
}

void KLMScintillatorFirmwareFitResult::setStartTime(int startTime)
{
  m_StartTime = startTime;
}

float KLMScintillatorFirmwareFitResult::getAmplitude() const
{
  return m_Amplitude;
}

void KLMScintillatorFirmwareFitResult::setAmplitude(float amplitude)
{
  m_Amplitude = amplitude;
}

float KLMScintillatorFirmwareFitResult::getBackgroundAmplitude() const
{
  return m_BackgroundAmplitude;
}

void KLMScintillatorFirmwareFitResult::setBackgroundAmplitude(float amplitude)
{
  m_BackgroundAmplitude = amplitude;
}

int KLMScintillatorFirmwareFitResult::getMinimalAmplitude() const
{
  return m_MinimalAmplitude;
}

void KLMScintillatorFirmwareFitResult::setMinimalAmplitude(int amplitude)
{
  m_MinimalAmplitude = amplitude;
}

