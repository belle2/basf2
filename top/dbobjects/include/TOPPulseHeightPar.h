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
  };
}
