/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <cmath>
#include "../include/SinEqLine.h"

using namespace Belle2;
using namespace CDCLocalTracking;





FloatType SinEqLine::computeSmallestPositiveRoot(const Index& maxIHalfPeriod) const
{
  /// Smallest positive root might before the first positive extermum
  FloatType root = computeRootLargerThanExtemumInHalfPeriod(-1);
  if (root > 0) return root;

  /// Most of the time to should be sufficient to look for the root in the first two half periods.
  /// So we spell out this case explicitly.
  root = computeRootLargerThanExtemumInHalfPeriod(0);
  if (root > 0) return root;

  for (Index iHalfPeriod = 1; iHalfPeriod <= maxIHalfPeriod; ++iHalfPeriod) {
    root = computeRootLargerThanExtemumInHalfPeriod(iHalfPeriod);

    /// Check if the solution returned is positiv, that implies that it is not NAN.
    if (root > 0) return root;

  }

  return NAN;
}





FloatType SinEqLine::computeRootLargerThanExtemumInHalfPeriod(const Index& iHalfPeriod) const
{
  if (hasLargeSlope()) {
    return computeRootForLargeSlope();

  } else {
    FloatType lowerX = computeExtremumXInHalfPeriod(iHalfPeriod);
    FloatType upperX = computeExtremumXInHalfPeriod(iHalfPeriod + 1);
    return computeRootInInterval(lowerX, upperX);

  }
}





FloatType SinEqLine::computeRootForLargeSlope() const
{
  if (not hasLargeSlope()) return NAN;

  FloatType xLineIsPlusOne = (1.0 - getIntercept()) / getSlope();
  FloatType xLineIsMinusOne = (-1.0 - getIntercept()) / getSlope();

  if (xLineIsPlusOne > xLineIsMinusOne) {
    return computeRootInInterval(xLineIsMinusOne, xLineIsPlusOne);

  } else if (xLineIsPlusOne < xLineIsMinusOne) {
    return computeRootInInterval(xLineIsPlusOne, xLineIsMinusOne);
  } else {
    return NAN;
  }

}





FloatType SinEqLine::computeRootInInterval(FloatType lowerX, FloatType upperX) const
{
  if (not(lowerX < upperX)) return NAN;

  Vector2D lower(lowerX, map(lowerX));
  Vector2D upper(upperX, map(upperX));

  /// Checks if convergence criterium has been met. For instance if one bound is already exactly at the root.
  if (isConverged(lower, upper)) {
    B2INFO("Coverage early");
    return getConvergedBound(lower, upper);
  }

  /// Checks if the function changes sign in the interval
  if (not changesSign(lower, upper)) {
    return NAN;
  }

  Vector2D last(lower);
  Vector2D current(upper);

  Vector2D next;
  next.setX(secantX(last, current));
  next.setY(map(next.x()));

  // Should  always succeed since we checked everything before
  bool updatedBound = updateBounds(lower, upper, next);
  if (not updatedBound) return NAN;

  while (not isConverged(lower, upper)) {
    // swap accepted values
    last.set(current);
    current.set(next);

    next.setX(newtonX(current));
    next.setY(map(next.x()));

    updatedBound = updateBounds(lower, upper, next);

    if (not updatedBound) {

      // fall back to sekant.
      next.setX(secantX(last, current));
      next.setY(map(next.x()));

      updatedBound = updateBounds(lower, upper, next);

      if (not updatedBound) {
        // fallback to interval devision.
        next.setX(middleX(lower, upper));
        next.setY(map(next.x()));
        updatedBound = updateBounds(lower, upper, next);

        if (not updatedBound) break;
      }

    }

  }

  if (isConverged(lower, upper)) {
    return getConvergedBound(lower, upper);

  } else {
    return NAN;

  }

}





FloatType SinEqLine::middleX(const Vector2D& lower, const  Vector2D& upper)
{
  return (lower.x() + upper.x()) / 2.0;
}





FloatType SinEqLine::secantX(const Vector2D& lower, const Vector2D& upper)
{
  return (lower.x() * upper.y() - upper.x() * lower.y()) / (upper.y() - lower.y());
}





FloatType SinEqLine::newtonX(const Vector2D& pos) const
{
  return pos.x() - pos.y() / gradient(pos.x());
}





bool SinEqLine::updateBounds(Vector2D& lower, Vector2D& upper, const Vector2D& current)
{
  /// Only update if the current point is inbetween the lower and upper bound
  if (not isBetween(lower, current, upper)) return false;

  IncDecInfo incDecInfo = getIncDecInfo(lower, upper);
  if (incDecInfo == INCREASING) {
    if (current.y() > 0.0 and upper.y() > 0.0) {
      upper.set(current);
      return true;

    } else if (current.y() <= 0.0 and lower.y() <= 0.0) {
      lower.set(current);
      return true;

    } else {
      return false;

    }

  } else if (incDecInfo == DECREASING) {
    if (current.y() >= 0 and lower.y() >= 0.0) {
      lower.set(current);
      return true;

    } else if (current.y() < 0 and upper.y() < 0) {
      upper.set(current);
      return true;

    } else {
      return false;

    }
  } else {
    return false;
  }
}





FloatType SinEqLine::computeExtremumXInHalfPeriod(const Index& iHalfPeriod) const
{
  const FloatType& slope = getSlope();
  FloatType extremumInFirstHalfPeriod = acos(slope);

  FloatType extremumInFirstPeriod = isEven(iHalfPeriod) ? extremumInFirstHalfPeriod : 2 * PI - extremumInFirstHalfPeriod;

  Index iPeriod = getIPeriodFromIHalfPeriod(iHalfPeriod);
  return extremumInFirstPeriod + 2 * PI * iPeriod;
}
