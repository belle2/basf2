/*************************************************************************
* basf2 (Belle II Analysis Software Framework)                           *
* Copyright(C) 2020  Belle II Collaboration                              *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Leo Piilonen, Giacomo De Pietro                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

namespace Belle2::Spline {

  /**
   * Construct the spline interpolation coefficients to smooth out a binned histogram.
   * This algorithm is used to determine the KLM transverse-likelihood in klm and b2bii packages.
   * Restrictions:
   *   - there must be more than 3 bins;
   *   - each bin must have the same size (dx);
   *   - the arrays Y, B, C and D must have the same size.
   * @param[in]  n  Number of bins.
   * @param[in]  dx Size of each bin.
   * @param[in]  Y  Value of the bins.
   * @param[out] B  First derivative coefficients.
   * @param[out] C  Second derivative coefficients.
   * @param[out] D  Third derivative coefficients.
   */
  void muidSpline(int n, double dx, double Y[], double B[], double C[], double D[]);

}
