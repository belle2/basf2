/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <klm/simulation/ScintillatorFirmware.h>
#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

KLM::ScintillatorFirmware::ScintillatorFirmware(int nPoints)
{
  m_nPoints = nPoints;
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
  /* No signal or cannot determine background level. */
  if (ithr < 0 || ithr <= nPointsSigBg)
    return c_ScintillatorFirmwareNoSignal;
  /* Region for background (pedestal) level. */
  ibg = ithr - nPointsSigBg;
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

