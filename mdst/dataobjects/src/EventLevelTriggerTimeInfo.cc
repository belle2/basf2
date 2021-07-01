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

#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/HardwareClockSettings.h>
#include <mdst/dbobjects/TTDOffsets.h>

using namespace Belle2;

// get time since the last injection (i.e. the injection-pre-kick signal) in microseconds
double EventLevelTriggerTimeInfo::getTimeSinceLastInjectionInMicroSeconds() const
{
  // use the 'HardwareClockSettings' to get the frequency the clock
  static DBObjPtr<HardwareClockSettings> m_clockSettings;
  // GlobalClockFrequency is in GHz, so we need an additional factor to convert to microseconds
  return ((double)m_timeSinceLastInjection) / (m_clockSettings->getGlobalClockFrequency() * 1e3);
}

// get time since the previous trigger in microseconds
double EventLevelTriggerTimeInfo::getTimeSincePrevTriggerInMicroSeconds() const
{
  // use the 'HardwareClockSettings' to get the frequency the clock
  static DBObjPtr<HardwareClockSettings> m_clockSettings;
  // GlobalClockFrequency is in GHz, so we need an additional factor to convert to microseconds
  return ((double)m_timeSincePrevTrigger) / (m_clockSettings->getGlobalClockFrequency() * 1e3);
}

// get time since the injected bunch passed the detector in clock ticks (FTSW clock)
int EventLevelTriggerTimeInfo::getTimeSinceInjectedBunch() const
{
  // Use the 'TTDOffsets' to get the delay between injection pre-kick signal and passage of the bunch at the IP
  static DBObjPtr<TTDOffsets> m_ttdOffsets;
  int delay = 0;
  if (m_ttdOffsets.isValid()) {
    delay = m_isHER ? m_ttdOffsets->getInjectionDelayHER() : m_ttdOffsets->getInjectionDelayLER();
  } else {
    B2ERROR("Payload 'TTDOffsets' not found. To get the (uncorrected) information from TTD, use 'getTimeSinceLastInjection() or 'getTimeSinceLastInjectionInMicroSeconds()'.");
  }
  return m_timeSinceLastInjection + delay;
}

// get time since the previous trigger in microseconds
double EventLevelTriggerTimeInfo::getTimeSinceInjectedBunchInMicroSeconds() const
{
  // use the 'HardwareClockSettings' to get the frequency the clock
  static DBObjPtr<HardwareClockSettings> m_clockSettings;
  // GlobalClockFrequency is in GHz, so we need an additional factor to convert to microseconds
  return ((double)getTimeSinceInjectedBunch()) / (m_clockSettings->getGlobalClockFrequency() * 1e3);
}

// get the actual (=global) number of the triggered bunch
int EventLevelTriggerTimeInfo::getTriggeredBunchNumberGlobal() const
{
  // Use the 'TTDOffsets' to get the offset between TTD and SKB bunch numbering
  static DBObjPtr<TTDOffsets> m_ttdOffsets;
  int offset = 0;
  if (m_ttdOffsets.isValid()) {
    offset = m_isHER ? m_ttdOffsets->getTriggeredBunchOffsetHER() : m_ttdOffsets->getTriggeredBunchOffsetLER();
  } else {
    B2ERROR("Payload 'TTDOffsets' not found. To get the (uncorrected) information from TTD, use 'getBunchNumber()'.");
  }
  return m_bunchNumber + offset;
}
