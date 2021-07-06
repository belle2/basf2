/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      float getLowerZ0() const
      { return *(getLowerBound<DiscreteZ0>()); }

      /// Concise getter for the upper z0 bound.
      float getUpperZ0() const
      { return *(getUpperBound<DiscreteZ0>()); }

      /// Concise getter for the lower tan lambda bound.
      float getLowerTanLambda() const
      { return *(getLowerBound<DiscreteTanL>()); }

      /// Concise getter for the upper tan lambda bound.
      float getUpperTanLambda() const
      { return *(getUpperBound<DiscreteTanL>()); }

      //FIXME this interface should not be here, but we need a generically named getter
      /// Get the lower z0 bound (alias)
      float getLowerX() const
      { return getLowerZ0(); }

      /// Get the upper z0 bound (alias)
      float getUpperX() const
      { return getUpperZ0(); }

      /// Get the lower tanlambda bound (alias)
      float getLowerY() const
      { return getLowerTanLambda(); }

      /// Get the upper tanlambda bound (alias)
      float getUpperY() const
      { return getUpperTanLambda(); }
    };
  }
}
