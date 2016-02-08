/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/hough/boxes/Box.h>
#include <tracking/trackFindingCDC/hough/axes/StandardAxes.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// A rectangular domain for the hough space over phi0 and two dimensional curvature.
    class Z0TanLambdaBox : public Box<DiscreteZ0, DiscreteTanL> {

    private:
      /// Type of the base class
      using Super = Box<DiscreteZ0, DiscreteTanL>;

    public:
      /// Using the constructors of the base class
      using Super::Super;

    public:
      /// Concise getter for the lower z0 bound.
      const float& getLowerZ0() const
      { return *(getLowerBound<DiscreteZ0>()); }

      /// Concise getter for the upper z0 bound.
      const float& getUpperZ0() const
      { return *(getUpperBound<DiscreteZ0>()); }

      float getUnderLowerZ0() const
      { return 2 * getLowerZ0() - getUpperZ0(); }

      float getOverUpperZ0() const
      { return 2 * getUpperZ0() - getLowerZ0(); }

      /// Concise getter for the lower tan lambda bound.
      const float& getLowerTanLambda() const
      { return *(getLowerBound<DiscreteTanL>()); }

      /// Concise getter for the lower tan lambda bound.
      const float& getUpperTanLambda() const
      { return *(getUpperBound<DiscreteTanL>()); }

      float getUnderLowerTanLambda() const
      { return 2 * getLowerTanLambda() - getUpperTanLambda(); }

      float getOverUpperTanLambda() const
      { return 2 * getUpperTanLambda() - getLowerTanLambda(); }

    };
  } // end namespace TrackFindingCDC
} // end namespace Belle2
