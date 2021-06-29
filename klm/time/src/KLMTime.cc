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
#include <klm/time/KLMTime.h>

using namespace Belle2;

double KLMTime::getScintillatorTime(int ctime, int triggerCTime) const
{
  int cTimeShift = m_KLMTimeConversion->getCTimeShift();
  double tdcFrequency = m_HardwareClockSettings->getKLMTDCFrequency();
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
  return relativeTime / tdcFrequency;
}

std::pair<int, double> KLMTime::getRPCTimes(int ctime, int tdc, int triggerTime) const
{
  double tdcFrequency = m_HardwareClockSettings->getKLMTDCFrequency();
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
                                relativeTime / tdcFrequency);
}

double KLMTime::getTimeSimulation(int tdc, bool scintillator) const
{
  double timeOffset = m_KLMTimeConversion->getTimeOffset();
  double tdcFrequency = m_HardwareClockSettings->getKLMTDCFrequency();
  (void)scintillator;
  return tdc / tdcFrequency + timeOffset;
}

uint16_t KLMTime::getTDCByTime(double time) const
{
  double timeOffset = m_KLMTimeConversion->getTimeOffset();
  double tdcFrequency = m_HardwareClockSettings->getKLMTDCFrequency();
  return (time - timeOffset) * tdcFrequency;
}
