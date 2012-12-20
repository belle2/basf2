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

enum EKLM::FPGAFitStatus EKLM::FPGAFit(int* amp, int nPoints)
{
  int i;
  int sum;
  /* Calculate integral. */
  sum = 0;
  for (i = 0; i < nPoints; i++) {
    sum = sum + amp[i];
  }
  if (sum == 0)
    return c_FPGANoSignal;
  return c_FPGASuccessfulFit;
}

