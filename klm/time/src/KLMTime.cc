/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/time/KLMTime.h>
#include <utility>

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

double KLMTime::getScintillatorTime(int ctime, int tdc, int triggerCTime) const
{
  int cTimeShift = m_KLMTimeConversion->getCTimeShift();
  /* Relative time in TDC periods for scintillators. */
  int relativeTime;
  /*
   * All time values were shifted by 2 bits for the phase2 data.
   */
  int correctedCTime = (((ctime << cTimeShift) & 0xFFFF) << 3) + (tdc & 0x7);
  int correctedTriggerCTime = ((triggerCTime << cTimeShift) & 0xFFFF) << 3;
  /* Scintillator: 16-bit CTIME. */
  if (correctedCTime <= correctedTriggerCTime)
    relativeTime = correctedCTime - correctedTriggerCTime;
  else
    relativeTime = correctedCTime - correctedTriggerCTime - (0x10000 << 3);
  /* Get time in TDC periods. */
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

std::pair<int, double> KLMTime::getFTime(int ctime, int triggerTime) const
{
  /*
   * Algorithm (per Chris Ketter, cketter_klm_weekly_251111.pdf):
   * Result is negative (hits in the past from L1 trigger), typically -1200 to +200 TTD clocks.
   */

  /* FRAME9_MAX: RPC ftime counter cycles from 0 to 11519, then resets */
  const int FRAME9_MAX = 11520;

  /* Calculate relative time in TTD clock ticks with overflow correction */
  int relativeTime;
  if (ctime <= triggerTime) {
    /* Normal case: hit before trigger in same frame9 cycle */
    relativeTime = ctime - triggerTime;
  } else {
    /* Overflow: trigger wrapped at 11520, adjust before subtraction */
    relativeTime = ctime - (triggerTime + FRAME9_MAX);
  }

  /* Convert to nanoseconds using TTD period (~7.861 ns) */
  return std::pair<int, double>(relativeTime, relativeTime * m_CTimePeriod);
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
