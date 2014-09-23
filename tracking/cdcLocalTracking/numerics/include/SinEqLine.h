/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef SINEQLINE_H_
#define SINEQLINE_H_

#include "numerics.h"

#include <tracking/cdcLocalTracking/geometry/Line2D.h>
#include <tracking/cdcLocalTracking/geometry/Vector2D.h>

#include <tracking/cdcLocalTracking/typedefs/InfoTypes.h>
#include <tracking/cdcLocalTracking/typedefs/SignType.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>

#include <framework/logging/Logger.h>

#include <utility>
#include <math.h>

namespace Belle2 {

  namespace CDCLocalTracking {

    /** Helper class to calculate roots for the function f(x) = sin x - slope * x - intercept.
     *  Solves the equation sin x = slope * x + intercept by netwons method on the function f(x) = sin x - slope * x - intercept.
     *  There can be zero, infinitly many solutions and anything inbetween depending on the value of slope and intercept.
     *  To find solutions we expliot that the local exterma of the function f(x) can be found easily and that maximally one solutions can be found between consecutive extrema.
     *  There is exactly one local extermum in each half period of sin(x) and local extrema are therefore addressed by there half period index.
     *  Each possible solution of the equation is than addressed by the index of the closest *smaller* local extrema.
     *  However only a range of indices corresponds to realized solutions. For non existent solutions NAN is returned.
     *  Note that for fabs(slope) >= 1 there are no more local maxima. In this case only a single solution exists, which is always returned for any index.
     */
    class SinEqLine : public UsedTObject {

    public:
      /// Constructor taking the line that shall be superimposed with the sin curve.
      SinEqLine(const Line2D& line2D) :
        m_slope(line2D.slope()),
        m_intercept(line2D.intercept())
      {;}

      /// Constructor taking the slope and intercept of the line that shall be superimposed with the sin curve.
      SinEqLine(const FloatType& slope, const FloatType& intercept) :
        m_slope(slope),
        m_intercept(intercept)
      {;}


      /// Interpreting as the function f this method carries out the translation from x to y coordinates.
      FloatType map(const FloatType& x) const
      { return sin(x) - getSlope() * x - getIntercept(); }

      /// Interpreting as the function f this method calculates the gradient as need in Newtons algorithms.
      FloatType gradient(const FloatType& x) const
      { return cos(x) - getSlope(); }

      /// Returns the half period index in which the x position is located.
      Index getIHalfPeriod(const FloatType& x) const
      { return floor(x / PI); }

      /* Computes the positive solution that has the smallest value of x.
      The additional parameter serves as a criterion to abbort the search if the solutions is further away than the specified half period.
      */
      FloatType computeSmallestPositiveRoot(const Index& maxIHalfPeriod = 5) const;


      /// Computes the solution that is addressed by the given half period index.
      FloatType computeRootLargerThanExtemumInHalfPeriod(const Index& iHalfPeriod) const;

      /// Compute single solution in the case that fabs(slope) >= 1.
      FloatType computeRootForLargeSlope() const;

      /// Computes the solution in between the given x values. The x values are generally choosen consecutive local extermas.
      FloatType computeRootInInterval(FloatType lowerX, FloatType upperX) const;

    private:
      /// Shrinking method of the newton algorithm return the next candidate root.
      FloatType newtonX(const Vector2D& pos) const;

      /// Fall back shrinking method to the secant algorithm
      static FloatType secantX(const Vector2D& lower, const Vector2D& upper);

      /// Simple fall back shrinking method using trivial devision of the intervall.
      static FloatType middleX(const Vector2D& lower, const Vector2D& upper);

      /// Replaces the lower or upper bound inplace if the next candidate position is valid and within the intervall. Returns true on success.
      static bool updateBounds(Vector2D& lower, Vector2D& upper, const Vector2D& next);

      /// Check is next position is within the intervall given by lower and upper.
      static bool isBetween(const Vector2D& lower, const Vector2D& next, const Vector2D& upper)
      { return lower.x() < next.x() and next.x() < upper.x(); }

      /// Check if the intervall has shrunk close enough to the solution.
      static bool isConverged(const Vector2D& lower, const Vector2D& upper) {
        return fabs(lower.y()) < 10e-7 or fabs(upper.y()) < 10e-7;
      }

      /// Returns the better solution x from the bounds of the intervall.
      static FloatType getConvergedBound(const Vector2D& lower, const Vector2D& upper) {
        if (fabs(lower.y()) <= fabs(upper.y())) {
          return lower.x();

        } else if (fabs(lower.y()) > fabs(upper.y())) {
          return upper.x();

        } else {
          return NAN;

        }
      }

    public:
      /// Checks if the function changes sign in the intervall
      static bool changesSign(const Vector2D& lower, const Vector2D& upper)
      { return (lower.y() > 0 and upper.y() < 0) or (lower.y() < 0 and upper.y() > 0); }

      /// Determines if the function is increasing or decreasing in the intervall.
      static IncDecInfo getIncDecInfo(const Vector2D& lower, const Vector2D& upper) {
        if (lower.y() < upper.y()) return INCREASING;
        else if (lower.y() > upper.y()) return DECREASING;
        else if (lower.y() == upper.y()) return CONSTANT;
        else return INVALID_INFO;
      }

    public:
      /// Get the local extermum that is located in the half period indicated by the given index.
      FloatType computeExtremumXInHalfPeriod(const Index& iHalfPeriod) const;

      /// Helper function to translate the index of the half period to index of the containing period.
      static Index getIPeriodFromIHalfPeriod(const Index& iHalfPeriod)
      { return isEven(iHalfPeriod) ? iHalfPeriod / 2 : (iHalfPeriod - 1) / 2; }

    public:
      /// Indicates that the slope is so large such that the function has no local exterma.
      bool hasLargeSlope() const
      { return fabs(getSlope()) >= 1; }

      /// Getter for the slope
      const FloatType& getSlope() const
      { return m_slope; }

      /// Getter for the intercept
      const FloatType& getIntercept() const
      { return m_intercept; }

    private:
      /// Memory for the slope.
      FloatType m_slope;

      /// Memory for the intercept.
      FloatType m_intercept;

      /// ROOT Macro to make SinEqLine a ROOT class.
      ClassDefInCDCLocalTracking(SinEqLine, 1);

    };

  } // end namespace CDCLocalTracking
} // end namespace Belle2

#endif // SINEQLINE_H_
