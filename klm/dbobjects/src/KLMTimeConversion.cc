/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <klm/dbobjects/KLMTimeConversion.h>

using namespace Belle2;

KLMTimeConversion::KLMTimeConversion()
{
  m_TDCPeriod = 0;
  m_TimeOffset = 0;
}

KLMTimeConversion::~KLMTimeConversion()
{
}

void KLMTimeConversion::setTDCFrequency(double frequency)
{
  m_TDCPeriod = 1.0 / frequency;
}

void KLMTimeConversion::setTimeOffset(double offset)
{
  m_TimeOffset = offset;
}

double KLMTimeConversion::getTime(int ctime, int tdc, int triggerCTime) const
{
  int relativeCTime;
  if (ctime < triggerCTime)
    relativeCTime = ctime - triggerCTime;
  else
    relativeCTime = ctime - triggerCTime - 0x10000;
  return relativeCTime;
}

uint16_t KLMTimeConversion::getTDCByTime(double time) const
{
  return (time - m_TimeOffset) / m_TDCPeriod;
}

