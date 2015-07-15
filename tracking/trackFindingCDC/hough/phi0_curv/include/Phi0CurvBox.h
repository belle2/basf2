/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/hough/Box.h>
#include <tracking/trackFindingCDC/hough/DiscreteAngles.h>
#include <tracking/trackFindingCDC/hough/DiscreteValues.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Type for discrete float values
    typedef DiscreteValue<float> DiscreteCurvature;

    /// Type for the container of the discrete values
    typedef DiscreteValueArray<float> DiscreteCurvatureArray;

    /// A rectangular domain for the hough space over phi0 and two dimensional curvature.
    class Phi0CurvBox : public Box<DiscreteAngle, DiscreteCurvature > {

    private:
      /// Type of the base class
      using Super = Box<DiscreteAngle, DiscreteCurvature >;

    public:
      /// Using the constructors of the base class
      using Super::Super;

    public:
      /// Concise getter for the lower phi0 bound.
      const Vector2D& getLowerPhi0Vec() const
      { return getLowerBound<0>().getAngleVec(); }

      /// Concise getter for the upper phi0 bound.
      const Vector2D& getUpperPhi0Vec() const
      { return getUpperBound<0>().getAngleVec(); }

      /// Concise getter for the lower curvature bound.
      const float& getLowerCurv() const
      { return getLowerBound<1>().getValue(); }

      /// Concise getter for the lower curvature bound.
      const float& getUpperCurv() const
      { return getUpperBound<1>().getValue(); }
    };
  } // end namespace TrackFindingCDC
} // end namespace Belle2
