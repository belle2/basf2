/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Frank Meier                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <utility>

namespace Belle2 {

  /**
   * Sort pair of value and index in descending order with special treatment of NaN
   */
  template <class T>
  auto higherPair = [](const T& a, const T& b) -> bool {
    // always put NaN to the end
    if (std::isnan(a.first)) return false;
    if (std::isnan(b.first)) return true;
    return a.first > b.first;
  };

  /**
   * Sort pair of value and index in ascending order with special treatment of NaN
   */
  template <class T>
  auto lowerPair = [](const T& a, const T& b) -> bool {
    // always put NaN to the end
    if (std::isnan(a.first)) return false;
    if (std::isnan(b.first)) return true;
    return a.first < b.first;
  };

}
