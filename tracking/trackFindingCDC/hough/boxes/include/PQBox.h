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
      /// Get the lower p bound (alias)
      float getLowerX() const
      { return getLowerP(); }

      /// Get the upper p bound (alias)
      float getUpperX() const
      { return getUpperP(); }

      /// Get the lower q bound (alias)
      float getLowerY() const
      { return getLowerQ(); }

      /// Get the upper q bound (alias)
      float getUpperY() const
      { return getUpperQ(); }
    };
  }
}
