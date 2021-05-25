/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/dataobjects/TTDEventInfo.h>

using namespace Belle2;

// get time since the last injection in microseconds
Double_t TTDEventInfo::getTimeSinceLastInjectionInMicroSeconds()
{
  // GlobalClockFrequency is in GHz, so we need an additional factor to convert to microseconds
  return ((Double_t)m_timeSinceLastInjection) / (m_clockSettings->getGlobalClockFrequency() * 1e3);
}

// get time since the previous trigger in microseconds
Double_t TTDEventInfo::getTimeSincePrevTriggerInMicroSeconds()
{
  // GlobalClockFrequency is in GHz, so we need an additional factor to convert to microseconds
  return ((Double_t)m_timeSincePrevTrigger) / (m_clockSettings->getGlobalClockFrequency() * 1e3);
}