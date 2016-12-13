/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TFile.h>
#include <TH1F.h>

/* Belle2 headers. */
#include <eklm/simulation/FPGAFitter.h>
#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLM::FPGAFitter::FPGAFitter(int nPoints)
{
  m_nPoints = nPoints;
}

EKLM::FPGAFitter::~FPGAFitter()
{
}

enum EKLM::FPGAFitStatus EKLM::FPGAFitter::fit(int* amp, int threshold,
                                               EKLMFPGAFit* fitData)
{
  double bg;
  int i, sum, max;
  i = 0;
  sum = 0;
  max = -1;
  /* No data before signal. */
  if (amp[i] > threshold)
    return c_FPGANoSignal;
  /* Time before signal: calculate average value. */
  do {
    sum = sum + amp[i];
    if (amp[i] > max)
      max = amp[i];
    ++i;
    if (i == m_nPoints)
      return c_FPGANoSignal;
  } while (amp[i] <= threshold);
  fitData->setStartTime(i);
  bg = sum / i;
  fitData->setBackgroundAmplitude(bg);
  sum = 0;
  while (i < m_nPoints) {
    sum = sum + amp[i] - bg;
    if (amp[i] > max)
      max = amp[i];
    ++i;
  }
  fitData->setAmplitude(sum);
  fitData->setMaximalAmplitude(max);
  return c_FPGASuccessfulFit;
}

