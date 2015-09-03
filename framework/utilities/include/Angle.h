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

  class BaseAngle {
  public:
    /** Constructor
     * @param angle
     * @param error
     */
    BaseAngle(double angle, double error) : m_angle(angle), m_error(error) {};

    /** Destructor */
    virtual ~BaseAngle() {};

    /** Getter for the angle.
     * @return
     */
    double getAngle() const { return m_angle; }

    /** Getter for the error of the angle.
    * @return
    */
    double getError() const { return m_error; }

    /** Getter for angle in degrees.
     * @return
     */
    double getAngleInDeg() const { return m_angle * TMath::RadToDeg(); }

    /** Getter for the error of the angle in degrees.
     * @return
     * */
    double getErrorInDeg() const { return m_error * TMath::RadToDeg(); }

    /** Getter for the lower interval bound: angle-sigme*error.
     * @param sigma
     * @return
     */
    double getLowerIntervalBoundary(double sigma = 1) const { return m_angle - sigma * m_error; }

    /** Getter for the upper interval bound: angle+sigma*error
     * @param sigma
     * @return
     */
    double getUpperIntervalBoundary(double sigma = 1) const { return m_angle + sigma * m_error; }

  protected:
    typedef std::pair<double, double> Interval;

    double m_angle; /**< Angle in rad */
    double m_error; /**< Error in rad */

    /**
     * Checks if the intervals overlap at some point.
     * If comparing phi angles, be sure to have the interval in a way this function can handle it, meaning always between [0, 2pi).
     * @param x
     * @param y
     * @return
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


  class ThetaAngle : public BaseAngle {
  public:
    /**
     * Constructor using radian units.
     * Theta in [0, Pi)
     * @param angle
     * @param error
     */
    ThetaAngle(double angle, double error) : BaseAngle(angle, error)
    {
      m_angle = std::fabs(std::fmod(m_angle, TMath::Pi()));
    }

    /** Check if two angles are compatible.
     * @param angle
     * @return
     */
    bool contains(const ThetaAngle& angle) const
    {
      return containsIn(angle, 1);
    }

    /** Check if two angles are compatible.
     * @param angle
     * @param sigma
     * @return
     */
    bool containsIn(const ThetaAngle& angle, double sigma) const
    {
      const Interval x(getLowerIntervalBoundary(sigma), getUpperIntervalBoundary(sigma));
      const Interval y(angle.getLowerIntervalBoundary(sigma), angle.getUpperIntervalBoundary(sigma));

      if (intervalsCompatible(x, y)) return true;

      return false;
    }
  };


  class PhiAngle : public BaseAngle {
  public:
    /**
     * Constructor using radian units.
     * Phi in [0, 2pi]
     * @param angle
     * @param error
     */
    PhiAngle(double angle, double error) : BaseAngle(angle, error)
    {
      m_angle = std::fabs(std::fmod(m_angle, TMath::TwoPi()));
    }

    /** Check if two angles are compatible.
     * @param angle
     * @return
     */
    bool contains(const PhiAngle& angle) const
    {
      return containsIn(angle, 1);
    }

    /** Check if two angles are compatible.
     * @param angle
     * @param sigma
     * @return
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

      if (twoPi < x.second) return true;  // x covers [0, 2Pi)

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
