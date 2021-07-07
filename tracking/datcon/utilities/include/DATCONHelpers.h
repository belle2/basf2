/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
