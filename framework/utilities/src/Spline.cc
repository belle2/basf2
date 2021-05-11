/*************************************************************************
* basf2 (Belle II Analysis Software Framework)                           *
* Copyright(C) 2020  Belle II Collaboration                              *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Leo Piilonen, Giacomo De Pietro                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

/* Own header. */
#include <framework/utilities/Spline.h>

/* Framework headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

void Spline::muidSpline(int n, double dx, double Y[], double B[], double C[], double D[])
{
  if (n < 2)
    B2FATAL("The spline interpolation can't be applied (histograms with less than 3 bins).");
  D[0] = dx;
  C[1] = (Y[1] - Y[0]) / dx;
  for (int i = 1; i < n - 1; i++) {
    D[i]     = dx;
    B[i]     = dx * 4.0;
    C[i + 1] = (Y[i + 1] - Y[i]) / dx;
    C[i]     = C[i + 1] - C[i];
  }
  B[0]     = -dx;
  B[n - 1] = -dx;
  C[0]     = (C[2]   - C[1]) / 6.0;
  C[n - 1] = -(C[n - 2] - C[n - 3]) / 6.0;
  for (int i = 1; i < n; i++) {
    double temp = dx / B[i - 1];
    B[i] -= temp * dx;
    C[i] -= temp * C[i - 1];
  }
  C[n - 1] /= B[n - 1];
  for (int i = n - 2; i >= 0; i--) {
    C[i] = (C[i] - D[i] * C[i + 1]) / B[i];
  }
  B[n - 1] = (Y[n - 1] - Y[n - 2]) / dx + (C[n - 2] + C[n - 1] * 2.0) * dx;
  for (int i = 0; i < n - 1; i++) {
    B[i] = (Y[i + 1] - Y[i]) / dx - (C[i + 1] + C[i] * 2.0) * dx;
    D[i] = (C[i + 1] - C[i]) / dx;
    C[i] = C[i] * 3.0;
  }
  C[n - 1] = C[n - 1] * 3.0;
  D[n - 1] = D[n - 2];
}
