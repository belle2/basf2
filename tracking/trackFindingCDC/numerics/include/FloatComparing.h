/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
