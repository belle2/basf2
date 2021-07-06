/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/time/KLMTime.h>

using namespace Belle2;

KLMTime& KLMTime::Instance()
{
  static KLMTime klmElementNumbers;
  return klmElementNumbers;
}

void KLMTime::updateConstants()
{
  m_TDCPeriod =
    1.0 / m_HardwareClockSettings->getClockFrequency(Const::KLM, "TDC");
  m_CTimePeriod = 1.0 / m_HardwareClockSettings->getGlobalClockFrequency();
}

double KLMTime::getScintillatorTime(int ctime, int triggerCTime) const
{
  int cTimeShift = m_KLMTimeConversion->getCTimeShift();
  /* Relative time in TDC periods for scintillators. */
  int relativeTime;
  /*
   * All time values were shifted by 2 bits for the phase2 data.
   */
  int correctedCTime = (ctime << cTimeShift) & 0xFFFF;
  int correctedTriggerCTime = (triggerCTime << cTimeShift) & 0xFFFF;
  /* Scintillator: 16-bit CTIME. */
  if (correctedCTime <= correctedTriggerCTime)
    relativeTime = correctedCTime - correctedTriggerCTime;
  else
    relativeTime = correctedCTime - correctedTriggerCTime - 0x10000;
  /* Get time in TDC periods. */
  relativeTime = (relativeTime << 3);
  return relativeTime * m_TDCPeriod;
}

std::pair<int, double> KLMTime::getRPCTimes(int ctime, int tdc, int triggerTime) const
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
  return std::pair<int, double>(ctime - triggerTime,
                                relativeTime * m_TDCPeriod);
}

double KLMTime::getTimeSimulation(int tdc, bool scintillator) const
{
  double timeOffset = m_KLMTimeConversion->getTimeOffset();
  (void)scintillator;
  return tdc * m_TDCPeriod + timeOffset;
}

uint16_t KLMTime::getTDCByTime(double time) const
{
  double timeOffset = m_KLMTimeConversion->getTimeOffset();
  return (time - timeOffset) / m_TDCPeriod;
}
