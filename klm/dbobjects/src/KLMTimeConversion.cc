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

KLMTimeConversion::KLMTimeConversion() :
  m_TDCPeriod(0),
  m_TimeOffset(0),
  m_CTimeShift(0)
{
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

void KLMTimeConversion::setCTimeShift(int shift)
{
  m_CTimeShift = shift;
}

double KLMTimeConversion::getTime(int ctime, int tdc, int triggerCTime,
                                  bool scintillator) const
{
  /* Relative time in TDC periods. */
  int relativeTime;
  /*
   * All time values were shifted by 2 bits for the phase2 data.
   */
  int correctedCTime = (ctime << m_CTimeShift) & 0xFFFF;
  int correctedTriggerCTime = (triggerCTime << m_CTimeShift) & 0xFFFF;
  int correctedTdc = (tdc << m_CTimeShift) & 0x07FF;
  if (scintillator) {
    /* Scintillator: 16-bit CTIME. */
    if (correctedCTime <= correctedTriggerCTime)
      relativeTime = correctedCTime - correctedTriggerCTime;
    else
      relativeTime = correctedCTime - correctedTriggerCTime - 0x10000;
    /* Get time in TDC periods. */
    relativeTime = (relativeTime << 3);
  } else {
    /*
     * RPC: 11-bit TDC, but reduced to 10, since the first bit does not depend
     * on trigger CTIME. TDC frequency is 8 times greater than CTIME frequency.
     * The 2 last bits are zero, thus, the precision is 4 * (TDC period).
     */
    int trigger = (correctedTriggerCTime & 0x7F) << 3;
    int tdcReduced = correctedTdc & 0x3FF;
    if (tdcReduced <= trigger)
      relativeTime = tdcReduced - trigger;
    else
      relativeTime = tdcReduced - trigger - 0x400;
  }
  return relativeTime * m_TDCPeriod;
}

uint16_t KLMTimeConversion::getTDCByTime(double time) const
{
  return (time - m_TimeOffset) / m_TDCPeriod;
}

