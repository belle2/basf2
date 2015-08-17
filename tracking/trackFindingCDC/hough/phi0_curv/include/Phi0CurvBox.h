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

#include <tracking/trackFindingCDC/hough/Box.h>
#include <tracking/trackFindingCDC/hough/perigee/DiscretePhi0.h>
#include <tracking/trackFindingCDC/hough/perigee/DiscreteCurv.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A rectangular domain for the hough space over phi0 and two dimensional curvature.
    class Phi0CurvBox : public Box<DiscretePhi0, DiscreteCurv> {

    private:
      /// Type of the base class
      using Super = Box<DiscretePhi0, DiscreteCurv >;

    public:
      /// Using the constructors of the base class
      using Super::Super;

    public:
      /// Concise getter for the lower phi0 bound.
      const Vector2D& getLowerPhi0Vec() const
      { return static_cast<const Vector2D&>(getLowerBound<0>()); }

      /// Concise getter for the upper phi0 bound.
      const Vector2D& getUpperPhi0Vec() const
      { return static_cast<const Vector2D&>(getUpperBound<0>()); }

      /// Concise getter for the lower curvature bound.
      const float& getLowerCurv() const
      { return static_cast<const float&>(getLowerBound<1>()); }

      /// Concise getter for the lower curvature bound.
      const float& getUpperCurv() const
      { return static_cast<const float&>(getUpperBound<1>()); }
    };
  } // end namespace TrackFindingCDC
} // end namespace Belle2
