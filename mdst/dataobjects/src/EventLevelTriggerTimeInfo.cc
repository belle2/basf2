/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>

using namespace Belle2;

// get time since the last injection (i.e. the injection-pre-kick signal) in microseconds
double EventLevelTriggerTimeInfo::getTimeSinceLastInjectionInMicroSeconds()
{
  // GlobalClockFrequency is in GHz, so we need an additional factor to convert to microseconds
  return ((double)m_timeSinceLastInjection) / (m_clockSettings->getGlobalClockFrequency() * 1e3);
}

// get time since the previous trigger in microseconds
double EventLevelTriggerTimeInfo::getTimeSincePrevTriggerInMicroSeconds()
{
  // GlobalClockFrequency is in GHz, so we need an additional factor to convert to microseconds
  return ((double)m_timeSincePrevTrigger) / (m_clockSettings->getGlobalClockFrequency() * 1e3);
}