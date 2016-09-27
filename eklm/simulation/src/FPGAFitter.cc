/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* System headers. */
#include <math.h>

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

enum EKLM::FPGAFitStatus EKLM::FPGAFitter::fit(int* amp,
                                               struct FPGAFitParams* par)
{
  const int thr = 110;
  int i, sum;
  i = 0;
  sum = 0;
  /* No data before signal. */
  if (amp[i] > thr)
    return c_FPGANoSignal;
  /* Time before signal: calculate average value. */
  do {
    sum = sum + amp[i];
    ++i;
    if (i == m_nPoints)
      return c_FPGANoSignal;
  } while (amp[i] <= thr);
  par->startTime = i;
  par->bgAmplitude = sum / i;
  sum = 0;
  while (i < m_nPoints) {
    sum = sum + amp[i] - par->bgAmplitude;
    ++i;
  }
  par->amplitude = sum;
  return c_FPGASuccessfulFit;
}

