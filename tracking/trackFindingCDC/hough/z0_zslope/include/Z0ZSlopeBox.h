/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/hough/Box.h>
#include <tracking/trackFindingCDC/hough/DiscreteValues.h>

// For better readability we use z slope instead of inverse z slope everywhere!
namespace Belle2 {
  namespace TrackFindingCDC {
    /// Type for discrete float values in z0 direction
    typedef DiscreteValue<float> DiscreteZ0;

    /// Type for discrete float values in inverse z slope direction
    typedef DiscreteValue<float> DiscreteZSlope;

    /// Type for the container of the discrete values in z0 direction
    typedef DiscreteValueArray<float> DiscreteZ0Array;

    /// Type for the container of the discrete values in inverse z slope direction
    typedef DiscreteValueArray<float> DiscreteZSlopeArray;

    /// A rectangular domain for the hough space over phi0 and two dimensional curvature.
    class Z0ZSlopeBox : public Box<DiscreteZ0, DiscreteZSlope> {

    private:
      /// Type of the base class
      using Super = Box<DiscreteZ0, DiscreteZSlope>;

    public:
      /// Using the constructors of the base class
      using Super::Super;

    public:
      /// Concise getter for the lower z0 bound.
      const float& getLowerZ0() const
      { return getLowerBound<0>().getValue(); }

      /// Concise getter for the upper z0 bound.
      const float& getUpperZ0() const
      { return getUpperBound<0>().getValue(); }

      /// Concise getter for the lower inverse z0 slope bound.
      const float& getLowerZSlope() const
      { return getLowerBound<1>().getValue(); }

      /// Concise getter for the lower inverse z0 slope bound.
      const float& getUpperZSlope() const
      { return getUpperBound<1>().getValue(); }
    };
  } // end namespace TrackFindingCDC
} // end namespace Belle2
