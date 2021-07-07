/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
