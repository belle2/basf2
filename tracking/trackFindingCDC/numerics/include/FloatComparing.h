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
  namespace TrackFindingCDC {

    /// Extended equality for floating point values such at all NANs are identical to each other
    template<class AFloat>
    bool identicalFloat(AFloat lhs, AFloat rhs)
    {
      return rhs == lhs or (std::isnan(lhs) and std::isnan(rhs));
    }

    /// Safe less (<) comparision for floating point values putting NAN at the high end
    template<class AFloat>
    bool lessFloatHighNaN(AFloat lhs, AFloat rhs)
    {
      if (std::isnan(lhs)) return false;
      if (std::isnan(rhs)) return true;
      return lhs < rhs;
    }

    /// Safe greater (>) comparision for floating point values putting NAN at the high end
    template<class AFloat>
    bool greaterFloatHighNaN(AFloat lhs, AFloat rhs)
    {
      if (std::isnan(rhs)) return false;
      if (std::isnan(lhs)) return true;
      return lhs > rhs;
    }
  }
}
