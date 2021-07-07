/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>

namespace Belle2 {

  /**
   * Calibration constants for time-walk correction and for tuning of electronic time
   * resolution in digitization.
   */
  class TOPCalTimeWalk: public TObject {

  public:

    /**
     * Calibration status of a constant
     */
    enum EStatus {
      c_Default = 0,    /**< uncalibrated default value */
      c_Calibrated = 1, /**< good calibrated value */
      c_Unusable = 2    /**< bad calibrated value */
    };

    /**
     * Default constructor
     */
    TOPCalTimeWalk()
    {}

    /**
     * Sets all the parameters and switches status to calibrated
     * @param timeWalkParams polynomial coefficients of time-walk calibration curve [ns]
     * @param noise noise term excess coefficient of electronic time resolution [ns]
     * @param quadratic quadratic term coefficient of electronic time resolution [ns]
     */
    void set(const std::vector<double>& timeWalkParams, double noise, double quadratic);

    /**
     * Switches calibration status to unusable to flag badly calibrated constants
     */
    void setUnusable() {m_status = c_Unusable;}

    /**
     * Returns polynomial coefficients of time walk calibration curve
     * @return coefficents of polynomial [ns]
     */
    const std::vector<double>& getTimeWalkParams() const {return m_timeWalkParams;}

    /**
     * Returns noise term excess coefficient of electronic time resolution
     * @return noise term excess coefficient [ns]
     */
    double getNoiseCoefficient() const {return m_noise;}

    /**
     * Returns quadratic term coefficient of electronic time resolution
     * @return quadratic term coefficient [ns]
     */
    double getQuadraticCoefficient() const {return m_quadratic;}

    /**
     * Returns time-walk at given pulse height
     * @param pulseHeight pulse height [ADC counts]
     * @return time-walk [ns]
     */
    double getTimeWalk(int pulseHeight) const;

    /**
     * Returns an excess of electronic time resolution at given pulse height
     * @param pulseHeight pulse height [ADC counts]
     * @return excess sigma squared [ns^2]
     */
    double getSigmaSq(int pulseHeight) const;

    /**
     * Returns an excess of electronic time resolution at given pulse height
     * @param pulseHeight pulse height [ADC counts]
     * @return excess sigma [ns]
     */
    double getSigma(int pulseHeight) const;

    /**
     * Returns calibration status
     * @return true, if good calibrated
     */
    bool isCalibrated() const {return m_status == c_Calibrated;}

    /**
     * Returns calibration status
     * @return true, if default (not calibrated)
     */
    bool isDefault() const {return m_status == c_Default;};

    /**
     * Returns calibration status
     * @return true, if bad calibrated
     */
    bool isUnusable() const {return m_status == c_Unusable;};

  private:

    std::vector<double> m_timeWalkParams; /**< parameters of calibration curve [ns] */
    double m_noise = 0; /**< noise term excess coefficient [ns] */
    double m_quadratic = 0; /**< quadratic term coefficient [ns] */
    EStatus m_status = c_Default; /**< calibration status */

    ClassDef(TOPCalTimeWalk, 1); /**< ClassDef */

  };


} // end namespace Belle2
