/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/simulation/ScintillatorFirmware.h>

using namespace Belle2;

KLM::ScintillatorFirmware::ScintillatorFirmware(int nPoints) :
  m_nPoints(nPoints)
{
}

KLM::ScintillatorFirmware::~ScintillatorFirmware()
{
}

enum KLM::ScintillatorFirmwareFitStatus KLM::ScintillatorFirmware::fit(
  int* amp, int threshold, KLMScintillatorFirmwareFitResult* fitData)
{
  /*
   * Upper bound of the background region: number of points before threshold
   * crossing.
   */
  const int nPointsSigBg = 10;
  double bg;
  float sigAmp;
  int i, ithr, ibg, sum, bgSum, min;
  sum = 0;
  bgSum = 0;
  /* Get threshold crossing time, sum of ADC outputs and minimal output. */
  ithr = -1;
  min = 0; /* To avoid warning, re-initialized at the first cycle. */
  for (i = 0; i < m_nPoints; i++) {
    if (amp[i] < threshold) {
      if (ithr < 0)
        ithr = i;
    }
    sum = sum + amp[i];
    if (i == 0)
      min = amp[i];
    else if (amp[i] < min)
      min = amp[i];
  }
  /* No signal. */
  if (ithr < 0)
    return c_ScintillatorFirmwareNoSignal;
  /* Region for background (pedestal) level. */
  ibg = std::max(ithr - nPointsSigBg, 0);
  /* Cannot determine background level, no data before signal. */
  /* cppcheck-suppress knownConditionTrueFalse */
  if (ibg == 0)
    return c_ScintillatorFirmwareNoSignal;
  /* Determine background (pedestal) level. */
  for (i = 0; i < ibg; i++)
    bgSum = bgSum + amp[i];
  bg = float(bgSum) / i;
  sigAmp = bg * m_nPoints - sum;
  if (sigAmp < 0)
    sigAmp = 0;
  fitData->setStartTime(ithr);
  fitData->setBackgroundAmplitude(bg);
  fitData->setAmplitude(sigAmp);
  fitData->setMinimalAmplitude(min);
  return c_ScintillatorFirmwareSuccessfulFit;
}

