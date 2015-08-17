/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/hough/DiscreteAngles.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type to represent an array of discrete phi0 values
    using DiscretePhi0Array = DiscreteAngleArray;

    /// Type to represent a discrete phi0 value
    class DiscretePhi0 : public DiscreteAngle {
    public:
      /// Inheriting the constructor of the base class
      using DiscreteAngle::DiscreteAngle;

      /// The type of the array which contains the underlying values.
      using Array = DiscretePhi0Array;

    public:
      /// Extract the range from an array providing the discrete values.
      static std::pair<DiscretePhi0, DiscretePhi0> getRange(const DiscretePhi0Array& valueArray)
      { return {DiscretePhi0(valueArray.front()), DiscretePhi0(valueArray.back())}; }
    };
  } // end namespace TrackFindingCDC
} // end namespace Belle2
