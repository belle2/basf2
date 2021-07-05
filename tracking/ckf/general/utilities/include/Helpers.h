/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once


namespace Belle2 {

  /// Check capacity of a vector and create a fresh one if capacity too large
  /// If the capacity of a std::vector is very large without being used, it just allocates RAM for no reason, increasing the RAM
  /// usage unnecessarily. In this case, start with a fresh one.
  /// Since std::vector.shrink() or std::vector.shrink_to_fit() not necessarily reduce the capacity in the desired way, create a
  /// temporary vector of the same type, swap them to use the vector at the new location afterwards, and clear the tempoary vector.
  template<class T>
  void checkResizeClear(std::vector<T>& vectorToCheck, uint limit)
  {

    if (vectorToCheck.capacity() > limit) {
      B2DEBUG(29, "Capacity of vector too large, create fresh vector and swap.");
      std::vector<T> tmp;
      std::swap(vectorToCheck, tmp);
      tmp.clear();
    }
    vectorToCheck.clear();
    vectorToCheck.reserve(limit / 4);
  }

}
