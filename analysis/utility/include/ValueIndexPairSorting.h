/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <utility>

namespace Belle2 {
  namespace ValueIndexPairSorting {

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
}
