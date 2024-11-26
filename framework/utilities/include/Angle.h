/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

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

  protected:

    double m_angle; /**< Angle in rad */
    double m_error; /**< Error in rad */

  };


  /**
   * @sa BaseAngle
   * Theta specific.
   */
  class ThetaAngle : public BaseAngle {
  public:
    /**
     * Constructor using radian units.
     * Theta in [0, pi].
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
      double angularDistance, sigmaError;
      /* Distance in the range [0, pi]. */
      angularDistance = std::abs(m_angle - angle.getAngle());
      sigmaError = sigma * (m_error + angle.getError());
      return angularDistance < sigmaError;
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
     * Phi in [0, 2 * pi).
     * @param angle phi angle
     * @param error error on the phi angle
     */
    PhiAngle(double angle, double error) : BaseAngle(angle, error)
    {
      /* Angle in the range (-2 * pi, 2 * pi). */
      m_angle = std::fmod(m_angle, TMath::TwoPi());
      if (m_angle < 0)
        m_angle = m_angle + TMath::TwoPi();
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
      double angularDistance, shortestAngularDistance, sigmaError;
      /* Distance in the range [0, 2 * pi). */
      angularDistance = std::abs(m_angle - angle.getAngle());
      /* Distance in the range [0, pi]. */
      if (angularDistance > TMath::Pi())
        shortestAngularDistance = TMath::TwoPi() - angularDistance;
      else
        shortestAngularDistance = angularDistance;
      sigmaError = sigma * (m_error + angle.getError());
      return shortestAngularDistance < sigmaError;
    }

  };

}
