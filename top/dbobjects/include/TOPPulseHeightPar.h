/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

namespace Belle2 {

  /**
   * Struct holding the  pulse height parameterizations for the TOP counter.
   * Parameterization is: P(x) = (x/x0)^p1 * exp(-(x/x0)^p2), x0 > 0, p1 >= 0, p2 > 0
   * From channel gain/efficiency calibration
   */
  struct TOPPulseHeightPar {
    float x0 = 0; /**< distribution parameter x0 [ADC counts] */
    float p1 = 0; /**< distribution parameter p1 */
    float p2 = 0; /**< distribution parameter p2 */

    /**
     * Returns a value of non-normalized pulse height distribution at x
     * Multiply with getNorm() to get normalized one
     * @param x argument [ADC counts]
     */
    double getValue(double x) const;

    /**
     * Returns normalization constant
     * @return normalization [1/ADC counts]
     */
    double getNorm() const;

    /**
     * Returns distribution mean
     * @return mean [ADC counts]
     */
    double getMean() const;

    /**
     * Returns distribution standard deviation
     * @return standard deviation [ADC counts]
     */
    double getRMS() const;

    /**
     * Returns the position of distribution maximum
     * @return peak position [ADC counts]
     */
    double getPeakPosition() const;

    /**
     * Returns threshold efficiency
     * @param threshold threshold value [ADC counts]
     * @param rmsNoise r.m.s of electronic noise [ADC counts]
     * @param n number of steps in numerical integration
     * @return threshold efficiency
     */
    double getThresholdEffi(double threshold, double rmsNoise, int n = 100) const;

  };
}
