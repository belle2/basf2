/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
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

double KLMTimeConversion::getScintillatorTime(int ctime, int triggerCTime) const
{
  /* Relative time in TDC periods for scintillators. */
  int relativeTime;
  /*
   * All time values were shifted by 2 bits for the phase2 data.
   */
  int correctedCTime = (ctime << m_CTimeShift) & 0xFFFF;
  int correctedTriggerCTime = (triggerCTime << m_CTimeShift) & 0xFFFF;
  /* Scintillator: 16-bit CTIME. */
  if (correctedCTime <= correctedTriggerCTime)
    relativeTime = correctedCTime - correctedTriggerCTime;
  else
    relativeTime = correctedCTime - correctedTriggerCTime - 0x10000;
  /* Get time in TDC periods. */
  relativeTime = (relativeTime << 3);
  return relativeTime * m_TDCPeriod;
}

std::pair<int, double> KLMTimeConversion::getRPCTimes(int ctime, int tdc, int triggerTime) const
{
  /* Relative time in TDC periods for RPC hits. */
  int relativeTime = tdc & 0x7FF;
  /*
   * TDC frequency is 8 times greater than revo9 frequency.
   * The 2 last bits are zero, thus, the precision is 4 * (TDC period).
   * The TDC has 11 bits of dynamic range. Shift the triggerTime by
   * 10 ticks to align the new prompt-time peak with the old
   * TriggerCTime-relative peak (for backward compatibility of calibration).
   */
  int trigger = ((triggerTime - 10) & 0x0FF) << 3;
  if (relativeTime <= trigger)
    relativeTime -= trigger;
  else
    relativeTime -= trigger + 0x800;
  return std::pair<int, double>(ctime - triggerTime, relativeTime * m_TDCPeriod);
}

double KLMTimeConversion::getTimeSimulation(int tdc, bool scintillator) const
{
  (void)scintillator;
  return tdc * m_TDCPeriod + m_TimeOffset;
}

uint16_t KLMTimeConversion::getTDCByTime(double time) const
{
  return (time - m_TimeOffset) / m_TDCPeriod;
}

