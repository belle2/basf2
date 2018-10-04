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
    /// A rectangular domain for the hough space over p and q.
    class PQBox : public Box<DiscreteP, DiscreteQ> {

    private:
      /// Type of the base class
      using Super = Box<DiscreteP, DiscreteQ>;

    public:
      /// Using the constructors of the base class
      using Super::Super;

    public:
      /// Concise getter for the lower p bound.
      float getLowerP() const
      { return *(getLowerBound<DiscreteP>()); }

      /// Concise getter for the upper p bound.
      float getUpperP() const
      { return *(getUpperBound<DiscreteP>()); }

      /// Concise getter for the lower q bound.
      float getLowerQ() const
      { return *(getLowerBound<DiscreteQ>()); }

      /// Concise getter for the upper q bound.
      float getUpperQ() const
      { return *(getUpperBound<DiscreteQ>()); }

      //FIXME this interface should not be here, but we need a generically named getter
      float getLowerX() const
      { return getLowerP(); }

      float getUpperX() const
      { return getUpperP(); }

      float getLowerY() const
      { return getLowerQ(); }

      float getUpperY() const
      { return getUpperQ(); }
    };
  }
}
