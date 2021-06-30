/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <cmath>

namespace Belle2 {

  /// Convert float or double to long int for more similarity to the FPGA implementation
  /// @param value to be converted
  /// @param power multiply value by 10^power
  inline long convertFloatToInt(double value, int power)
  {
    long factor = (long)pow(10, power);
    return round(factor * value);
  }

} // end namespace Belle2
