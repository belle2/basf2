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
#include <tracking/trackFindingCDC/hough/Box.h>
#include <tracking/trackFindingCDC/hough/DiscreteValue.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Tag for z0 direction
    class Z0Tag;

    /// Type for discrete float values in z0 direction
    typedef DiscreteValue<float, Z0Tag> DiscreteZ0;

    /// Tag for z slope direction.
    class TanLambdaTag;

    /// Type for discrete float values in inverse z slope direction
    typedef DiscreteValue<float, TanLambdaTag> DiscreteTanLambda;

    /// Type for the container of the discrete values in z0 direction
    typedef DiscreteZ0::Array DiscreteZ0Array;

    /// Type for the container of the discrete values in inverse z slope direction
    typedef DiscreteTanLambda::Array DiscreteTanLambdaArray;

    /// A rectangular domain for the hough space over phi0 and two dimensional curvature.
    class Z0TanLambdaBox : public Box<DiscreteZ0, DiscreteTanLambda> {

    private:
      /// Type of the base class
      using Super = Box<DiscreteZ0, DiscreteTanLambda>;

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
      { return *(getLowerBound<DiscreteTanLambda>()); }

      /// Concise getter for the lower tan lambda bound.
      const float& getUpperTanLambda() const
      { return *(getUpperBound<DiscreteTanLambda>()); }

      float getUnderLowerTanLambda() const
      { return 2 * getLowerTanLambda() - getUpperTanLambda(); }

      float getOverUpperTanLambda() const
      { return 2 * getUpperTanLambda() - getLowerTanLambda(); }

    };
  } // end namespace TrackFindingCDC
} // end namespace Belle2
