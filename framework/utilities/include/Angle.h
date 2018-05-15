/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Markus Prim                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>

#include <TMath.h>

#include <utility>

namespace Belle2 {

  /**
   * Class to compare if two angles are compatible withing a given error range.
   */
  class BaseAngle {
  public:
    /** Constructor
     * @param angle angle to compare
     * @param error error on the angle
     */
    BaseAngle(double angle, double error) : m_angle(angle), m_error(error) {};

    /** Destructor */
    virtual ~BaseAngle() {};

    /** Getter for the angle.
     */
    double getAngle() const { return m_angle; }

    /** Getter for the error of the angle.
    */
    double getError() const { return m_error; }

    /** Getter for angle in degrees.
     */
    double getAngleInDeg() const { return m_angle * TMath::RadToDeg(); }

    /** Getter for the error of the angle in degrees.
     * */
    double getErrorInDeg() const { return m_error * TMath::RadToDeg(); }

    /** Getter for the lower interval bound: angle-sigma*error.
     * @param sigma width of the error interval in standard deviations
     * @return angle - sigma * error
     */
    double getLowerIntervalBoundary(double sigma = 1) const { return m_angle - sigma * m_error; }

    /** Getter for the upper interval bound: angle+sigma*error
     * @param sigma width of the error interval in standard deviations
     * @return angle + sigma * error
     */
    double getUpperIntervalBoundary(double sigma = 1) const { return m_angle + sigma * m_error; }

  protected:
    typedef std::pair<double, double> Interval;  /**< Shortcut for std::pair used as interval. */

    double m_angle; /**< Angle in rad */
    double m_error; /**< Error in rad */

    /**
     * Checks if the intervals overlap at some point.
     * If comparing phi angles, be sure to have the interval in a way this function can handle it, meaning always between [0, 2pi).
     * @param x_interval first interval to check
     * @param y_interval second interval
     * @return true if x and y overlap
     */
    bool intervalsCompatible(const Interval& x_interval, const Interval& y_interval) const
    {
      const double xCenter = (x_interval.second + x_interval.first) / 2.;
      const double yCenter = (y_interval.second + y_interval.first) / 2.;
      const double delta = std::fabs(yCenter - xCenter);

      if (delta >= 0) {
        const double xArm = x_interval.second - xCenter;
        const double yArm = yCenter - y_interval.first;
        const double sigmaSum = xArm + yArm;
        if (delta < sigmaSum) return true;
        return false;
      } else {
        const double xArm = xCenter - x_interval.first;
        const double yArm = y_interval.second - yCenter;
        const double sigmaSum = xArm + yArm;
        if (std::fabs(delta) < sigmaSum) return true;
        return false;
      }

    }

  };


  /**
   * @sa BaseAngle
   * Theta specific.
   */
  class ThetaAngle : public BaseAngle {
  public:
    /**
     * Constructor using radian units.
     * Theta in [0, Pi)
     * @param angle theta angle to use
     * @param error error on the angle
     */
    ThetaAngle(double angle, double error) : BaseAngle(angle, error)
    {
      m_angle = std::fabs(std::fmod(m_angle, TMath::Pi()));
    }

    /** Check if two angles are compatible.
     * @param angle second angle to compare to
     * @return true if within one standard deviation
     */
    bool contains(const ThetaAngle& angle) const
    {
      return containsIn(angle, 1);
    }

    /** Check if two angles are compatible.
     * @param angle second angle to compare to
     * @param sigma width of the error band in standard deviations
     * @return true if within sigma standard deviations
     */
    bool containsIn(const ThetaAngle& angle, double sigma) const
    {
      const Interval x(getLowerIntervalBoundary(sigma), getUpperIntervalBoundary(sigma));
      const Interval y(angle.getLowerIntervalBoundary(sigma), angle.getUpperIntervalBoundary(sigma));

      if (intervalsCompatible(x, y)) return true;

      return false;
    }
  };


  /**
   * @sa BaseAngle
   * Phi specific.
   */
  class PhiAngle : public BaseAngle {
  public:
    /**
     * Constructor using radian units.
     * Phi in [0, 2pi]
     * @param angle phi angle
     * @param error error on the phi angle
     */
    PhiAngle(double angle, double error) : BaseAngle(angle, error)
    {
      m_angle = std::fabs(std::fmod(m_angle, TMath::TwoPi()));
    }

    /** Check if two angles are compatible.
     * @param angle second angle to compare
     * @return true if within one standard deviation
     */
    bool contains(const PhiAngle& angle) const
    {
      return containsIn(angle, 1);
    }

    /** Check if two angles are compatible.
     * @param angle second angle to compare to
     * @param sigma width of the error band in standard deviations
     * @return true if within sigma standard deviations
     */
    bool containsIn(const PhiAngle& angle, double sigma) const
    {
      const double twoPi = TMath::TwoPi();
      const Interval x(getLowerIntervalBoundary(sigma), getUpperIntervalBoundary(sigma));
      const Interval y(angle.getLowerIntervalBoundary(sigma), angle.getUpperIntervalBoundary(sigma));

      //Transform intervals
      const double shift = x.first;
      const Interval xShifted(x.first - shift, x.second - shift);
      const Interval yShifted(y.first - shift, y.second - shift);

      if (twoPi < xShifted.second) return true;  // x covers [0, 2Pi)

      if ((yShifted.first <= 0) and (twoPi < yShifted.second)) return true;  // y covers [0, 2Pi)

      if ((0. <= yShifted.first) and (yShifted.second < twoPi)) {
        return intervalsCompatible(xShifted, yShifted);
      }

      if (!(0. <= yShifted.first) and (yShifted.second < twoPi)) {
        const Interval y1(y.first + twoPi, twoPi);
        const Interval y2(0., y.second);

        if (intervalsCompatible(x, y1)) return true;
        if (intervalsCompatible(x, y2)) return true;
        return false;
      }

      if ((0 <= yShifted.first) and !(yShifted.second < twoPi)) {
        const Interval y1(y.first, twoPi);
        const Interval y2(0., y.second - twoPi);

        if (intervalsCompatible(x, y1)) return true;
        if (intervalsCompatible(x, y2)) return true;
        return false;
      }

      return false;
    }
  };
}
