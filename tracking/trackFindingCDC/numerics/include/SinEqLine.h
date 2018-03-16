/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Modulo.h>

#include <tracking/trackFindingCDC/geometry/Line2D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/numerics/EIncDec.h>
#include <tracking/trackFindingCDC/numerics/ESign.h>

#include <utility>
#include <cmath>

namespace Belle2 {

  namespace TrackFindingCDC {

    /** Helper class to calculate roots for the function f(x) = sin x - slope * x - intercept.
     *  Solves the equation sin x = slope * x + intercept by netwons method on the function f(x) = sin x - slope * x - intercept.
     *  There can be zero, infinitly many solutions and anything inbetween depending on the value of slope and intercept.
     *  To find solutions we expliot that the local exterma of the function f(x) can be found easily and that maximally one solutions can be found between consecutive extrema.
     *  There is exactly one local extermum in each half period of sin(x) and local extrema are therefore addressed by there half period index.
     *  Each possible solution of the equation is than addressed by the index of the closest *smaller* local extrema.
     *  However only a range of indices corresponds to realized solutions. For non existent solutions NAN is returned.
     *  Note that for fabs(slope) >= 1 there are no more local maxima. In this case only a single solution exists, which is always returned for any index.
     */
    class SinEqLine  {

    public:
      /// Default constructor initializing slope and intercept to zero.
      SinEqLine() :
        m_slope(0.0),
        m_intercept(0.0)
      {}


      /// Constructor taking the line that shall be superimposed with the sin curve.
      explicit SinEqLine(const Line2D& line2D) :
        m_slope(line2D.slope()),
        m_intercept(line2D.intercept())
      {}

      /// Constructor taking the slope and intercept of the line that shall be superimposed with the sin curve.
      SinEqLine(const double slope, const double intercept) :
        m_slope(slope),
        m_intercept(intercept)
      {}


      /// Interpreting as the function f this method carries out the translation from x to y coordinates.
      double map(const double x) const
      { return sin(x) - getSlope() * x - getIntercept(); }

      /// Interpreting as the function f this method calculates the gradient as need in Newtons algorithms.
      double gradient(const double x) const
      { return cos(x) - getSlope(); }

      /// Returns the half period index in which the x position is located.
      int getIHalfPeriod(const double x) const
      { return floor(x / M_PI); }

      /* Computes the positive solution that has the smallest value of x.
      The additional parameter serves as a criterion to abbort the search if the solutions is further away than the specified half period.
      */
      double computeSmallestPositiveRoot(int maxIHalfPeriod = 5) const;


      /// Computes the solution that is addressed by the given half period index.
      double computeRootLargerThanExtemumInHalfPeriod(int iHalfPeriod) const;

      /// Compute single solution in the case that fabs(slope) >= 1.
      double computeRootForLargeSlope() const;

      /// Computes the solution in between the given x values. The x values are generally choosen consecutive local extermas.
      double computeRootInInterval(double lowerX, double upperX) const;

    private:
      /// Shrinking method of the newton algorithm return the next candidate root.
      double newtonX(const Vector2D& pos) const;

      /// Fall back shrinking method to the secant algorithm
      static double secantX(const Vector2D& lower, const Vector2D& upper);

      /// Simple fall back shrinking method using trivial devision of the intervall.
      static double middleX(const Vector2D& lower, const Vector2D& upper);

      /// Replaces the lower or upper bound inplace if the next candidate position is valid and within the intervall. Returns true on success.
      static bool updateBounds(Vector2D& lower, Vector2D& upper, const Vector2D& next);

      /// Check is next position is within the intervall given by lower and upper.
      static bool isBetween(const Vector2D& lower, const Vector2D& next, const Vector2D& upper)
      { return lower.x() < next.x() and next.x() < upper.x(); }

      /// Check if the intervall has shrunk close enough to the solution.
      static bool isConverged(const Vector2D& lower, const Vector2D& upper)
      {
        return fabs(lower.y()) < 10e-7 or fabs(upper.y()) < 10e-7;
      }

      /// Returns the better solution x from the bounds of the intervall.
      static double getConvergedBound(const Vector2D& lower, const Vector2D& upper)
      {
        if (not std::isfinite(lower.y()) or not std::isfinite(upper.y())) {
          return NAN;
        }

        if (fabs(lower.y()) <= fabs(upper.y())) {
          return lower.x();
        }

        if (fabs(lower.y()) > fabs(upper.y())) {
          return upper.x();
        }

        return NAN;
      }

    public:
      /// Checks if the function changes sign in the intervall
      static bool changesSign(const Vector2D& lower, const Vector2D& upper)
      { return (lower.y() > 0 and upper.y() < 0) or (lower.y() < 0 and upper.y() > 0); }

      /// Determines if the function is increasing or decreasing in the intervall.
      static EIncDec getEIncDec(const Vector2D& lower, const Vector2D& upper)
      {
        if (lower.y() < upper.y()) {
          return EIncDec::c_Increasing;
        } else if (lower.y() > upper.y()) {
          return EIncDec::c_Decreasing;
        } else if (lower.y() == upper.y()) {
          return EIncDec::c_Constant;
        } else {
          return EIncDec::c_Invalid;
        }
      }

    public:
      /// Get the local extermum that is located in the half period indicated by the given index.
      double computeExtremumXInHalfPeriod(int iHalfPeriod) const;

      /// Helper function to translate the index of the half period to index of the containing period.
      static int getIPeriodFromIHalfPeriod(int iHalfPeriod)
      { return isEven(iHalfPeriod) ? iHalfPeriod / 2 : (iHalfPeriod - 1) / 2; }

    public:
      /// Indicates that the slope is so large such that the function has no local exterma.
      bool hasLargeSlope() const
      { return fabs(getSlope()) >= 1; }

      /// Getter for the slope
      double getSlope() const
      { return m_slope; }

      /// Getter for the intercept
      double getIntercept() const
      { return m_intercept; }

    private:
      /// Memory for the slope.
      double m_slope;

      /// Memory for the intercept.
      double m_intercept;


    };

  }
}

