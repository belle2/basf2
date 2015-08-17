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

    /// Type to represent a discrete phi0 value
    class DiscretePhi0 : public DiscreteAngle {
    public:
      /// Inheriting the constructor of the base class
      using DiscreteAngle::DiscreteAngle;

    };

    //ing DiscretePhi0Array = DiscreteAngleArray;

    /// Type to represent an array of discrete phi0 values
    class DiscretePhi0Array : public DiscreteAngleArray {

    private:
      /// Type of the base class.
      typedef DiscreteAngleArray Super;

    public:
      /// Inheriting the constructors of the base class
      using Super::Super;

      using Super::operator=;

    public:
      /// Getter for the first of the discrete phi0 values
      DiscretePhi0 front() const
      { return DiscretePhi0(Super::front()); }

      /// Getter for the last of the discrete phi0 values
      DiscretePhi0 back() const
      { return DiscretePhi0(Super::back()); }

      /// Getter for the complete phi0 range
      std::pair<DiscretePhi0, DiscretePhi0> getRange()
      { return std::pair<DiscretePhi0, DiscretePhi0>(front(), back()); }

    };
  } // end namespace TrackFindingCDC
} // end namespace Belle2
