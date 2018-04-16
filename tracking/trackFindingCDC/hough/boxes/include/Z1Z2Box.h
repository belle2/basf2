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
    class Z1Z2Box : public Box<DiscreteZ1, DiscreteZ2> {

    private:
      /// Type of the base class
      using Super = Box<DiscreteZ1, DiscreteZ2>;

    public:
      /// Using the constructors of the base class
      using Super::Super;

    public:
      /// Concise getter for the lower z1 bound.
      float getLowerZ1() const
      { return *(getLowerBound<DiscreteZ1>()); }

      /// Concise getter for the upper z1 bound.
      float getUpperZ1() const
      { return *(getUpperBound<DiscreteZ1>()); }

      /// Concise getter for the lower z2 bound.
      float getLowerZ2() const
      { return *(getLowerBound<DiscreteZ2>()); }

      /// Concise getter for the upper z2 bound.
      float getUpperZ2() const
      { return *(getUpperBound<DiscreteZ2>()); }

      //FIXME this interface should not be here, but we need a generically named getter
      float getLowerX() const
      { return getLowerZ1(); }

      float getUpperX() const
      { return getUpperZ1(); }

      float getLowerY() const
      { return getLowerZ2(); }

      float getUpperY() const
      { return getUpperZ2(); }
    };
  }
}
