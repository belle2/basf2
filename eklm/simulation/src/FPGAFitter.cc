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
#include <eklm/simulation/FPGAFitter.h>

using namespace Belle2;

enum EKLM::FPGAFitStatus EKLM::FPGAFit(int* amp, int nPoints, double par[5])
{
  const int thr = 100;
  int i;
  int sum;
  int firstSig;
  int lastSig;
  sum = 0;
  firstSig = -1;
  /**
   * Calculate integral above threshold and simultaneously find
   * first and last point above threshold.
   */
  for (i = 0; i < nPoints; i++) {
    if (amp[i] > thr) {
      sum = sum + amp[i];
      if (firstSig == -1)
        firstSig = i;
      lastSig = i;
    }
  }
  if (firstSig == -1)
    return c_FPGANoSignal;
  par[0] = firstSig;
  par[1] = 2;
  par[2] = 2. / (lastSig - firstSig + 1);
  par[3] = sum / (lastSig - firstSig + 1);
  par[4] = 0.;
  return c_FPGASuccessfulFit;
}

