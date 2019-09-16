/***************************************************************************
  * BASF2 (Belle Analysis Framework 2)                                     *
  * Copyright(C) 2017 - Belle II Collaboration                             *
  *                                                                        *
  * Author: The Belle II Collaboration                                     *
  * Contributors: Marko Staric                                             *
  *                                                                        *
  * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <framework/logging/Logger.h>

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
