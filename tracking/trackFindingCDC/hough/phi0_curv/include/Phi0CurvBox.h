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
    using DiscreteCurvature = DiscreteValue<float>;

    /// Type for the container of the discrete values
    class DiscreteCurvatureArray : public DiscreteValueArray<float> {

    public:
      /// Inheriting the constuctor
      using DiscreteValueArray<float>::DiscreteValueArray;

    public:
      /** Constructs equally spaced discrete curvature values with discrete overlap specification
       *
       *  @param minCurv  Lower bound of the curvature range
       *  @param maxCurv  Upper bound of the curvature range
       *  @param nBins    Total number of final bins to be constructed
       *  @param nWidth   Number of discrete values in each bin
       *                  (counted semi open [start, stop)).
       *  @param nOverlap Number of discrete values that overlapping bins have in common
       *                  (counted semi open [start, stop)).
       */
      static
      DiscreteCurvatureArray
      forCurvatureBinsWithOverlap(float minCurv,
                                  float maxCurv,
                                  size_t nBins,
                                  size_t nWidth,
                                  size_t nOverlap);

      /** Constructs equally spaced discrete curvature values
       *  for searches in the positive curvature range.
       *
       *  The bounds are constructed such that the first bin
       *  corresponds to near zero curvature values.
       *  To avoid cut of effects for hits that spill over to the
       *  subzero curvature values due to their uncertainty, the
       *  lowest bin is slightly expanded such that its is symmetric around zero.
       *  To maintain an equal spacing all other bound are expanded accordingly.
       *
       *  @param maxCurv  Upper bound of the positive curvature range
       *  @param nBins    Total number of final bins to be constructed
       *  @param nWidth   Number of discrete values in each bin
       *                  (counted semi open [start, stop)).
       *  @param nOverlap Number of discrete values that overlapping bins have in common
       *                  (counted semi open [start, stop)).
       */
      static
      DiscreteCurvatureArray
      forPositiveCurvatureBinsWithOverlap(float maxCurv,
                                          size_t nBins,
                                          size_t nWidth = 1,
                                          size_t nOverlap = 0);
    };

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
